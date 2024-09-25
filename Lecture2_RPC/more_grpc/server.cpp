#include <iostream>  
#include <memory>  
#include <string> 
#include <unistd.h> 
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
        static size_t request_num = 0;

        // record when the RPC has been called
        std::chrono::steady_clock::time_point startTimer = std::chrono::steady_clock::now();
        
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

        // sleep for 1~5 seconds randomly
        //sleep(1+rand()%5); 

        // record when the RPC call has returned
        std::chrono::steady_clock::time_point endTimer = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTimer - startTimer);
        
        // record the running time and communication overhead
        float time_cost = duration.count(); // unit: ms
        float communication_cost = request->ByteSizeLong() + response->ByteSizeLong(); // unit: bytes
        std::cout << "request no.: " << ++request_num 
                    << ", time cost: " << time_cost << " [ms], " 
                    << "communication cost: " << communication_cost/1024.0 << " [kb]" << std::endl;
        
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
