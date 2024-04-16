#include <iostream>
#include <vector>
#include <functional>
#include <cmath>

class Value {
private:
    double data_;
    double grad_;
    std::vector<Value*> prev_;
    std::function<void()> backward_;
    std::string op_;

public:
    Value(double data, const std::vector<Value*>& children = {}, const std::string& op = "")
        : data_(data), grad_(0.0), prev_(children), backward_([](){}), op_(op) {}

    double data() const { return data_; }
    double grad() const { return grad_; }

    Value operator+(const Value& other) const {
        Value out(data_ + other.data_, {const_cast<Value*>(this), const_cast<Value*>(&other)}, "+");
        out.backward_ = [&, out]() mutable {
            grad_ += out.grad_;
            const_cast<Value&>(other).grad_ += out.grad_;
        };
        return out;
    }

    Value operator*(const Value& other) const {
        Value out(data_ * other.data_, {const_cast<Value*>(this), const_cast<Value*>(&other)}, "*");
        out.backward_ = [&, out, other]() mutable {
            grad_ += other.data_ * out.grad_;
            const_cast<Value&>(other).grad_ += data_ * out.grad_;
        };
        return out;
    }

    Value operator^(double other) const {
        Value out(std::pow(data_, other), {const_cast<Value*>(this)}, "^" + std::to_string(other));
        out.backward_ = [&, out, other]() mutable {
            grad_ += other * std::pow(data_, other - 1) * out.grad_;
        };
        return out;
    }

    Value relu() const {
        Value out(data_ > 0 ? data_ : 0.0, {const_cast<Value*>(this)}, "ReLU");
        out.backward_ = [&, out]() mutable {
            grad_ += (out.data_ > 0) * out.grad_;
        };
        return out;
    }

    void backward() {
        std::vector<Value*> topo;
        std::unordered_set<Value*> visited;
        std::function<void(Value*)> build_topo = [&](Value* v) {
            if (visited.find(v) == visited.end()) {
                visited.insert(v);
                for (Value* child : v->prev_) {
                    build_topo(child);
                }
                topo.push_back(v);
            }
        };
        build_topo(this);

        grad_ = 1.0;
        for (auto it = topo.rbegin(); it != topo.rend(); ++it) {
            (*it)->backward_();
        }
    }

    Value operator-() const {
        return *this * -1;
    }

    friend Value operator+(double lhs, const Value& rhs) {
        return rhs + lhs;
    }

    Value operator-(const Value& other) const {
        return *this + (-other);
    }

    friend Value operator-(double lhs, const Value& rhs) {
        return lhs + (-rhs);
    }

    friend Value operator*(double lhs, const Value& rhs) {
        return rhs * lhs;
    }

    Value operator/(const Value& other) const {
        return *this * (other ^ -1);
    }

    friend Value operator/(double lhs, const Value& rhs) {
        return lhs * (rhs ^ -1);
    }

    friend std::ostream& operator<<(std::ostream& os, const Value& value) {
        os << "Value(data=" << value.data_ << ", grad=" << value.grad_ << ")";
        return os;
    }
};
