#include <string>
#include <iostream>
#include <string>  
#include <cctype> // 用于std::tolower和std::toupper  
  
using namespace std;  
  
// 凯撒密码加密函数  
string caesarCipherEncrypt(const string& plain_data, int shift) {  
    string encrypt_data;
    for (size_t i=0,sz=plain_data.size(); i<sz; ++i) {
        char ch = toupper(plain_data[i]);
        if (isalpha(ch)) {  
            // 字母表循环  
            ch = 'A' + (ch - 'A' + shift) % 26;  
        }
        encrypt_data.push_back(ch);
    } 
    return encrypt_data;  
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
  
    string encrypt_data = caesarCipherEncrypt(input_data, shift);  
    cout << "Encrypted text: " << encrypt_data << endl;  
  
    return 0;  
}
