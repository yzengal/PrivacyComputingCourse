#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <grpcpp/grpcpp.h>


#include "utils/AES.h"
#include "utils/util.hpp"
#include "DiffieHellman.grpc.pb.h" 
  
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using google::protobuf::Empty;
using DiffieHellman::DiffieHellmanService;
using DiffieHellman::DiffieHellmanParams;
using DiffieHellman::DiffieHellmanRg;
using DiffieHellman::EncryptData;
  
class DiffieHellmanServiceImpl final : public DiffieHellmanService::Service {
    Status GetParams(ServerContext* context, const Empty* request,  
                        DiffieHellmanParams* response) override {
        // 这里可以随机生成p和g，或者使用预定义的  
        p = 797546779;// 示例质数  
        g = 3; // 示例生成元  
        response->set_p(p);
        response->set_g(g);

        // std::cout << "Generated Diffie-Hellman parameters:" << std::endl;
        // std::cout << "\t" << "p: " << p << std::endl;
        // std::cout << "\t" << "g: " << g << std::endl;
        
        return Status::OK;
    }

    Status GetRandom(ServerContext* context, const DiffieHellmanRg* request,  
                        DiffieHellmanRg* response) override {
        // 从1到p-2之间随机生成a
        uint64_t a = sample_random(1, p-2);
        uint64_t A = mod_pow(g, a, p);
        uint64_t B = request->rg_mod_p();

        // 设置响应值为A，即实现随机数A与B的交换
        response->set_rg_mod_p(A);

        // 计算共享密钥 B^a%p  
        uint64_t shared_secret_key = mod_pow(B, a, p);

        std::cout << std::hex;
        // std::cout << "Server private key: " << a << std::endl;
        // std::cout << "Server public key: " << A << std::endl;
        // std::cout << "Client public key: " << B << std::endl;
        std::cout << "Shared secret key: 0x" << std::setw(8) << std::setfill('0') << shared_secret_key << std::endl;
        std::cout << std::dec;

        shared_secret_key_list.emplace_back(shared_secret_key);

        return Status::OK;
    }

    Status GetEncryption(ServerContext* context, const EncryptData* request,  
                        Empty* response) override {
        
        std::string received_data = request->data();
        std::vector<unsigned char> encrypt_data = stringToUnsignedVector(received_data);

        // initialize aes key & iv
        assert(4 == shared_secret_key_list.size());
        std::vector<unsigned char> aes_key, aes_iv;

        /// initialize AES key (128bit)
        for (int i=0; i<=1; ++i) {
            std::vector<unsigned char> aes_tmp = uint64_to_uint8_vector(shared_secret_key_list[i]);
            aes_key.insert(aes_key.end(), aes_tmp.begin(), aes_tmp.end());
        }

        /// initialize AES IV (128bit)
        for (int i=2; i<=3; ++i) {
            std::vector<unsigned char> aes_tmp = uint64_to_uint8_vector(shared_secret_key_list[i]);
            aes_iv.insert(aes_iv.end(), aes_tmp.begin(), aes_tmp.end());
        }

        /// print the aes key and iv
        assert(16==aes_iv.size() && 16==aes_key.size());
        printUnsignedVectorInHex(aes_key, "aes_key");
        printUnsignedVectorInHex(aes_iv, "aes_iv ");

        // create AES decryptor
        AES aes(AESKeyLength::AES_128);

        // use CBC mode to encrypt the input message
        std::vector<unsigned char> plain_data = aes.DecryptCBC(encrypt_data, aes_key, aes_iv);
        std::string message(plain_data.begin(), plain_data.end());
        
        // print the encrypted message
        std::cout << "message: " << message << std::endl;
        printUnsignedVectorInHex(encrypt_data, "encrypt_data");
        std::cout << "Receive encrypt data size: " << request->ByteSizeLong() << " [byte]." << std::endl;

        // clear the shared secret keys
        shared_secret_key_list.clear();

        return Status::OK;
    }

private:
    uint64_t p, g;
    std::vector<uint64_t> shared_secret_key_list;
};
  
void RunServer() {
    std::string server_address("0.0.0.0:");
    server_address += std::to_string(PORT);
    DiffieHellmanServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();
}
  
int main(int argc, char** argv) {
    RunServer();

    return 0;
}