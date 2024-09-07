#include <iostream>  
#include <memory>  
#include <string>   
#include <cstdlib> 
#include "grpcpp/grpcpp.h"  
#include "grpcpp/support/channel_arguments.h"  
#include "payment.grpc.pb.h"  

#define PORT 50000 
  
using grpc::Channel;  
using grpc::ClientContext;  
using grpc::Status;  
using payment::PaymentRequest;  
using payment::PaymentResponse;  
using payment::PaymentService;  
  
class PaymentClient {  
public:  
    PaymentClient(std::shared_ptr<Channel> channel)  
        : stub_(PaymentService::NewStub(channel)) {}  

    // Assembles the client's payload, sends it and presents the response back  
    // from the server.  
    Status ProcessPayment(const PaymentRequest& request, PaymentResponse* response) {  
        ClientContext context;
        // Actually call the server.  
        return stub_->ProcessPayment(&context, request, response);  
    }  
private:  
    std::unique_ptr<PaymentService::Stub> stub_;  
};  
  
int main(int argc, char** argv) {
    std::string ip_addr("127.0.0.1");
    if (argc > 1) {
        ip_addr = std::string(argv[1]);
    }
    std::string ip_addr_withport = ip_addr;
    ip_addr_withport += ":";
    ip_addr_withport += std::to_string(PORT);
    std::cout << "Connect with server: IP " << ip_addr << " at PORT " << PORT << std::endl; 
    
    PaymentRequest request;  
    request.set_credit_card_number("1234xxxx5678");  
    request.set_total_charge(5000.00);  
    request.set_user_name("Bob");  

    PaymentResponse response;  
    PaymentClient payment_client(grpc::CreateChannel(  
        ip_addr_withport, grpc::InsecureChannelCredentials()));  

    Status status = payment_client.ProcessPayment(request, &response);  

    if (status.ok()) {  
        std::cout << "Payment status: " << (response.success() ? "successful" : "failed")  
                    << ", message: " << response.message() << std::endl;  
    } else {  
        std::cout << "RPC failed" << std::endl;  
    }  

    return 0;  
}
