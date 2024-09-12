#include <string>
#include <iostream>
#include <string>  
#include <cctype> // 用于std::tolower和std::toupper  
  
using namespace std;  
  
// 凯撒密码加密函数  
string CaesarCipherEncrypt(const string& plain_data, int shift) {  
    string encrypt_data;
    for (size_t i=0,sz=plain_data.size(); i<sz; ++i) {
        char ch = plain_data[i];
        if (isalpha(ch)) {  
            // 字母表循环 
            char base = isupper(ch) ? 'A' : 'a';   
            ch = (ch - base + shift) % 26 + base;  
        }
        encrypt_data.push_back(ch);
    } 
    return encrypt_data;  
} 

// 凯撒密码解密函数  
string CaesarCipherDecrypt(const string& encrypt_data, int shift) {  
    string plain_data;
    shift = 26 - shift;
    for (size_t i=0,sz=encrypt_data.size(); i<sz; ++i) {
        char ch = encrypt_data[i];
        if (isalpha(ch)) {  
            // 字母表循环  
            char base = isupper(ch) ? 'A' : 'a';   
            ch = (ch - base + shift) % 26 + base;   
        }
        plain_data.push_back(ch);
    } 
    return plain_data;  
}  
  
int main(int argc, char* argv[]) {  
    if (argc != 3 || argv[1][0] != '-' || argv[1][1] != 'k') {  
        cerr << "Usage: " << argv[0] << " -k <shift size>" << endl;  
        return -1;  
    }  
   
    int shift = stoi(argv[2]); // 将偏移量字符串转换为整数  
  
    string input_data;  
    cout << "Enter the text to encrypt: ";  
    getline(cin, input_data); // 从标准输入读取字符串  
  
    string encrypt_data = CaesarCipherEncrypt(input_data, shift);  
    cout << "Encrypted text: " << encrypt_data << endl;  

    string decrypt_data = CaesarCipherDecrypt(encrypt_data, shift);  
    cout << "Decrypted text: " << decrypt_data << endl; 
  
    return 0;  
}
