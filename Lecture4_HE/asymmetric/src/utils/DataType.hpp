#ifndef UTILS_DATA_TYPE_HPP
#define UTILS_DATA_TYPE_HPP

#include <vector>
#include <iomanip>
#include <cstddef>
#include <stdexcept>
#include <sstream>  
#include <string>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cstdint>

typedef int64_t VectorDimensionType;
typedef long VidType;
   
struct VectorDataType {  
    VidType vid;  
    std::vector<VectorDimensionType> data;  

    VectorDataType(size_t dim, VidType _vid=0): vid(_vid) {
        data.reserve(dim);
        data.resize(dim);
        std::fill(data.begin(), data.end(), (VectorDimensionType)0);
    }

    VectorDataType(size_t dim, VidType _vid, const VectorDataType& vector_point): vid(_vid) {
        if (vector_point.Dimension() != dim) {
            throw std::invalid_argument("vector data dimension does not match");
            std::exit(EXIT_FAILURE);
        }
        data.reserve(dim);
        data.resize(dim);
        std::copy_n(vector_point.data.begin(), dim, data.begin());
    }

    VectorDataType(size_t dim, VidType _vid, const std::vector<VectorDimensionType>& arr) : vid(_vid) {
        if (arr.size() != dim) {
            throw std::invalid_argument("Data size does not match with vector data dimension");
            std::exit(EXIT_FAILURE);
        }
        data.reserve(dim);
        data.resize(dim);
        std::copy_n(arr.begin(), dim, data.begin());
    }

    void SetVid(VidType _vid) {
        vid = _vid;
    }

    void SetVectorPoint(const VectorDataType& vector_point) {
        const size_t dim = this->Dimension();
        if (vector_point.Dimension() != dim) {
            throw std::invalid_argument("vector data dimension does not match");
            std::exit(EXIT_FAILURE);
        }
        std::copy_n(vector_point.data.begin(), dim, data.begin());
    }

    void SetVectorPoint(const std::vector<VectorDimensionType>& arr) {
        const size_t dim = this->Dimension();
        if (arr.size() != dim) {
            throw std::invalid_argument("Data size does not match with vector data dimension");
            std::exit(EXIT_FAILURE);
        }
        std::copy_n(arr.begin(), dim, data.begin());
    }
  
    VectorDataType& operator=(const VectorDataType& other) {
        if (this != &other) {  
            const size_t dim = this->Dimension();
            if (other.Dimension() != dim) {
                throw std::invalid_argument("Data size does not match with vector data dimension");
                std::exit(EXIT_FAILURE);
            }
            vid = other.vid; 
            std::copy_n(other.data.begin(), dim, data.begin());
        }  
        return *this;  
    }  
  
    bool operator==(const VectorDataType& other) const {  
        if (vid != other.vid) return false;
        const size_t dim = this->Dimension();
        if (other.Dimension() != dim) return false;

        for (size_t i=0; i<dim; ++i) {
            if (other.data[i] != data[i])
                return false;
        }

        return true;  
    }  
   
    bool operator!=(const VectorDataType& other) const {  
        return !(*this == other);  
    }  
  
    VectorDimensionType& at(size_t k) {  
        if (k >= this->Dimension()) {  
            throw std::out_of_range("Index out of range");  
        }  
        return data[k];  
    }  

    const VectorDimensionType& at(size_t k) const {  
        if (k >= this->Dimension()) {  
            throw std::out_of_range("Index out of range");  
        }  
        return data[k];  
    }  
  
    VectorDimensionType& operator[](size_t k) {  
        if (k >= this->Dimension()) {  
            throw std::out_of_range("Index out of range");  
        }  
        return data[k];  
    }  

    const VectorDimensionType& operator[](size_t k) const {  
        if (k >= this->Dimension()) {  
            throw std::out_of_range("Index out of range");  
        }  
        return data[k];  
    } 

    size_t Dimension() const {
        return data.size();
    }

    std::string to_string(size_t print_size=4, size_t prec=6) const {
        const size_t dim = this->Dimension();
        std::stringstream ss;  

        ss << std::fixed << std::setprecision(prec);
        ss << "#" << vid << ": (";
        if (dim <= print_size*2) {
            for (size_t i=0; i<dim; ++i) {
                if (i > 0)
                    ss << ", " << data[i];
                else
                    ss << data[i];
            }
        } else {
            for (size_t i=0; i<print_size; ++i) {
                if (i > 0)
                    ss << ", " << data[i];
                else
                    ss << data[i];
            }
            ss << ", ...";
            for (size_t i=dim-print_size; i<dim; ++i) {
                ss << ", " << data[i];
            }
        }
        ss << ")";
        
        return ss.str();  
    } 
};

VectorDimensionType EuclideanSquareDistance(const VectorDataType& a, const VectorDataType& b);
double EuclideanDistance(const VectorDataType& a, const VectorDataType& b);

VectorDimensionType EuclideanSquareDistance(const VectorDataType& a, const VectorDataType& b) {  
    if (a.Dimension() != b.Dimension()) {
        throw std::invalid_argument("Vector data must have the same dimension");
    }

    const size_t dim = a.Dimension();
    VectorDimensionType sum = 0;  
    for (size_t i = 0; i < dim; ++i) {  
        sum += (a[i] - b[i]) * (a[i] - b[i]);  
    }  
    return sum;  
} 

double EuclideanDistance(const VectorDataType& a, const VectorDataType& b) {  
    if (a.Dimension() != b.Dimension()) {
        throw std::invalid_argument("Vector data must have the same dimension");
    }

    const size_t dim = a.Dimension();
    VectorDimensionType sum = 0;  
    for (size_t i = 0; i < dim; ++i) {  
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    }  
    return std::sqrt(sum*1.0);  
} 

#endif  // UTILS_DATA_TYPE_HPP
