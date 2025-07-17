#ifndef Token_HPP_
#define Token_HPP_

#include <variant>
#include <list>
#include <string>

namespace mgr {

enum class TokenType {
    Literal,
    Escape,

    Pipe,
    Dot,
    Plus,
    Question,
    Star,

    LParen,
    RParen,
    LCurly,
    RCurly,

    Comma,
    Number,

    // GroupCreate,
    // GroupRef,
    End
};

// tokens type:
struct TokenSimple {
    TokenType type;
};

struct TokenSymbol {
    TokenType type;
    char symbol;
};

struct TokenNumber {
    TokenType type;
    int number;
};

struct TokenName {
    TokenType type;
    std::string name;
};

using TokenVariant = std::variant<TokenSimple, TokenSymbol, TokenNumber, TokenName>;

class regex;

class Tokenizer {
    friend regex;

public:
    Tokenizer() = default;

    std::list<TokenVariant> Tokens;
    std::list<TokenVariant>& Tokenize(const std::string& str);
};

} // namespace mgr

#endif
