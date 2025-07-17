#include <gtest/gtest.h>
#include "../regex_compile/token.hpp"

using namespace mgr;

namespace {

TokenType getType(const TokenVariant& t) {
    return std::visit([](auto&& tok) { return tok.type; }, t);
}

int getNumber(const TokenVariant& t) {
    return std::get<TokenNumber>(t).number;
}

char getSymbol(const TokenVariant& t) {
    return std::get<TokenSymbol>(t).symbol;
}

std::string getName(const TokenVariant& t) {
    return std::get<TokenName>(t).name;
}

TEST(TokenizerTest, BasicSymbols) {
    Tokenizer tk;
    auto& tokens = tk.Tokenize("a+*?|().$");

    ASSERT_EQ(tokens.size(), 9);

    auto it = tokens.begin();
    EXPECT_EQ(getType(*it++), TokenType::Literal);   // a
    EXPECT_EQ(getType(*it++), TokenType::Plus);      // +
    EXPECT_EQ(getType(*it++), TokenType::Star);      // *
    EXPECT_EQ(getType(*it++), TokenType::Question);  // ?
    EXPECT_EQ(getType(*it++), TokenType::Pipe);      // |
    EXPECT_EQ(getType(*it++), TokenType::LParen);    // (
    EXPECT_EQ(getType(*it++), TokenType::RParen);    // )
    EXPECT_EQ(getType(*it++), TokenType::Dot);       // .
    EXPECT_EQ(getType(*it++), TokenType::End);       // $
}

TEST(TokenizerTest, EscapedCharacters) {
    Tokenizer tk;
    auto& tokens = tk.Tokenize("a&+&*&?&|&.");

    ASSERT_EQ(tokens.size(), 6);

    auto it = tokens.begin();
    EXPECT_EQ(getType(*it++), TokenType::Literal);        // a
    EXPECT_EQ(getSymbol(*it++), '+');
    EXPECT_EQ(getSymbol(*it++), '*');
    EXPECT_EQ(getSymbol(*it++), '?');
    EXPECT_EQ(getSymbol(*it++), '|');
    EXPECT_EQ(getSymbol(*it++), '.');
}

TEST(TokenizerTest, CurlyRepeatForms) {
    Tokenizer tk1;
    auto& t1 = tk1.Tokenize("a{3}");
    ASSERT_EQ(t1.size(), 4);

    Tokenizer tk2;
    auto& t2 = tk2.Tokenize("a{1,3}");
    ASSERT_EQ(t2.size(), 6);

    Tokenizer tk3;
    auto& t3 = tk3.Tokenize("a{2,}");
    ASSERT_EQ(t3.size(), 6);

    Tokenizer tk4;
    auto& t4 = tk4.Tokenize("a{,4}");
    ASSERT_EQ(t4.size(), 5);
}

// TEST(TokenizerTest, NamedGroupSimple) {
//     Tokenizer tk;
//     auto& tokens = tk.Tokenize("(<g>a)<g>");

//     // Имя группы сейчас пока не парсится как отдельный тип
//     // Но если реализуешь `NamedGroupStart`, `NamedGroupRef`, то это будет работать
//     // Вот тогда это:
//     // ( → LParen
//     // <g> → NamedGroupStart g
//     // a → Literal
//     // ) → RParen
//     // <g> → NamedGroupRef g

//     // Пока что будет токенизировано как:
//     ASSERT_GE(tokens.size(), 6);  // если реализация добавлена
// }

// TEST(TokenizerTest, MixedComplex) {
//     Tokenizer tk;
//     auto& tokens = tk.Tokenize("a(&+)b{1,2}(<word>x|y)<word>$");

//     ASSERT_GE(tokens.size(), 14);  // зависит от реализации named-group
//     // Тестируй вручную, если группы пока не поддерживаются
// }

} // namespace
