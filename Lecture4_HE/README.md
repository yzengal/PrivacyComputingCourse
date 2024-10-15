# Lecture 4: Homomorphic Encryption

The purpose of this project is to help students learn the basic tools of **Homomorphic Encryption** based on the open-sourced tool [SEAL](https://github.com/microsoft/SEAL).

## Environment

Linux: Ubuntu 22.04 LTS   
CMake: >= 3.10.0
gRPC: >= 1.62.0  
[Boost C++ library](https://www.boost.org/): >= 1.72.0  
[Microsoft SEAL](https://github.com/microsoft/SEAL): >= 4.1.0

## Application scenario

**Homomorphic Encryption**, often leveraged in the research domain of **secure query processing**, enables sophisticated data retrieval operations without compromising privacy. Consider two primary scenarios: the first involves non-sensitive query objects, where the inquirer is willing to disclose the query details to the data holder, aligning with the principles of Asymmetric Private Information Retrieval (**Asymmetric PIR**). In this setup, the inquirer seeks to determine the **nearest neighbor** (with respect to the **nearest distance**) between their non-confidential query object and the data possessed by the holder. Conversely, in the second scenario, the query object is sensitive and cannot be revealed to the data holder, adhering to Symmetric Private Information Retrieval (**Symmetric PIR**). 

Here, Homomorphic Encryption plays a pivotal role, facilitating secure multi-party computations that allow the inquirer to ascertain the desired distance without exposing the sensitive query object. This technology ensures that both types of queries can be processed efficiently while safeguarding the privacy of the involved parties (i.e., inquirer and data holders).

## Examples

In the following, we have provided two example codes for simulating the aforementioned application scenario.

### Example 1: Asymmetric Nearest Neighbor Query

#### 1.1 Problem Definition

Given a query object, the **nearest neighbor query** aims to retrieve the nearest data object from the dataset.
Intuitively, the **asymmetric nearest neighbor query** aims to obtain the nearest data object to the given query object from distributed data that are hold by different data holders. 

In this problem, a data object $o$ is defined as a multi-dimensional vector $\{x_1, x_2, \cdots, x_d\}$ with $d$ dimensions. Given two data objects $o = \{x_1, x_2, \cdots, x_d\}$ and $p = \{y_1, y_2, \cdots, y_d\}$, the euclidean distance is used to model their distance (i.e., similariy):  

$$dist(o, p) = \sqrt{\sum_{i=1}^{d}{(x_i-y_i)^2}}$$

Based on the above concepts, the formal problem definition of **Asymmetric Nearest Neighbor Query** (**ANNQ** as short) is given as follows (we assume that there're only two data holders for simplicity).
We are given **two** data holders (i.e., Alice and Bob) and **one** query user (i.e., Tom). Among these participants, Alice holds a set $X$ of data objects, 
and Bob holds a set $Y$ of data objects, where $X = \{o_1, o_2, \cdots, o_n\}$ and $Y = \{p_1, p_2, \cdots, p_n\}$.
The query user wants to find the nearest neighbor $NN$ in $X \cup Y$ to the query object $q$, where the set $X$ and $Y$ are assumed to be disjoint, i.e.,  

$$\forall o \in X, dist(NN, q) \le dist(o, q)$$
$$\forall p \in Y, dist(NN, q) \le dist(p, q)$$

Moreover, there are additional security requirements:  
+ For query user, Tom, the only thing he knows from Alice and Bob is $NN$;
+ For data holders, Alice and Bob, they can only know their own dataset;
+ The query object, $q$, can be revealed to any participant, both Alice and Bob.  

Notice: if there are more than two distinct objects in $X \cup Y$ that achieve the same nearest distance, any one can be retrieved as the query answer.

#### 1.2 Methodology: Partially Secure Algorithm (PSA)

To solve the **ANNQ** problem, we propose a secure and efficient algorithm as follows:  
1. The query user (Tom) sends the query object $q$ to the data holders (Alice and Bob);

2. After receiving the query object, both Alice and Bob can compute the nearest neighbor to the query object $q$ within their local dataset, say $o^{\ast}$ and $p^{\ast}$.

3. Next, we only need to **securely** pick the nearest one between $o^{\ast}$ and $p^{\ast}$, which is very similar to **The Millionaire Problem**. The key distinction lies in the fact that, in the ANNQ problem, neither Alice nor Bob can know the comparison result, whereas in the millionaire problem, either Alice or Bob is privy to the outcome of the comparison. Therefore, we can leverage **Homomorphic Encryption** (i.e., the BGV scheme in [Microsoft SEAL](https://github.com/microsoft/SEAL)) to tackle the problem as follows.

    + Key generation: the query user (Tom) generates the public key $pk$ and secret key $sk$, and he will broadcast the public key $pk$ to the data holders (Alice and Bob).

    + Distance difference encryption: both Alice and Bob can encrypt their nearest distance to the query object $q$: $E_{pk}[dist(o^{\ast},q)]$ and $E_{pk}[dist(p^{\ast},q)]$. After that, the encrypted distances will undergo subtraction (e.g., by Alice who receives $E_{pk}[dist(p^{\ast},q)]$ from Bob), and the (encrypted) result will be sent to the query user Tom.

    + Distance difference decryption: now, Tom receives the encrypted result and decrypt it with his secrypt key (the decrypted value is denoted as $\Delta$).

    + Request query answer: if $\Delta = dist(o^{\ast},q) - dist(p^{\ast},q) < 0$, the query user (Tom) will seek the nearest neighbor $o^{\ast}$ from Alice. Otherwise, he will seek the nearest neighbor $p^{\ast}$ from Bob.
  
$$E_{pk,sk}^{-1}(E_{pk}[dist(o^{\ast},q)] - E_{pk}[dist(p^{\ast},q)]) = dist(o^{\ast},q) - dist(p^{\ast},q)$$

**Analysis**: in the PSA algorithm, the last step inadvertently discloses additional information about $dist(p^{\ast},q)$ from Bob to the query user Tom, when $o^{\ast}$ from Alice is the nearest neighbor, and vice versa.

#### 1.3 Methodology: Fully Secure Algorithm (FSA)

In the FSA algorithm, we aim to prevent additional information leakage of the PSA algorithm by the following procedure.

+ Key generation: the query user (Tom) generates the public key $pk$ and secret key $sk$, and he will broadcast the public key $pk$ to the data holders (Alice and Bob).

+ Perturbed distance computation: for either Alice or Bob, he/she generates a positive, private, and random number ($a$ for Alice and $b$ for Bob). 
He/She will always keep the random number as a secret.
Then, he can peturb the distance and encrypt as follows:

$$\widetilde{dist}(o^{\ast},q) = a \cdot dist(o^{\ast},q) + a$$
$$\widetilde{dist}(p^{\ast},q) = b \cdot dist(p^{\ast},q) + b$$

Here, $\widetilde{dist}$ is used to denote the peturbed (plaintext) distance.

+ Exchange encrypted perturbed distance: Alice and Bob encrypt their peturbed distances $E_{pk}[\widetilde{dist}(o^{\ast},q)]$ and $E_{pk}[\widetilde{dist}(p^{\ast},q)]$. Then, they will exchange the encrypted data through network.

+ Double perturbed the encrypted distance: after receiving the encrypted data, Alice and Bob further peturb the encrypted data with their own secret number $a$ and $b$:

$$a \cdot E_{pk}[\widetilde{dist}(p^{\ast},q)] = E_{pk}[ab \cdot dist(p^{\ast},q) + ab]$$
$$b \cdot E_{pk}[\widetilde{dist}(o^{\ast},q)] = E_{pk}[ba \cdot dist(o^{\ast},q) + ba]$$

+ Subtract encrypted perturbed distance: 
Bob send $a \cdot E_{pk}[\widetilde{dist}(p^{\ast},q)]$ to Alice, and Alice will compute the difference of the encrypted perturbed distance:  

$$b \cdot E_{pk}[\widetilde{dist}(o^{\ast},q)] - a \cdot E_{pk}[\widetilde{dist}(p^{\ast},q)] = E_{pk}[ab \cdot (dist(o^{\ast},q) - dist(p^{\ast},q))]$$

+ Decrypt perturbed distance difference: now, Tom receives the encrypted result and decrypt it with his secrypt key (the decrypted value is denoted as $\Delta$).

$$E_{pk,sk}^{-1}(E_{pk}[ab \cdot dist(p^{\ast},q)]) = ab \cdot (dist(o^{\ast},q) - dist(p^{\ast},q))$$

+ Request query answer: if $\Delta = ab \cdot (dist(o^{\ast},q) - dist(p^{\ast},q)) < 0$ (where $a>0$ and $b>0$), the query user (Tom) will seek the nearest neighbor $o^{\ast}$ from Alice. Otherwise, he will seek the nearest neighbor $p^{\ast}$ from Bob.

**Analysis**: in the FSA algorithm, Tom can only know the peturbed distance (with unknown random number $a$ and $b$). Then, even if Tom and Alice collude, they cannot obtain $dist(p^{\ast},q)$, and vice versa.

#### 1.4 Experiment

1. Update the environment variables related to gRPC by the following command:
```
cd Lecture4_HE/asymmetric
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

3. Execute the following command in one terminal to enable the **query user** Tom:
```
./server Tom
```

4. Execute the following commands in other two terminals to enable the **data holder** Alice and Tom:
```
./client Alice
```
```
./client Bob
```
### Example 2: Symmetric Nearest Neighbor Query

#### 2.1 Problem Definition

Given a query object, the **nearest neighbor query** aims to retrieve the nearest data object from the dataset.
Intuitively, the **symmetric nearest neighbor query** aims to obtain the nearest data object to the given query object from distributed data that are hold by different data holders, while keeping both the query object and data object private.

In this problem, a data object $o$ is defined as a multi-dimensional vector $\{x_1, x_2, \cdots, x_d\}$ with $d$ dimensions. Given two data objects $o = \{x_1, x_2, \cdots, x_d\}$ and $p = \{y_1, y_2, \cdots, y_d\}$, the euclidean distance is used to model their distance (i.e., similariy):  

$$dist(o, p) = \sqrt{\sum_{i=1}^{d}{(x_i-y_i)^2}}$$

Based on the above concepts, the formal problem definition of **Symmetric Nearest Neighbor Query** (**SNNQ** as short) is given as follows (we assume that there're only two data holders for simplicity).
We are given **two** data holders (i.e., Alice and Bob) and **one** query user (i.e., Tom). Among these participants, Alice holds a set $X$ of data objects, 
and Bob holds a set $Y$ of data objects,
where $X = \{o_1, o_2, \cdots, o_n\}$ and $Y = \{p_1, p_2, \cdots, p_n\}$.
The query user wants to find the nearest neighbor $NN$ in $X \cup Y$ to the query object $q$, where the set $X$ and $Y$ are assumed to be disjoint, i.e.,

$$\forall o \in X, dist(NN, q) \le dist(o, q)$$
$$\forall p \in Y, dist(NN, q) \le dist(p, q)$$

Moreover, there are additional security requirements:  
+ For query user, Tom, the only thing he knows from Alice and Bob is $NN$;
+ For data holders, Alice and Bob, they can only know their own dataset;
+ The query object, $q$, **cannot** be revealed to any participant, either Alice or Bob.  

Notice: if there are more than two distinct objects in $X \cup Y$ that achieve the same nearest distance, any one can be retrieved as the query answer.

#### 2.2 Methodology: Naive Algorithm (NA)

To solve the **SNNQ** problem, we aim to follow the framework of the FSA algorithm. Thus, we need to address three technical challenges:

1. How to **securely** compute the distance between the query object and a data object from the data holders?

2. How to **securely** find the local nearest neighbor for each data holder?

3. How to **securely** pick the nearest neighbor between the local nearest ones from the data holders (Alice and Bob).

##### 2.2.1 Secure Distance Computation

Given two objects $o = \{x_1, x_2, \cdots, x_d\}$ and $p = \{y_1, y_2, \cdots, y_d\}$, we can compute the square euclidean distance instead of the square root, i.e.,

$$dist(o, p) = \sum_{i=1}^{d}{(x_i-y_i)^2}$$

Accordingly, we can first compute

$$L = \langle E_{pk}[x_1]-E_{pk}[y_1], E_{pk}[x_2]-E_{pk}[y_2], \cdots, E_{pk}[x_d]-E_{pk}[y_d] \rangle$$

Next, we can compute the inner product of $L$ and $L$, i.e.,

$$L^2 = \langle (E_{pk}[x_1]-E_{pk}[y_1])^2, (E_{pk}[x_2]-E_{pk}[y_2])^2, \cdots, (E_{pk}[x_d]-E_{pk}[y_d])^2 \rangle$$

Finally, the encrypted (square) distance is 

$$(E_{pk}[x_1]-E_{pk}[y_1])^2 + (E_{pk}[x_2]-E_{pk}[y_2])^2 + \cdots + (E_{pk}[x_d]-E_{pk}[y_d])^2$$

Based on this equation, Tom can first encrypt the query object $q$ with the public key and then send it to Alice or Bob. Alice and Bobe can compute the encrypted square distance based on the above equation and send it back to Tom.
After receiving the encrypted square distance, Tom can decrypt it with the secret key and obtain the square distance.

##### 2.2.2 Secure Local Nearest Neighbor

In general, sending the encrypted square distance to Tom will leak additional information about the data objects from Alice or Bob. Thus, instead of sending the distance, Alice or Bob can send the encrypted distance difference, i.e.,

$$E_{pk}[dist(o_i,q)] - E_{pk}[dist(o_j,q)]$$

When Bob receives the encrypted data, he will decrypt it and obtain the difference between the square distances for $o_i$ and $o_j$ to the query object $q$.
If the value is negative, it implies that $o_i$ is closer to $q$ than $o_j$.

By iteratively checking the distance difference via Tom, both Alice and Bob can obtain their local nearest neighbor.

##### 2.2.3 Secure Global Nearest Neighbor

As long as Alice and Bob have found their local nearest neighbor, we can further use the FSA algorithm to eventually pick the global nearest neighbor.
The main difference is that the query user Tom needs to send encrypted query object, i.e., $\{E_{pk}[z_1], E_{pk}[z_2], \cdots, E_{pk}[z_d]\}$, to Alice and Bob.
The other steps are almost identical to the last step of the FSA algorithm.

##### 2.2.4 Analysis

In the naive extension, **secure local nearest neighbor** inadvertently discloses additional information about the distance difference from either Alice or Bob to the query user Tom.
Moreover, it is time-consuming to securely compute the square distance. Is it possible that we can do better in both security and efficiency?

#### 2.3 Methodology: Optimized Algorithm (OA)

In the following, we first introduce how to enhance the security in Section 2.3.1 and then elaborate on how to improve the efficiency in Section 2.3.2.

##### 2.3.1 Enhance the Security

Similar to the NFA algorithm for ANNQ problem, we can use a random number to perturb the distance in order to reveal the plaintext distance.

+ Key generation: the query user (Tom) generates the public key $pk$ and secret key $sk$, and he will broadcast the public key $pk$ to a data holder (either Alice or Bob).

+ Perturbed distance computation: for the participant (e.g., Alice), she generates a positive, private, and random number ($a$ for Alice). 
She will always keep the random number as a secret.
Then, he can peturb the distance and encrypt as follows:

$$\widetilde{dist}(o_i,q) = a \cdot dist(o_i,q)$$
$$\widetilde{dist}(o_j,q) = a \cdot dist(o_j,q)$$

Here, $\widetilde{dist}$ is used to denote the peturbed (plaintext) square distance. Similarly, now the encrypted distance difference will be multiplied with a rando number $a$.

+ Decrypt perturbed distance difference: now, Tom receives the perturbed encrypted distance difference and decrypt it with his secrypt key (the decrypted value is denoted as $\Delta$).
+ 
$$E_{pk,sk}^{-1}(E_{pk}[a \cdot (dist(o_i,q) - dist(o_j,q))]) = a \cdot (dist(o_i,q) - dist(o_j,q))$$

+ Request query answer: if $\Delta = a \cdot (dist(o_i,q) - dist(o_j,q)) < 0$ (where $a>0$), the query user (Tom) will inform the comparison result $o_i \lhd o_j$ to the data holder Alice.

##### 2.3.2 Improve the Efficiency

The square euclidean distance is defined as

$$dist(o, q) = \sum_{i=1}^{d}{(x_i-z_i)^2}$$

Equivalently, it also equals

$$dist(o, q) = \sum_{i=1}^{d}{{x_i}^2} + \sum_{i=1}^{d}{{z_i}^2} - 2\sum_{i=1}^{d}{x_iz_i}$$

Therefore, the difference between the square distance $dist(o_i, p)$ and the square distance $dist(o_j, q)$ is  

$$
\begin{aligned}
dist(o_i, q) - dist(o_j, q) &= \Big(\sum_{i=1}^{d}{{x_i}^2} + \sum_{i=1}^{d}{{z_i}^2} - 2\sum_{i=1}^{d}{x_i z_i} \Big) - \Big(\sum_{i=1}^{d}{{y_i}^2} + \sum_{i=1}^{d}{{z_i}^2} - 2\sum_{i=1}^{d}{y_i z_i} \Big) \\
&= \sum_{i=1}^{d}{{x_i}^2} - \sum_{i=1}^{d}{{y_i}^2} - 2\sum_{i=1}^{d}{z_i (x_i-y_i)}
\end{aligned}
$$

where $o_i = \{x_1, x_2, \cdots, x_d\}$, $o_j = \{y_1, y_2, \cdots, y_d\}$, and $q = \{z_1, z_2, \cdots, z_d\}$.

By multiplying 0.5 in both left-hand side and right-hand side of the equation, we have

$$dist(o_i, q) - dist(o_j, q) = 0.5\sum_{i=1}^{d}{{x_i}^2} - 0.5\sum_{i=1}^{d}{{y_i}^2} - \sum_{i=1}^{d}{z_i (x_i-y_i)}$$

The first two terms can be first computed in plaintext and then encrypted with the public key. The last term can be computed by homomorphic encryption in encrypted data form. By using this way, we can save many secure computations.

##### 2.3.3 Analysis

In the OA algorithm, Tom can only know the peturbed distance (with an unknown random number $a$ or $b$). Then, he cannot derive anything else from Alice and Tom about their data objects.

#### 2.4 Experiment

1. Update the environment variables related to gRPC by the following command:
```
cd Lecture4_HE/symmetric
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

3. Execute the following command in one terminal to enable the **query user** Tom:
```
./server Tom
```

4. Execute the following commands in other two terminals to enable the **data holder** Alice and Tom:
```
./client Alice
```
```
./client Bob
