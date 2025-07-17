#ifndef DKA_HPP_
#define DKA_HPP_

#include <forward_list>
#include <vector>
#include <unordered_set>
#include <string>
#include "regex_tree.hpp"

namespace mgr {

    class DKA {
    public:

        struct Transition{
            char from = ' ', to = '~';
            size_t target;
        };

        struct State{
            std::forward_list<Transition> transitions;
            bool is_final = false;
        };

        std::vector<State> states;
        size_t start_state = 0;

        inline size_t addState(bool is_final = false) {
            states.push_back(State{ {}, is_final });
            return states.size() - 1;
        }

        inline void addTransition(size_t from, char c1, char c2, size_t to) {
            // std::cerr << from << "->" << to << '\n';
            states[from].transitions.push_front(Transition{ c1, c2, to });
        }

        void TreeToDKA(const RegexTree &rt);
        void minimize();
        bool match(const std::string& str) const;
        std::string to_regex()const;
        void complete();
        DKA complement() const;
        DKA intersect(const DKA& other) const;
        DKA operator-(const DKA& other) const;

        private:
        using Frontier = std::unordered_set<size_t>;
        Frontier TreeToDKA_Helper(const NodePtr& node, Frontier from);
        Frontier addOnce(const NodePtr& leaf, Frontier from);
        Frontier addRepeat(const Repeat& rep, Frontier from);
        Frontier addAlternation(const Alternation& alt, Frontier from);
    };

}

#endif