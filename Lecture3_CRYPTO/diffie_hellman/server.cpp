#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>

#include "util.hpp"
#include "DiffieHellman.grpc.pb.h" 
  
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using google::protobuf::Empty;
using DiffieHellman::DiffieHellmanService;
using DiffieHellman::DiffieHellmanParams;
using DiffieHellman::DiffieHellmanRg;
  
class DiffieHellmanServiceImpl final : public DiffieHellmanService::Service {
    Status GetParams(ServerContext* context, const Empty* request,  
                        DiffieHellmanParams* response) override {
        // 这里可以随机生成p和g，或者使用预定义的  
        p = 797546779;// 示例质数  
        g = 3; // 示例生成元  
        response->set_p(p);
        response->set_g(g);

        std::cout << "Generated Diffie-Hellman parameters:" << std::endl;
        std::cout << "\t" << "p: " << p << std::endl;
        std::cout << "\t" << "g: " << g << std::endl;
        
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
  
        std::cout << "Server private key: " << a << std::endl;
        std::cout << "Server public key: " << A << std::endl;
        std::cout << "Client public key: " << B << std::endl;
        std::cout << "Shared secret key: " << shared_secret_key << std::endl;

        return Status::OK;
    }

private:
    uint64_t p, g;
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