#include "my_regex.hpp"
#include <iostream>

int main(){
    mgr::regex rx("(M+(e+)?p+|(h+)?i)$");
    rx.compile();
    std:: cout << rx.match("MMMMMMMMMMMMMMMMMp") << '\n';
    std:: cout << rx.match("Mepp") << '\n';
    std:: cout << rx.match("Mp") << '\n';
    std:: cout << rx.match("i") << '\n';
    std:: cout << rx.match("hi") << '\n';
    std:: cout << rx.match("hhhi") << '\n';
}