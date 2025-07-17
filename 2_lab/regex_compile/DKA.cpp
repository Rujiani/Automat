#include "DKA.hpp"
#include "regex_tree.hpp"
#include <stdexcept>
#include <unordered_set>
#include <variant>
#include <set>
#include <map>
#include <queue>

namespace mgr {
    bool DKA::match(const std::string& str) const {
        size_t current = start_state;
        size_t counter = 0;
        for (char ch : str) {
            bool advanced = false;
            for (const auto& tr : states[current].transitions) {
                // std::cerr << tr.from << ':' << tr.to << '\n' << counter++ << '\n';
                if (ch >= tr.from && ch <= tr.to) {
                    current = tr.target;
                    advanced = true;
                    break;
                }
            }
            if (!advanced) return false;
        }
        return states[current].is_final;
    }

    using Frontier = std::unordered_set<size_t>;

    DKA::Frontier DKA::addOnce(const NodePtr& node, Frontier from) {
        NodeType type = getType(node);

        switch (type) {
            case NodeType::Literal:
            case NodeType::Wildcard: {
                Frontier next;
                for (size_t s : from) {
                    size_t t = addState();
                    if (auto* lit = std::get_if<Literal>(node.get()))
                        addTransition(s, lit->value, lit->value, t);
                    else
                        addTransition(s, ' ', '~', t);
                    next.insert(t);
                }
                return next;
            }

            case NodeType::Repeat:
                return addRepeat(std::get<Repeat>(*node), from);

            case NodeType::Alternation:
                return addAlternation(std::get<Alternation>(*node), from);

            case NodeType::Concat: {
                Frontier cur = from;
                for (const auto& child : std::get<Concat>(*node).children)
                    cur = addOnce(child, cur);
                return cur;
            }

            case NodeType::End:
            case NodeType::Epsilon:
                return from;

            default:
                throw std::logic_error("Unsupported node type in addOnce");
        }
    }

    DKA::Frontier DKA::addRepeat(const Repeat& rep, Frontier from)
    {
        Frontier entry = from;
        for (int i = 0; i < rep.min; ++i)
            entry = addOnce(rep.child, entry);

        Frontier exits = addOnce(rep.child, entry);

        if (rep.max == INFINITY) {
            for (size_t src : exits) {
                for (size_t e : entry) {
                    for (const auto& tr : states[e].transitions)
                        addTransition(src, tr.from, tr.to, tr.target);
                }
            }
            exits.insert(entry.begin(), entry.end());
            return exits;
        }

        for (int i = 0; i < rep.max - rep.min - 1; ++i) {
            exits = addOnce(rep.child, exits);
        }
        exits.insert(entry.begin(), entry.end());
        return exits;
    }

    DKA::Frontier DKA::addAlternation(const Alternation& alt, Frontier from)
    {
        Frontier merged;
        for (const auto& branch : alt.children) {
            Frontier copy = from;
            merged.merge(addOnce(branch, copy));
        }
        return merged;
    }



    Frontier DKA::TreeToDKA_Helper(const NodePtr& node, Frontier from) {
        NodeType type = getType(node);

        switch (type) {
            case NodeType::Literal:
            case NodeType::Wildcard:
                return addOnce(node, from);

            case NodeType::Repeat:
                return addRepeat(std::get<Repeat>(*node), from);

            case NodeType::Alternation: {
                Frontier result;
                for (auto& child : std::get<Alternation>(*node).children) {
                    Frontier copy = from;
                    result.merge(TreeToDKA_Helper(child, copy));
                }
                return result;
            }

            case NodeType::Concat: {
                Frontier cur = from;
                for (auto& child : std::get<Concat>(*node).children) {
                    cur = TreeToDKA_Helper(child, cur);
                }
                return cur;
            }

            case NodeType::End: {
                for (size_t s : from)
                    states[s].is_final = true;
                return from;
            }

            case NodeType::Epsilon:
                return from;

            case NodeType::EmptySet:
                return {};

            default:
                throw std::logic_error("Unknown node type in TreeToDKA_Helper");
        }
    }


