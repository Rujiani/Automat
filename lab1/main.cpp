#include "AppClass.hpp"
#include <iostream>
#include <string>
#include <unordered_map>

int main(){
    AppClass _fsm;
    std::unordered_map<std::string, size_t> map;
    std::string str;
    while(std::cin >> str){
        if(_fsm.CheckString(str.c_str()))
            map[_fsm.getHashBuff()]++;
    }
    
    for(auto &pair : map){
        std::cout << pair.first << " - " << pair.second << '\n';
    }
    return 0;
}
