#include "token.hpp"
#include <limits>
#include <stdexcept>
#include <unordered_set>
#include <charconv>
#include <system_error>

namespace mgr {

static void parseCurly(size_t &i, const std::string &str, std::list<TokenVariant> &Tokens) {
    size_t posStart = i + 1;
    size_t posEnd = str.find('}', i);
    if (posEnd == std::string::npos || posEnd <= posStart)
        throw std::invalid_argument("Incorrect string: Curly Error");
    i = posEnd;

    Tokens.emplace_back(TokenSimple{TokenType::LCurly});

    const char* start = str.data() + posStart;
    const char* end = str.data() + posEnd;

    if (*start == ',') {
        // {,y}
        Tokens.emplace_back(TokenSimple{TokenType::Comma});
        int y;
        auto [ptr2, err2] = std::from_chars(start + 1, end, y);
        if (err2 != std::errc())
            throw std::invalid_argument("Invalid {,y} bounds");
        Tokens.emplace_back(TokenNumber{TokenType::Number, y});
    } else {
        // {x} or {x,} or {x,y}
        int x;
        auto [ptr, err] = std::from_chars(start, end, x);
        if (err != std::errc())
            throw std::invalid_argument("Invalid {x} or {x,y} bounds");

        Tokens.emplace_back(TokenNumber{TokenType::Number, x});

        if (*ptr == ',') {
            Tokens.emplace_back(TokenSimple{TokenType::Comma});
            int y;
            auto [ptr2, err2] = std::from_chars(ptr + 1, end, y);
            if (err2 != std::errc()) {
                Tokens.emplace_back(TokenNumber{TokenType::Number, std::numeric_limits<int>::max()});  // {x,}
            } else {
                Tokens.emplace_back(TokenNumber{TokenType::Number, y});  // {x,y}
            }
        }
    }

    Tokens.emplace_back(TokenSimple{TokenType::RCurly});
}

std::list<TokenVariant>& Tokenizer::Tokenize(const std::string& str) {
    std::unordered_set<std::string> groups;
    Tokens.clear();
    bool isOpenParen = false;
    for (size_t i = 0; i < str.size(); i++) {
        switch (str[i]) {
            case '&':
                if (i + 1 >= str.size())
                    throw std::invalid_argument("Dangling escape '&' at end");
                Tokens.emplace_back(TokenSymbol{TokenType::Escape, str[++i]});
                break;
            case '|':
                Tokens.emplace_back(TokenSimple{TokenType::Pipe});
                break;
            case '.':
                Tokens.emplace_back(TokenSimple{TokenType::Dot});
                break;
            case '+':
                Tokens.emplace_back(TokenSimple{TokenType::Plus});
                break;
            case '*':
                Tokens.emplace_back(TokenSimple{TokenType::Star});
                break;
            case '?':
                Tokens.emplace_back(TokenSimple{TokenType::Question});
                break;
            case '$':
                if(i < str.size() - 1)
                    throw std::invalid_argument("Incorrect string: found $(EOL) before end");
                Tokens.emplace_back(TokenSimple{TokenType::End});
                break;
            case '{':
                parseCurly(i, str, Tokens);
                break;
            case '(':
                Tokens.emplace_back(TokenSimple{TokenType::LParen});
                break;
            case ')':
                Tokens.emplace_back(TokenSimple{TokenType::RParen});
                break;
            default:
                Tokens.emplace_back(TokenSymbol{TokenType::Literal, str[i]});
                break;
        }
    }
    return Tokens;
}

} // namespace mgr
