#include <iostream>  
#include <cstring>  
#include <unistd.h>  
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
  
#define PORT 5000  
  
void handle_client(int sockfd) {  
    char buffer[1024];  
    int valread = read(sockfd, buffer, 1024);  
    std::cout << "Received: " << std::string(buffer, valread) << std::endl;  
  
    // Simulate payment verification (always returns success for simplicity)  
    const char* response = "Payment successful!";  
    send(sockfd, response, strlen(response), 0);  
  
    close(sockfd);  
}  
  
int main() {  
    const int QUEUE_SIZE = 3;
    int server_fd, new_socket;  
    struct sockaddr_in address;  
    int addrlen = sizeof(address);  
  
    // Creating socket file descriptor  
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {  
        perror("socket failed");  
        exit(EXIT_FAILURE);  
    }  
  
    // Forcefully attaching socket to the port 5000  
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons(PORT);  
  
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    }  
  
    if (listen(server_fd, QUEUE_SIZE) < 0) {  
        perror("listen");  
        exit(EXIT_FAILURE);  
    }  

    std::cout << "Listen at PORT " << PORT << std::endl;  
  
    while (true) {  
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {  
            perror("accept");  
            exit(EXIT_FAILURE);  
        } 

	// when connection is accepted, obtain the ip address and port from the client.
	char client_ipstr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &address.sin_addr, client_ipstr, sizeof(client_ipstr));
    	int client_port = ntohs(address.sin_port);
	printf("Accepted connection from IP %s at PORT %d\n", client_ipstr, client_port);
  
        // Create a new thread to handle the client  
        // Note: For simplicity, this example does not create a new thread.  
        // In a real application, you would want to handle each client in a separate thread or process.  
        handle_client(new_socket);  
    }  
  
    return 0;  
}
