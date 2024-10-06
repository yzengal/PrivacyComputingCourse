#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <grpcpp/grpcpp.h>

#include "util.hpp"
#include "DiffieHellman.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using google::protobuf::Empty;
using DiffieHellman::DiffieHellmanService;
using DiffieHellman::DiffieHellmanParams;
using DiffieHellman::DiffieHellmanRg;
  
class DiffieHellmanClient {  
public:  
    DiffieHellmanClient(std::shared_ptr<Channel> channel)  
        : stub_(DiffieHellmanService::NewStub(channel)) {}  

    void GetParams() {  
        Empty request;
        DiffieHellmanParams response;
        ClientContext context;

        Status status = stub_->GetParams(&context, request, &response);

        if (status.ok()) {
            p = response.p();
            g = response.g();
            std::cout << "Received Diffie-Hellman parameters:" << std::endl;
            std::cout << "\t" << "p: " << response.p() << std::endl;
            std::cout << "\t" << "g: " << response.g() << std::endl;
        } else {
            std::cerr << "RPC failed: " << status.error_message() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
  
    void PerformKeyExchange() {
        DiffieHellmanRg request;
        DiffieHellmanRg response;
        ClientContext context;

        // 从1到p-2之间随机生成a
        uint64_t b = sample_random(1, p-2);
        uint64_t B = mod_pow(g, b, p);
        uint64_t A;

        // 从gRPC服务获取Diffie-Hellman参数  
        request.set_rg_mod_p(B);
        Status status = stub_->GetRandom(&context, request, &response);
  
        if (!status.ok()) {
            std::cerr << "RPC failed: " << status.error_message() << std::endl;
            std::cout << "Failed to get Diffie-Hellman random value A from server." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        A = response.rg_mod_p();

        // 计算共享密钥 A^b%p  
        uint64_t shared_secret_key = mod_pow(A, b, p);
  
        std::cout << "Client private key: " << b << std::endl;
        std::cout << "Client public key: " << B << std::endl;
        std::cout << "Server public key: " << A << std::endl;
        std::cout << "Shared secret key: " << shared_secret_key << std::endl;
    }

private:
    uint64_t p, g;
    std::unique_ptr<DiffieHellmanService::Stub> stub_;
};
  
int main(int argc, char** argv) { 
    std::string ip_address("localhost");
    if (argc > 1) {
        ip_address = std::string(argv[1]);
    }
    ip_address += std::string(":");
    ip_address += std::to_string(PORT);
    std::cout << "Connect with server: IP " << ip_address << " at PORT " << PORT << std::endl; 
     
    DiffieHellmanClient client(grpc::CreateChannel(ip_address, grpc::InsecureChannelCredentials()));
    
    // receive p and g
    client.GetParams();
    // perform key exchange
    client.PerformKeyExchange();

    return 0;
}
