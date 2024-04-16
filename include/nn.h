#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H

#include <vector>
#include <memory>
#include <string>
#include "autograd.h"

class Module {
public:
    virtual void zeroGrad();
    virtual std::vector<Value*> parameters();
    virtual ~Module() {}
};

class Neuron : public Module {
private:
    std::vector<Value> w_;
    Value b_;
    bool nonlin_;

public:
    Neuron(size_t nin, bool nonlin = true);
    Value operator()(const std::vector<Value>& x);
    std::vector<Value*> parameters() override;
    std::string toString() const;
};

class Layer : public Module {
private:
    std::vector<std::unique_ptr<Neuron>> neurons_;

public:
    Layer(size_t nin, size_t nout, bool nonlin = true);
    std::vector<Value> operator()(const std::vector<Value>& x);
    std::vector<Value*> parameters() override;
    std::string toString() const;
};

class MLP : public Module {
private:
    std::vector<std::unique_ptr<Layer>> layers_;

public:
    MLP(size_t nin, const std::vector<size_t>& nouts);
    std::vector<Value> operator()(const std::vector<Value>& x);
    std::vector<Value*> parameters()
