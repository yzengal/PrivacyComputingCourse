# PrivacyComputingCourse
This GitHub repository is the project of "**Efficient Privacy Computing for Large-Scale Federated Data**" in the undergraduate research class at Beihang University.

## Environment

Linux: Ubuntu 18.04.4 LTS   
GCC/G++: 11.2.0   
CMake: 3.19.7
gRPC: 1.62.0   

## Independent Project

### Lecture 2: RPC

The RPC (Remote Procedure Call) project is a comprehensive learning initiative designed to empower students with hands-on experience in utilizing the powerful open-source **gRPC** framework. This project not only delves into the intricacies of building and deploying RPC systems but also fosters an understanding of crucial development tools such as **CMake**, which plays a pivotal role in managing build processes and dependencies.

Through this project, students can gain practical knowledge on how to define service interfaces in Protocol Buffers (**ProtoBuf**), implement these services in their preferred programming languages (e.g., C/C++ in the example), and then seamlessly communicate between different processes or even different servers. The focus on CMake as a build system helps students appreciate the importance of automation and reproducibility in software development, enabling them to streamline their build and deployment processes.

Please refer to the project folder [Lecture2_RPC](Lecture2_RPC) for more details and instructions.

### Lecture 3: Cryptography

#### What is Cryptography and Its Classification?

Cryptography is the science and practice of encoding and decoding messages to ensure confidentiality, integrity, and authenticity of information. It serves as a fundamental tool in securing communications and data. Cryptography can be broadly classified into two categories: **Classical Cryptography** and **Modern Cryptography**. Classical cryptography, often referred to as historical or ancient cryptography, includes methods such as substitution ciphers (e.g., the Caesar Cipher) and transposition ciphers, which were widely used before the advent of computers. Modern cryptography, on the other hand, leverages advanced mathematical principles and computational techniques to create more secure encryption systems, including symmetric and asymmetric encryption algorithms like AES and Diffie-Hellman key exchange.

#### Source Code Overview: Algorithms and Demonstration Scenario

The lecture source code for this Cryptography course encompasses three pivotal algorithms: **Caesar Cipher**, **Diffie-Hellman Key Exchange**, and **AES Encryption & Decryption**. The **Caesar Cipher** serves as a classic example of a substitution cipher, demonstrating its basic usage and illustrating a simple method for breaking it, providing a historical context and understanding of early cryptographic techniques. The **Diffie-Hellman Key Exchange** algorithm demonstrates how two parties can securely generate a shared secret key over an insecure channel, laying the foundation for modern secure communications. **AES (Advanced Encryption Standard)** represents the modern symmetric encryption standard, providing robust encryption and decryption capabilities essential for contemporary data security.

In addition to these algorithms, the source code integrates the **gRPC (Google Remote Procedure Call)** communication framework to create a demonstration scenario where messages are encrypted using AES and securely transmitted between two parties. The demonstration also showcases the use of Diffie-Hellman for key exchange, ensuring that the encryption keys used for AES are securely shared. This integration not only underscores the practical application of cryptographic principles but also highlights the importance of secure communication protocols in modern computing environments.

Please refer to the project folder [Lecture3_CRYPTO](Lecture3_CRYPTO) for more details and instructions.