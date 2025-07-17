#include <iostream>
#include <vector>
#include "DK.hpp"

#define a 0
#define b 1

void check(bool cond){
    if(cond)
        std::cout << "Correct\n";
    else  
        std::cout << "Incorrect\n";

}

using namespace MGA;

int main(){
    std::vector<int> word1 = {a, a, b, a}, w2 {b, b, a, a}, w3 {b, a, a, a}, w4{b, b, b, a, a};
   DK dk(5, 2, {-1, 1, 2, 3, 2, -1, 4, -1, 4, -1});
    check(dk.apply_word(word1) == false);
    check(dk.apply_word(w2) == true);
    check(dk.apply_word(w3) == true);
    check(dk.apply_word(w4) == false);
}