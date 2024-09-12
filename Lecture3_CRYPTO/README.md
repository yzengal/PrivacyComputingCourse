# Lecture 3: Cryptography

The purpose of this project is to help students learn the basic tools of cryptography, including **Caesar Cipher**, **AES**, and **SEAL**.

## Application scenario

In a practical application scenario, imagine a situation where an individual or an organization needs to securely transmit confidential information, such as a password, a sensitive message, or any piece of data that should not be easily readable by unauthorized parties. Rather than sending the information in plaintext, which could be intercepted and understood by anyone, they can use this program to encrypt the confidential string before sending it.

## Examples

In the following, we have provided three example codes for simulating the aforementioned application scenario.

### Example 1: Caesar Cipher

1. Execute the following commands to compile **caesar** and **server**:
```
cd Lecture3_CRYPTO/caesar
mkdir build
cd build
cmake ..
make
```

2. Execute the following command to run the **caesar** to encrypt any input data:
```
./caesar -k 3
```
Here, ``-k 3`` implies the shift size in Caesar Cipher.

