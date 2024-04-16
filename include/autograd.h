#ifndef AUTOGRAD_H
#define AUTOGRAD_H

#include <iostream>
#include <vector>
#include <functional>
#include <string>

class Value {
	private:
		double data_;
		double grad_;
		std::vector<Value*> prev_;
		std::function<void()> backward_;
		std::string op_;

	public:
		Value(double data, const std::vector<Value*>& children, const std::string& op);
		
		double data();
		double grad();
		
		Value operator+(const Value& other) const;
		Value operator*(const Value& other) const;
		Value operator^(const Value& other) const;
		Value relu();
		void backward();
		Value operator-();
		friend Value operator+(double lhs, const Value& rhs);
		Value operator-(const Value& other);
		friend Value operator-(double lhs, const Value& rhs);
		friend Value operator*(double lhs, const Value& rhs);
		Value operator/(const Value& other) const;
		friend Value operator/(double lhs, const Value& rhs);
		friend std::ostream& operator<<(std::ostream& os, const Value& v);

};

#endif