    void DKA::TreeToDKA(const RegexTree& rt) {
        if (!rt.root)
            throw std::logic_error("Regex tree is empty");

        addState();
        Frontier start{ start_state };
        TreeToDKA_Helper(rt.root, start);
    }


    void DKA::minimize() {
        size_t n = states.size();
        if (n <= 1) return;

        std::set<char> alphabet;
        for (const auto& st : states)
            for (const auto& tr : st.transitions)
                for (char c = tr.from; c <= tr.to; ++c)
                    alphabet.insert(c);

        std::vector<std::set<size_t>> partitions;
        std::map<size_t, size_t> state_to_class;

        std::set<size_t> final, non_final;
        for (size_t i = 0; i < n; ++i)
            (states[i].is_final ? final : non_final).insert(i);

        if (!final.empty()) partitions.push_back(final);
        if (!non_final.empty()) partitions.push_back(non_final);

        for (size_t i = 0; i < partitions.size(); ++i)
            for (size_t s : partitions[i])
                state_to_class[s] = i;

        bool changed;
        do {
            changed = false;
            std::vector<std::set<size_t>> new_partitions;

            for (const auto& cls : partitions) {
                std::map<std::vector<size_t>, std::set<size_t>> splitter;

                for (size_t s : cls) {
                    std::vector<size_t> signature;
                    for (char c : alphabet) {
                        size_t next = n + 1;
                        for (const auto& tr : states[s].transitions)
                            if (c >= tr.from && c <= tr.to) {
                                next = state_to_class[tr.target];
                                break;
                            }
                        signature.push_back(next);
                    }
                    splitter[signature].insert(s);
                }

                if (splitter.size() == 1) {
                    new_partitions.push_back(cls);
                } else {
                    changed = true;
                    for (auto& [_, group] : splitter)
                        new_partitions.push_back(std::move(group));
                }
            }

            partitions = std::move(new_partitions);
            state_to_class.clear();
            for (size_t i = 0; i < partitions.size(); ++i)
                for (size_t s : partitions[i])
                    state_to_class[s] = i;

        } while (changed);

        std::vector<State> new_states(partitions.size());
        for (size_t i = 0; i < partitions.size(); ++i) {
            size_t repr = *partitions[i].begin(); // representative
            new_states[i].is_final = states[repr].is_final;
            for (const auto& tr : states[repr].transitions)
                new_states[i].transitions.push_front(Transition{
                    tr.from, tr.to, state_to_class[tr.target]
                });
        }

        start_state = state_to_class[start_state];
        states = std::move(new_states);
    }

        static std::string concat(const std::string& a, const std::string& b) {
        if (a.empty()) return b;
        if (b.empty()) return a;
        return a + b;
    }

    static std::string alt(const std::string& a, const std::string& b) {
        if (a.empty()) return b;
        if (b.empty()) return a;
        if (a == b) return a;
        return "(" + a + "|" + b + ")";
    }

    static std::string star(const std::string& a) {
        if (a.empty()) return "";
        return "(" + a + ")*";
    }

