#include <iostream>  
#include <cstring>  
#include <unistd.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
  
#define PORT 5000  
  
int main() {  
    struct sockaddr_in serv_addr;  
    int sock = 0;  
  
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {  
        std::cerr << "Socket creation error" << std::endl;  
        return -1;  
    }  
  
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_port = htons(PORT);  
  
    // Convert IPv4 and IPv6 addresses from text to binary form  
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {  
        std::cerr << "Invalid address or address not supported" << std::endl;  
        return -1;  
    }  
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {  
        std::cerr << "Connection Failed" << std::endl;  
        return -1;  
    }  
  
    std::string message = "User: Bob, CreditCard: 1234xxxx5678, TotalCharge: 5000.00";  
    send(sock, message.c_str(), message.length(), 0);  
  
    char buffer[1024] = {0};  
    int valread = read(sock, buffer, 1024);  
    std::cout << "Received from server: " << std::string(buffer, valread) << std::endl;  
  
    close(sock);  
    return 0;  
}