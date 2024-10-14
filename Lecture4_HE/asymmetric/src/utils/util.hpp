#ifndef UTIL_HPP
#define UTIL_HPP

#include <random>
#include <iostream>  
#include <cstdint>
#include <vector>
#include <iomanip>

extern const int PORT = 50051;

uint64_t mod_pow(uint64_t base, uint64_t exponent, uint64_t modulus);
uint64_t sample_random(uint64_t from, uint64_t to);

// 简单的模幂运算函数，用于计算 g^x mod p  
uint64_t mod_pow(uint64_t base, uint64_t exponent, uint64_t modulus) {  
    uint64_t result = 1;
    while (exponent > 0) {  
        if (exponent % 2 == 1) {  
            result = (result * base) % modulus;
        }  
        base = (base * base) % modulus;
        exponent /= 2;
    }  
    return result;
} 

// 简单的随机采样函数
uint64_t sample_random(uint64_t from, uint64_t to) {
    if (from > to) {  
        std::cerr << "[sample_random] ``from`` cannot be smaller than ``to``" << std::endl;
        exit(-1);
    }

    if (from == to) return from;

    // 定义随机设备  
    std::random_device rd;  
    // 定义基于随机设备的随机数生成器  
    std::mt19937 gen(rd());  
    // 定义分布：在from到to之间（包含）  
    std::uniform_int_distribution<uint64_t> dis(from, to);  

    return dis(gen);
}

// Convert uint64 to eight uint8
std::vector<unsigned char> uint64_to_uint8_vector(uint64_t value) {  
    std::vector<unsigned char> result(8); // 创建一个大小为8的vector，用于存储8个字节  
  
    // 逐个字节地提取并存储到vector中  
    for (int i = 0; i < 8; ++i) {  
        result[i] = static_cast<unsigned char>((value >> (i * 8)) & 0xFF);  
    }  
  
    return result;  
}

// Convert string to vector<unsigned char>
std::vector<unsigned char> stringToUnsignedVector(const std::string& str) {  
    std::vector<unsigned char> vec(str.size());  
    for (size_t i = 0; i < str.size(); ++i) {  
        vec[i] = static_cast<unsigned char>(str[i]);  
    }  
    return vec;  
}  

void printUnsignedVectorInHex(const std::vector<unsigned char>& vec, const std::string& var_name) {
    if (!var_name.empty())
        std::cout << var_name << ": ";
    size_t idx = 1; 
    for (unsigned char byte : vec) {  
        // 使用 std::hex 设置输出为十六进制格式  
        // 使用 std::setw(2) 和 std::setfill('0') 来确保每个字节占两个字符，不足时前面补0  
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        if (idx%4 == 0) {
            std::cout << " ";
        }
        ++idx;
    }  
    std::cout << std::dec << std::endl; // 打印完所有字节后换行  
}  

#endif // UTIL_HPP