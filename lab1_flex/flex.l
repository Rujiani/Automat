%{
#include <string>
#include <map>
std::map<std::string, size_t> hashMap; 

%}

%option noyywrap
%x HASH

%%
<INITIAL>{
^"ed2k://|file|"[a-zA-Z+\-_\.]+\|[0-9]+\| BEGIN(HASH);
.|\n {}
}
<HASH>{
(?i:[0-9a-f]{32}\|\/)\n {BEGIN(INITIAL); yyless(32);hashMap[std::string(yytext)]++;}
.|\n BEGIN(INITIAL);
}
%%

#include<iostream>
int main(){
    yylex();
    for(auto &i: hashMap){
        std::cout << i.first << " - " << i.second << '\n';
    }
    return 0;
}