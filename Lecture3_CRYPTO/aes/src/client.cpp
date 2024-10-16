#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <grpcpp/grpcpp.h>

#include "utils/AES.h"
#include "utils/util.hpp"
#include "DiffieHellman.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using google::protobuf::Empty;
using DiffieHellman::DiffieHellmanService;
using DiffieHellman::DiffieHellmanParams;
using DiffieHellman::DiffieHellmanRg;
using DiffieHellman::EncryptData;
  
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
            // std::cout << "Received Diffie-Hellman parameters:" << std::endl;
            // std::cout << "\t" << "p: " << response.p() << std::endl;
            // std::cout << "\t" << "g: " << response.g() << std::endl;
        } else {
            std::cerr << "RPC failed: " << status.error_message() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
  
    uint64_t PerformKeyExchange() {
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

        std::cout << std::hex;
        // std::cout << "Client private key: " << b << std::endl;
        // std::cout << "Client public key: " << B << std::endl;
        // std::cout << "Server public key: " << A << std::endl;
        std::cout << "Shared secret key: 0x" << std::setw(8) << std::setfill('0') << shared_secret_key << std::endl;
        std::cout << std::dec;

        return shared_secret_key;
    }

    void SendEncryptData(const std::vector<unsigned char>& encrypt_data) {
        EncryptData request;
        Empty response;
        ClientContext context;

        request.set_data(std::string(encrypt_data.begin(), encrypt_data.end()));
        Status status = stub_->GetEncryption(&context, request, &response);
  
        if (!status.ok()) {
            std::cerr << "RPC failed: " << status.error_message() << std::endl;
            std::cout << "Failed to send encrypt data." << std::endl;
            std::exit(EXIT_FAILURE);
        }

        std::cout << "Send encrypt data size: " << request.ByteSizeLong() << " [byte]." << std::endl;
    }

    void AesTest(const std::string& message) {
        std::vector<unsigned char> aes_key, aes_iv;

        // perform key exchange twice to obtain AES keys (128bit)
        for (int i=0; i<2; ++i) {
            uint64_t aes_key_64bit = PerformKeyExchange();
            std::vector<unsigned char> aes_tmp = uint64_to_uint8_vector(aes_key_64bit);
            aes_key.insert(aes_key.end(), aes_tmp.begin(), aes_tmp.end());
        }

        // perform key exchange twice to obtain AES IV (128bit)
        for (int i=0; i<2; ++i) {
            uint64_t aes_iv_64bit = PerformKeyExchange();
            std::vector<unsigned char> aes_tmp = uint64_to_uint8_vector(aes_iv_64bit);
            aes_iv.insert(aes_iv.end(), aes_tmp.begin(), aes_tmp.end());
        }

        // print the aes key and iv
        assert(16==aes_iv.size() && 16==aes_key.size());
        printUnsignedVectorInHex(aes_key, "aes_key");
        printUnsignedVectorInHex(aes_iv, "aes_iv ");

        // create AES encrytor
        AES aes(AESKeyLength::AES_128);

        // use CBC mode to encrypt the input message
        std::vector<unsigned char> plain_data = stringToUnsignedVector(message);
        if (plain_data.size()%16 != 0) {
            std::cout << "[BEFORE Padding] plain_data.size() = " << plain_data.size() << std::endl;
            for (int c=plain_data.size()%16; c<16; ++c) {
                plain_data.emplace_back('\0');
            }
            std::cout << "[AFTER Padding] plain_data.size() = " << plain_data.size() << std::endl;
        }
        std::vector<unsigned char> encrypt_data = aes.EncryptCBC(plain_data, aes_key, aes_iv);
        
        // print the encrypted message
        std::cout << "message: " << message << std::endl;
        printUnsignedVectorInHex(encrypt_data, "encrypt_data");

        // send the encrypted message to server via gRPC
        SendEncryptData(encrypt_data);
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

    // perform AES encryption/decryption test
    std::string message("Talk is cheap, show me the code.");
    if (argc > 2) {
        message = std::string(argv[2]);
    }
    client.AesTest(message);


    return 0;
}
