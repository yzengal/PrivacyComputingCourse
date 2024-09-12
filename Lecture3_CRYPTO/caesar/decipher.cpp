#include <iostream>  
#include <fstream>  
#include <sstream>  
#include <unordered_set>  
#include <string>  
#include <vector>  
#include <cctype>  
#include <algorithm>
  
using namespace std; 

// 把字符串s转化成小写格式
string ToLower(const std::string& s) {
    string ret;
    for (size_t i=0,sz=s.size(); i<sz; ++i) {
        ret.push_back(tolower(s[i]));
    }
    return ret;
}
  
// 读取词典文件，构建单词集合  
unordered_set<string> ReadDictionary(const string& file_path) {  
    unordered_set<string> dictionary;  
    ifstream file(file_path);  
     
    if (!file.is_open()) {  
        cerr << "Failed to open file: " << file_path << endl;  
        return dictionary;  
    }  
  
    // 跳过第一行（单词数量）  
    size_t word_num;
    file >> word_num;
    string line; 
    while (getline(file, line)) {
        line = ToLower(line);
        dictionary.insert(line);  
    }  
  
    file.close();  
    return dictionary;  
}  
  
// 凯撒密码解密函数  
string CaesarCipherDecrypt(const string& encrypt_data, int shift) {  
    string decrypt_data;  

    shift = 26 - shift;
    for (size_t i=0,sz=encrypt_data.size(); i<sz; ++i) {
        char ch = encrypt_data[i];
        if (isalpha(ch)) {  
            char base = isupper(ch) ? 'A' : 'a';  
            ch = (ch - base + shift) % 26 + base;
        }
        decrypt_data.push_back(ch);
    }  

    return decrypt_data;  
}  
  
// 检查解密后的字符串是否全为词典中的单词  
bool CheckDecryption(const string& decrypt_data, const unordered_set<string>& dictionary) {  
    istringstream iss(decrypt_data);  
    string word;  
    while (iss >> word) {  
        word = ToLower(word);
        if (dictionary.find(word) == dictionary.end()) {  
            return false;  
        }  
    }  
    return true;  
}  
  
int main(int argc, char** argv) {  
    string dictionaryPath = "../english-word.txt";  
    string encrypt_data = "Gnyx vf purnc fubj zr gur pbqr";  
    if (argc > 1) {
        encrypt_data.clear();
        for (int i=1; i<argc; ++i) {
            if (i > 1) encrypt_data += string(" ");
            encrypt_data += string(argv[i]);
        }
    }
  
    unordered_set<string> dictionary = ReadDictionary(dictionaryPath);
    bool decrypt_success = false;  

    cout << "Encrypt data: " << encrypt_data << endl;
  
    for (int shift = 1; shift < 26; ++shift) {  
        string decrypt_data = CaesarCipherDecrypt(encrypt_data, shift);  
        if (CheckDecryption(decrypt_data, dictionary)) {  
            cout << "Decrypt with shift " << shift << ": " << decrypt_data << endl;
            decrypt_success = true;
        }  
    }  
    
    if (false == decrypt_success)
        cout << "No decryption found." << endl;  

    return 0;  
}