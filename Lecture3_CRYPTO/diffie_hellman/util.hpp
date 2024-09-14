#ifndef UTIL_HPP
#define UTIL_HPP

#include <random>

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

#endif // UTIL_HPP