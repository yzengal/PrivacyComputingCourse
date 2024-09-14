# Lecture 3: Cryptography

The purpose of this project is to help students learn the basic tools of cryptography, including **Caesar Cipher**, **Diffie-Hellmen**, **AES**, and **SEAL**.

## Application scenario

In a practical application scenario, imagine a situation where an individual or an organization needs to securely transmit confidential information, such as a password, a sensitive message, or any piece of data that should not be easily readable by unauthorized parties. Rather than sending the information in plaintext, which could be intercepted and understood by anyone, they can use this program to encrypt the confidential string before sending it.

## Examples

In the following, we have provided three example codes for simulating the aforementioned application scenario.

### Example 1: Caesar Cipher & Decipher

1. Execute the following commands to compile **caesar** and **decipher**:
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

3. After running the above command, you need to type in the text that waits for encryption, such as ``SEND HELP``, and will obtain the output ``Encrypted text: VHQG KHOS`` and ``Decrypted text: SEND HELP``.

4. Now, you have known how the casesar cipher works. As mentioned in the course slides, it is interesting to investigate how to decipher an encrypt data without knowing the exact shift. An example of such deciphers are summarized in ``decipher.cpp``. Execute the following command to run the **decipher**:
```
./decipher Gnyx vf purnc fubj zr gur pbqr
```
Here, ``Gnyx vf purnc fubj zr gur pbqr`` implies the encrypt data.

5. The decipher first reads all the english words as a dictionary from the file ``english-word.txt``. Then, the decipher enumerates all possible values for shift (i.e., 1~25). For each shift size, it tries to decrypt the data and verifies whether all the decrpted words are meaningful (i.e., by searching it in the dictionary). Whenever all the decrpted words are meaningful, the decipher will output the shift size and decrypt data. For example, the output of the above command is ``Decrypt with shift 13: Talk is cheap show me the code``.

### Example 2: Diffie-Hellman Key Exchange

The Diffie-Hellman algorithm is utilized when secure key exchange is required between two communicating parties in the context of symmetric encryption. This scenario arises when both parties need to agree upon a shared secret key over an insecure channel, ensuring that no eavesdropper can intercept and determine the key. By employing the Diffie-Hellman protocol, each party can independently compute the same secret key without ever transmitting the key itself over the communication link, thereby enhancing the security of the subsequent symmetric encryption process.

1. Update the environment variables related to gRPC by the following command:
```
cd Lecture3_CRYPTO/diffie_hellman
source environment.sh
```
Notice that, if your gRPC is not installed in ``/opt/gRPC``, you need to revise ``environment.sh`` by replacing with your install path of gRPC.

2. Execute the following commands to compile **client** and **server**:
```
mkdir build
cd build
cmake ..
make
```

3. Execute the following command to enable the **server**:
```
./server
```

4. Execute the following command to enable the **client**:
```
./client
```

5. From the output of both server-side and client-server, you will obtain the information about their public keys (A and B), private keys (a and b), and shared secret.

* a and b are random samples between 1 and p-2.
* $A = g^a%p$, $B = g^b%p$
* the shared secret is $g^{ab}%p$