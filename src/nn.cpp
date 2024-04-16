#include <vector>
#include <random>
#include <numeric>
#include <memory>
#include <sstream>
#include "autograd.h"

class Module {
public:
    virtual void zeroGrad() {
        for (auto& p : parameters()) {
            p->grad_ = 0.0;
        }
    }

    virtual std::vector<Value*> parameters() {
        return {};
    }

    virtual ~Module() {}
};

class Neuron : public Module {
private:
    std::vector<Value> w_;
    Value b_;
    bool nonlin_;

public:
    Neuron(size_t nin, bool nonlin = true) : b_(0.0), nonlin_(nonlin) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(-1.0, 1.0);

        for (size_t i = 0; i < nin; ++i) {
            w_.emplace_back(dis(gen));
        }
    }

    Value operator()(const std::vector<Value>& x) {
        Value act = std::inner_product(w_.begin(), w_.end(), x.begin(), b_);
        return nonlin_ ? act.relu() : act;
    }

    std::vector<Value*> parameters() override {
        std::vector<Value*> params;
        for (auto& w : w_) {
            params.push_back(&w);
        }
        params.push_back(&b_);
        return params;
    }

    std::string toString() const {
        std::stringstream ss;
        ss << (nonlin_ ? "ReLU" : "Linear") << "Neuron(" << w_.size() << ")";
        return ss.str();
    }
};

class Layer : public Module {
private:
    std::vector<std::unique_ptr<Neuron>> neurons_;

public:
    Layer(size_t nin, size_t nout, bool nonlin = true) {
        for (size_t i = 0; i < nout; ++i) {
            neurons_.emplace_back(std::make_unique<Neuron>(nin, nonlin));
        }
    }

    std::vector<Value> operator()(const std::vector<Value>& x) {
        std::vector<Value> out;
        for (auto& neuron : neurons_) {
            out.push_back((*neuron)(x));
        }
        return out.size() == 1 ? std::vector<Value>{out[0]} : out;
    }

    std::vector<Value*> parameters() override {
        std::vector<Value*> params;
        for (auto& neuron : neurons_) {
            auto neuron_params = neuron->parameters();
            params.insert(params.end(), neuron_params.begin(), neuron_params.end());
        }
        return params;
    }

    std::string toString() const {
        std::stringstream ss;
        ss << "Layer of [";
        for (size_t i = 0; i < neurons_.size(); ++i) {
            ss << neurons_[i]->toString();
            if (i < neurons_.size() - 1) {
                ss << ", ";
            }
        }
        ss << "]";
        return ss.str();
    }
};

class MLP : public Module {
private:
    std::vector<std::unique_ptr<Layer>> layers_;

public:
    MLP(size_t nin, const std::vector<size_t>& nouts) {
        std::vector<size_t> sz = {nin};
        sz.insert(sz.end(), nouts.begin(), nouts.end());

        for (size_t i = 0; i < nouts.size(); ++i) {
            layers_.emplace_back(std::make_unique<Layer>(sz[i], sz[i + 1], i != nouts.size() - 1));
        }
    }

    std::vector<Value> operator()(const std::vector<Value>& x) {
        std::vector<Value> result = x;
        for (auto& layer : layers_) {
            result = (*layer)(result);
        }
        return result;
    }

    std::vector<Value*> parameters() override {
        std::vector<Value*> params;
        for (auto& layer : layers_) {
            auto layer_params = layer->parameters();
            params.insert(params.end(), layer_params.begin(), layer_params.end());
        }
        return params;
    }

    std::string toString() const {
        std::stringstream ss;
        ss << "MLP of [";
        for (size_t i = 0; i < layers_.size(); ++i) {
            ss << layers_[i]->toString();
            if (i < layers_.size() - 1) {
                ss << ", ";
            }
        }
        ss << "]";
        return ss.str();
    }
};
