#include <vector>
#include <stdexcept>
#include <algorithm>
#include <numeric> 
#include <cassert>
#include <algorithm>
#include <random>
using namespace std;

class Tensor {
	private: 
		vector<double> data_; // We may need to overload for future quantization potential
		vector<uint16_t> shape_; // Will need to be careful handling wrap around behavior
		uint16_t ndim_;
		uint16_t size_;
		uint16_t itemsize_;  
		uint16_t nbytes_;

	
	public:
    Tensor(const vector<uint16_t>& shape, uint16_t itemsize = sizeof(double))
        : shape_(shape), itemsize_(itemsize) {
        ndim_ = shape_.size();
				if (ndim_ <= 0) {
					throw invalid_argument("Dimension cannot be non-positive");
				}
        size_ = accumulate(shape_.begin(), shape_.end(), 1, multiplies<uint16_t>());
        nbytes_ = size_ * itemsize_;
        data_.resize(size_); 
				// How can we accept multi-dimensional vectors? IMPORTANT
				// How do we initialize tensor data? 
		}

		void fill_vec(const vector<double>& values) {
			if (values.size() != size_) {
				throw invalid_argument("Vector size must match tensor size");
			}
			copy(values.begin(), values.end(), data_.begin());
		}

		void rand_fill(double min, double max) {
			random_device rd;
			mt19937 gen(rd);
			uniform_int_distribution<int16_t> dist(min, max);
			for (double& val : data_) {
				val = dist(gen);
			}
		}

		void homogenous_fill(double value) {
			fill(data_.begin(), data_.end(), value);
		}

		void zeros() {
			fill(data_.begin(), data_.end(), static_cast<double>(0));
		}

		void ones() {
			fill(data_.begin(), data_.end(), static_cast<double>(1));
		}

    uint16_t ndim() const { return ndim_; }
    const vector<uint16_t>& shape() const { return shape_; }
    uint16_t size() const { return size_; }
    uint16_t itemsize() const { return itemsize_; }
    uint16_t nbytes() const { return nbytes_; }

    double& operator[](const vector<uint16_t>& indices) {
        if (indices.size() != ndim_) {
            throw invalid_argument("Invalid number of indices");
        }
        uint16_t flatIndex = 0;
        uint16_t stride = 1;
        for (uint16_t i = ndim_ - 1; i >= 0; --i) {
            flatIndex += indices[i] * stride;
            stride *= shape_[i];
        }
        return data_[flatIndex];
    }

    Tensor broadcast(const vector<uint16_t>& newShape) const {
        if (newShape.size() < ndim_) {
            throw invalid_argument("New shape must have at least the same number of dimensions");
        }
        vector<uint16_t> paddedShape(newShape.size(), 1);
        copy(shape_.rbegin(), shape_.rend(), paddedShape.rbegin());

        for (size_t i = 0; i < newShape.size(); ++i) {
            if (paddedShape[i] != newShape[i] && paddedShape[i] != 1) {
                throw invalid_argument("Incompatible shapes for broadcasting");
            }
        }

        Tensor result(newShape, itemsize_);
        vector<uint16_t> indices(newShape.size(), 0);
        for (uint16_t i = 0; i < result.size(); ++i) {
            vector<uint16_t> originalIndices(ndim_, 0);
            for (uint16_t j = 0; j < ndim_; ++j) {
                originalIndices[j] = indices[newShape.size() - ndim_ + j] % shape_[j];
            }
            result[indices] = (*this)[originalIndices];

            for (uint16_t j = newShape.size() - 1; j >= 0; --j) {
                if (++indices[j] < newShape[j]) {
                    break;
                }
                indices[j] = 0;
            }
        }
        return result;
    }

		Tensor transpose(const vector<uint16_t>& newOrder) const {
    	if (newOrder.size() != ndim_) {
        throw invalid_argument("New order must have the same number of dimensions");
    	}

    	vector<uint16_t> newShape(ndim_);
    	for (uint16_t i = 0; i < ndim_; ++i) {
        if (newOrder[i] < 0 || newOrder[i] >= ndim_) {
            throw invalid_argument("Invalid dimension order");
        }
        newShape[i] = shape_[newOrder[i]];
    	}

    	Tensor result(newShape, itemsize_);
    	vector<uint16_t> indices(ndim_, 0);
    	for (uint16_t i = 0; i < size_; ++i) {
        vector<uint16_t> originalIndices(ndim_);
        for (uint16_t j = 0; j < ndim_; ++j) {
          originalIndices[newOrder[j]] = indices[j];
        }
        result[indices] = (*this)[originalIndices];

        for (uint16_t j = ndim_ - 1; j >= 0; --j) {
            if (++indices[j] < shape_[j]) {
                break;
            }
            indices[j] = 0;
        }
    	}
    	return result;
		}

};