    std::string DKA::to_regex() const
    {
        std::vector<bool> reachable(states.size(), false);
        std::queue<size_t> q;
        q.push(start_state);
        reachable[start_state] = true;
        while (!q.empty()) {
            size_t s = q.front(); q.pop();
            for (auto& tr : states[s].transitions)
                if (!reachable[tr.target]) {
                    reachable[tr.target] = true;
                    q.push(tr.target);
                }
        }

        std::vector<size_t> mapOld2New(states.size(), SIZE_MAX);
        std::vector<State>  compact;
        for (size_t i = 0; i < states.size(); ++i)
            if (reachable[i]) {
                mapOld2New[i] = compact.size();
                compact.push_back(states[i]);
            }

        DKA tmp;
        tmp.states      = std::move(compact);
        tmp.start_state = mapOld2New[start_state];
        tmp.minimize();

        const size_t N = tmp.states.size();
        if (N == 0) return "";

        size_t start = tmp.start_state;
        size_t final = 0;
        for (size_t i = 0; i < N; ++i)
            if (tmp.states[i].is_final) { final = i; break; }

        std::vector<std::vector<std::string>> R(N, std::vector<std::string>(N, ""));
        auto capped = [](std::string s) {
            const size_t LIM = 2048;
            return (s.size() > LIM) ? std::string("#LONG#") : s;
        };

        for (size_t i = 0; i < N; ++i)
            for (auto& tr : tmp.states[i].transitions) {
                std::string lbl = (tr.from == tr.to) ? std::string(1, tr.from) : ".";
                R[i][tr.target] = lbl;
            }

        auto alt = [](const std::string& a,const std::string& b){
            if (a.empty()) return b;
            if (b.empty()||a==b) return a;
            return "(" + a + "|" + b + ")";
        };
        auto concat = [](const std::string& a,const std::string& b){
            if (a.empty()) return b;
            if (b.empty()) return a;
            return a + b;
        };
        auto star = [](const std::string& a){
            if (a.empty() || a == "#LONG#") return a;
            return "(" + a + ")*";
        };

        for (size_t k = 0; k < N; ++k)
            for (size_t i = 0; i < N; ++i)
                for (size_t j = 0; j < N; ++j) {
                    std::string part = concat(R[i][k], concat(star(R[k][k]), R[k][j]));
                    R[i][j] = capped(alt(R[i][j], part));
                }

        return R[start][final];
    }


    void DKA::complete() {
        size_t N = states.size();

        const int ALPHABET_SIZE = '~' - ' ' + 1;
        std::vector<std::vector<bool>> covered(N, std::vector<bool>(ALPHABET_SIZE, false));

        for (size_t i = 0; i < N; ++i) {
            for (const auto& tr : states[i].transitions) {
                for (char c = tr.from; c <= tr.to; ++c) {
                    covered[i][c - ' '] = true;
                }
            }
        }

        size_t sink = addState(false);
        N = states.size();

        for (size_t i = 0; i < N - 1; ++i) {
            for (char c = ' '; c <= '~'; ++c) {
                if (!covered[i][c - ' ']) {
                    addTransition(i, c, c, sink);
                }
            }
        }

        for (char c = ' '; c <= '~'; ++c) {
            addTransition(sink, c, c, sink);
        }
    }




    DKA DKA::complement() const {
        DKA result = *this;
        result.complete();
        for (auto& state : result.states)
            state.is_final = !state.is_final;
        return result;
    }

    DKA DKA::intersect(const DKA& other) const {
        DKA result;
        using Pair = std::pair<size_t, size_t>;
        std::map<Pair, size_t> state_map;
        std::queue<Pair> q;

        auto get_state = [&](Pair p) -> size_t {
            if (state_map.count(p)) return state_map[p];
            bool final = states[p.first].is_final && other.states[p.second].is_final;
            size_t id = result.addState(final);
            state_map[p] = id;
            q.push(p);
            return id;
        };

        result.start_state = get_state({start_state, other.start_state});

        while (!q.empty()) {
            auto [s1, s2] = q.front(); q.pop();
            size_t id = state_map[{s1, s2}];

            for (const auto& tr1 : states[s1].transitions) {
                for (const auto& tr2 : other.states[s2].transitions) {
                    char from = std::max(tr1.from, tr2.from);
                    char to   = std::min(tr1.to, tr2.to);
                    if (from <= to) {
                        size_t tgt = get_state({tr1.target, tr2.target});
                        result.addTransition(id, from, to, tgt);
                    }
                }
            }
        }

        return result;
    }

    DKA DKA::operator-(const DKA& other) const {
        return this->intersect(other.complement());
    }


}