#ifndef REGEX_TREE_HPP_
#define REGEX_TREE_HPP_

#include <variant>
#include <memory>
#include <vector>
#include <stdexcept>
#include <limits>

#define INFINITY std::numeric_limits<int>::max()

namespace mgr {

enum class NodeType {
    Repeat,
    Alternation,
    Wildcard,
    Literal,
    Epsilon,
    EmptySet,
    // Group,
    Concat,
    End
};

struct Repeat;
struct Alternation;
struct Wildcard;
struct Literal;
struct Epsilon;
struct EmptySet;
struct Concat;
struct End;



using Node = std::variant<Repeat, Alternation, Wildcard, Literal, Epsilon, EmptySet, Concat, End>;
using NodePtr = std::shared_ptr<Node>;

struct Repeat {
    NodeType Type = NodeType::Repeat;
    int min, max;
    NodePtr child;
    Repeat(int i, int a = INFINITY) : min(i), max(a) {
        if (i > a) throw std::invalid_argument("Invalid repeat bounds");
    }
};

struct Alternation {
    NodeType Type = NodeType::Alternation;
    std::vector<NodePtr> children;
};

struct Concat {
    NodeType Type = NodeType::Concat;
    std::vector<NodePtr> children;
};

struct Wildcard {
    NodeType Type = NodeType::Wildcard;
};

struct End {
    NodeType Type = NodeType::End;
};

struct Literal {
    NodeType Type = NodeType::Literal;
    char value;
    explicit Literal(char ch) : value(ch) {}
};

struct Epsilon {
    NodeType Type = NodeType::Epsilon;
};
struct EmptySet {
    NodeType Type = NodeType::EmptySet;
};

class regex;

class RegexTree {
public:
    RegexTree() = default;
    NodePtr root;

    template<typename T, typename... Args>
    NodePtr makeNode(NodePtr& parent, Args&&... args) {
        auto node = std::make_shared<Node>(T(std::forward<Args>(args)...));
        addKid(node, parent);
        return node;
    }

    template<typename T, typename... Args>
    void makeRoot(Args&&... args) {
        root = T(std::forward<Args>(args)...);
    }

    void makeRoot(NodePtr& ptr) {
        root = ptr;
    }

    void makeRoot(NodePtr&& ptr) {
        root = std::move(ptr);
    }

    void addKid(const NodePtr kid, const NodePtr parent) {
        if (auto* ptr = std::get_if<Repeat>(parent.get())) {
            ptr->child = kid;
        }
        else if (auto* ptr = std::get_if<Alternation>(parent.get())) {
            ptr->children.emplace_back(kid);
        } else if(auto* ptr = std::get_if<Concat>(parent.get())){
            ptr->children.emplace_back(kid);
        } else {
            throw std::logic_error("This node type cannot accept children");
        }
    }
    inline NodePtr getRoot() const {
        return root;
    }
};

inline NodeType getType(const mgr::NodePtr& node) {
    return std::visit([](auto&& arg) -> NodeType {
        return arg.Type;
    }, *node);
}

} // namespace mgr

#endif // REGEX_TREE_HPP_
