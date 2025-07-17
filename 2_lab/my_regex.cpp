#include "my_regex.hpp"
#include "regex_compile/regex_tree.hpp"
#include "regex_compile/token.hpp"
#include <memory>
#include <stdexcept>

namespace mgr {
    NodePtr regex::ParseExpr() {
        auto root = ParseAlternation();
        return root;
    }

    NodePtr regex::ParseAlternation() {
        auto left = ParseConcat();
        while (!tv.empty() && GetTokenType(tv.front()) == TokenType::Pipe) {
            tv.pop_front();
            auto right = ParseConcat();
            Alternation alt;
            alt.children.push_back(left);
            alt.children.push_back(right);
            left = std::make_shared<Node>(std::move(alt));
        }
        return left;
    }

    NodePtr regex::ParseConcat() {
        std::vector<NodePtr> nodes;

        while (!tv.empty()) {
            TokenType type = GetTokenType(tv.front());
            if (type == TokenType::Pipe || type == TokenType::RParen || type == TokenType::End)
                break;

            nodes.push_back(ParseRepeat());
        }

        if (nodes.empty())
            throw std::logic_error("Error: Empty concat");

        if (!tv.empty() && GetTokenType(tv.front()) == TokenType::End) {
            tv.pop_front();
            nodes.push_back(std::make_shared<Node>(End{}));
        }

        Concat c;
        c.children = std::move(nodes);
        return std::make_shared<Node>(std::move(c));
    }

    NodePtr regex::ParseRepeat() {
        auto node = ParseAtom();
        if (tv.empty()) return node;

        TokenType type = GetTokenType(tv.front());
        int min = 0, max = INFINITY;

        if (type == TokenType::Plus) {
            min = 1; max = INFINITY;
            tv.pop_front();
        } else if (type == TokenType::Star) {
            min = 0; max = INFINITY;
            tv.pop_front();
        } else if (type == TokenType::Question) {
            min = 0; max = 1;
            tv.pop_front();
        } else if (type == TokenType::LCurly) {
            tv.pop_front();

            if (GetTokenType(tv.front()) == TokenType::Comma) {
                tv.pop_front();
                min = 0;
                if (!std::holds_alternative<TokenNumber>(tv.front()))
                    throw std::logic_error("Expected upper bound after ','");
                max = std::get<TokenNumber>(tv.front()).number;
                tv.pop_front();
            } else if (std::holds_alternative<TokenNumber>(tv.front())) {
                min = std::get<TokenNumber>(tv.front()).number;
                tv.pop_front();

                if (GetTokenType(tv.front()) == TokenType::Comma) {
                    tv.pop_front();
                    if (std::holds_alternative<TokenNumber>(tv.front())) {
                        max = std::get<TokenNumber>(tv.front()).number;
                        tv.pop_front();
                    } else {
                        max = INFINITY;
                    }
                } else {
                    max = min;
                }
            } else {
                throw std::logic_error("Expected number or ',' after '{'");
            }

            if (GetTokenType(tv.front()) != TokenType::RCurly)
                throw std::logic_error("Expected '}' after repeat bounds");
            tv.pop_front();
        } else {
            return node;
        }

        auto repeat = std::make_shared<Node>(Repeat{min, max});
        tr.addKid(node, repeat);
        return repeat;
    }

    NodePtr regex::ParseAtom() {
        if (tv.empty())
            throw std::logic_error("Unexpected end in ParseAtom");

        auto tok = tv.front();
        tv.pop_front();

        switch (GetTokenType(tok)) {
            case TokenType::Literal:
            case TokenType::Escape:
                return std::make_shared<Node>(Literal{std::get<TokenSymbol>(tok).symbol});
            case TokenType::Dot:
                return std::make_shared<Node>(Wildcard{});
            case TokenType::End:
                return std::make_shared<Node>(End{});
            case TokenType::LParen: {
                auto inner = ParseExpr();
                if (tv.empty() || GetTokenType(tv.front()) != TokenType::RParen)
                    throw std::logic_error("Expected closing ')'");
                tv.pop_front();
                return inner;
            }
            default:
                throw std::logic_error("Unexpected token in ParseAtom");
        }
    }

}