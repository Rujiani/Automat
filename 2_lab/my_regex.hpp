#ifndef MY_REGEX_HPP_
#define MY_REGEX_HPP_

#include "regex_compile/regex_tree.hpp"
#include "regex_compile/token.hpp"
#include "regex_compile/DKA.hpp"
#include <string>
#include <utility>
#include <variant>
#include <list>

#define tv tk.Tokens

using std::string;
using std::get;
using iter = std::list<mgr::TokenVariant>::iterator;

class RegexParserTest;

namespace mgr {

class regex {
public:
    RegexTree tr;

private:
    string prompt;

    TokenType GetTokenType(const TokenVariant& v) {
        TokenType res;
        std::visit([&res](const auto& tok) { res = tok.type; }, v);
        return res;
    }

    NodeType GetNodeTypeFromToken(const TokenType& tok);

    NodePtr ParseExpr();
    NodePtr ParseAlternation();
    NodePtr ParseConcat();
    NodePtr ParseRepeat();
    NodePtr ParseAtom();



public:
    DKA dka;
    Tokenizer tk;
    void TokenToTree() {
        if (tv.begin() == tv.end())
            throw std::logic_error("Token list is empty");
        tr.makeRoot<NodePtr>(ParseExpr());
    }
    inline regex(string str) : prompt(std::move(str)) {
        if (prompt.back() != '$')
            prompt.push_back('$');
    }

    inline void compile() {
        tk.Tokenize(prompt);
        TokenToTree();
        dka.TreeToDKA(tr);
        dka.minimize();
    }

    inline bool match(const string &str){
        return dka.match(str);
    }

    std::vector<std::pair<size_t, size_t>> findAll(const string&);
};

} // namespace mgr

#endif