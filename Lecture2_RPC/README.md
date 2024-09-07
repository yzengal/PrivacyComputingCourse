# Lecture 2: RPC

The purpose of this project is to help students learn how to use the basic tools, including **gRPC**, **SSH**, and **CMake**.

## Application scenario

The scenario of **online payment** between an e-commerce platform and a bank involves a seamless and secure transaction process that enables customers to purchase goods or services from the platform using their bank accounts. In this process, the e-commerce platform allows customers to select their preferred payment method, such as credit card, debit card, or bank transfer, during checkout. Upon selecting a payment option, the customer's bank account information is securely transmitted to the bank's servers for verification and authorization. Once the payment is approved, the funds are transferred from the customer's account to the e-commerce platform's account, and the transaction is completed. 

For simplicity, we assume that there is a customer named Bob and wants to pay 5000 yuan for a mobile phone. To achieve this goal, the e-commerce platform (i.e., **client**) will send a message with the customer name, credit card number, and total payment to the bank (i.e., **server**). After receiving the message, the bank will verify the payment request and send a response with either **success or failed** to the e-commerce platform.

## Examples

In the following, we have provided three example codes for simulating the aforementioned application scenario.

### Example 1: Implement by Socket

1. Execute the following commands to compile **client** and **server**:
```
cd Lecture2/RPC/socket
mkdir build
cd build
cmake ..
make
```

2. Execute the following command to enable the **server**:
```
./server
```

3. Execute the following command to enable the **client**:
```
./client
```

### Example 2: Implement by gRPC

Before compiling this example, you need to install the [gRPC](https://github.com/grpc/grpc) first by following the [guideline](https://grpc.io/docs/languages/cpp/quickstart/).

1. Update the environment variables related to gRPC by the following command:
```
cd Lecture2/RPC/grpc
source environment.sh
```
Notice that, if your gRPC is not installed in ``/opt/gRPC``, you need to revise ``environment.sh`` by replacing with your install path of gRPC.

2. Execute the following commands to compile **client** and **server**:
```
cd Lecture2/RPC/gRPC
mkdir build
cd build
cmake ..
make
```

3. Execute the following command to enable the **server**:
```
./grpc_server
```

4. Execute the following command to enable the **client**:
```
./grpc_client
```

### Example 3: More on gRPC

This example is similar to the second one. The major differences are
* This one has deployed ``sleep(1~5 second)`` in the server-side, so that students can understand the synchronous in RPC;
* This one has useful ``log`` information, such as running time and communication cost, which are general metrics of privacy computing techniques.
* 
1. Update the environment variables related to gRPC by the following command:
```
cd Lecture2/RPC/more_grpc
source environment.sh
```
Notice that, if your gRPC is not installed in ``/opt/gRPC``, you need to revise ``environment.sh`` by replacing with your install path of gRPC.

2. Execute the following commands to compile **client** and **server**:
```
cd Lecture2/RPC/more_grpc
mkdir build
cd build
cmake ..
make
```

3. Execute the following command to enable the **server**:
```
./grpc_server
```

4. Execute the following command to enable the **client**:
```
./grpc_client
```