#include <iostream>  
#include <memory>  
#include <string> 
#include "grpcpp/grpcpp.h"  
#include "payment.grpc.pb.h" 

#define PORT 50000
  
using grpc::Server;  
using grpc::ServerBuilder;  
using grpc::ServerContext;  
using grpc::Status;  
using payment::PaymentRequest;  
using payment::PaymentResponse;  
using payment::PaymentService;  
  
class PaymentServiceImpl final : public PaymentService::Service {  
    Status ProcessPayment(ServerContext* context, const PaymentRequest* request,  
                            PaymentResponse* response) override {  
        // Simulate payment processing  
        bool is_valid_card = request->credit_card_number().size() == 12;  // Simple validation  
        bool has_funds = request->total_charge() <= 100000;  // Assuming bank has funds for <= 100000  

        std::string result_message;  
        if (is_valid_card && has_funds) {  
            response->set_success(true);  
            result_message = "Payment successful!";  
        } else {  
            response->set_success(false);  
            if (!is_valid_card) {  
                result_message = "Invalid credit card number.";  
            } else {  
                result_message = "Insufficient funds.";  
            }  
        }  
        response->set_message(result_message);  
        return Status::OK;  
    }  
};  
  
void RunServer() {  
    std::string server_address("0.0.0.0:"); 
    server_address += std::to_string(PORT); 
    PaymentServiceImpl service;  

    ServerBuilder builder;  
    // Listen on the given address without any authentication mechanism.  
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
