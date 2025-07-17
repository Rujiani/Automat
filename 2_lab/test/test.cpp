#include <gtest/gtest.h>
#include "../my_regex.hpp"
using namespace mgr;

TEST(RegexTreeTest, LiteralAndEnd) {
    regex r("a$");
    r.compile();
    auto root = r.tr.getRoot();
    ASSERT_EQ(getType(root), NodeType::Concat);
    auto& children = std::get<Concat>(*root).children;
    ASSERT_EQ(children.size(), 2);
    ASSERT_EQ(getType(children[0]), NodeType::Literal);
    ASSERT_EQ(getType(children[1]), NodeType::End);
    ASSERT_EQ(std::get<Literal>(*children[0]).value, 'a');
}

TEST(RegexTreeTest, AlternationNestedConcat) {
    regex r("ab|cd$");
    r.compile();
    auto root = r.tr.getRoot();
    ASSERT_EQ(getType(root), NodeType::Alternation);
    auto& children = std::get<Alternation>(*root).children;
    ASSERT_EQ(children.size(), 2);
    for (auto& c : children)
        ASSERT_EQ(getType(c), NodeType::Concat);
}

TEST(RegexTreeTest, ComplexRepeatAndDot) {
    regex r("((a|b).)*$");
    r.compile();
    auto root = r.tr.getRoot();
    ASSERT_EQ(getType(root), NodeType::Concat);

    auto& cc = std::get<Concat>(*root).children;
    ASSERT_EQ(cc.size(), 2);
    ASSERT_EQ(getType(cc[1]), NodeType::End);
    ASSERT_EQ(getType(cc[0]), NodeType::Repeat);

    auto& rep = std::get<Repeat>(*cc[0]);
    ASSERT_EQ(rep.min, 0);
    ASSERT_EQ(rep.max, INFINITY);
    ASSERT_EQ(getType(rep.child), NodeType::Concat);

    auto& sub = std::get<Concat>(*rep.child).children;
    ASSERT_EQ(getType(sub[0]), NodeType::Alternation);
    ASSERT_EQ(getType(sub[1]), NodeType::Wildcard);
}

TEST(RegexTreeTest, RepeatBounded) {
    regex r("a{2,4}$");
    r.compile();
    auto root = r.tr.getRoot();
    ASSERT_EQ(getType(root), NodeType::Concat);

    auto& children = std::get<Concat>(*root).children;
    ASSERT_EQ(getType(children[0]), NodeType::Repeat);
    ASSERT_EQ(getType(children[1]), NodeType::End);

    auto& rep = std::get<Repeat>(*children[0]);
    ASSERT_EQ(rep.min, 2);
    ASSERT_EQ(rep.max, 4);
    ASSERT_EQ(getType(rep.child), NodeType::Literal);
    ASSERT_EQ(std::get<Literal>(*rep.child).value, 'a');
}

TEST(RegexTreeTest, OptionalGroup) {
    regex r("(a|b)?$");
    r.compile();
    auto root = r.tr.getRoot();
    ASSERT_EQ(getType(root), NodeType::Concat);

    auto& children = std::get<Concat>(*root).children;
    ASSERT_EQ(getType(children[0]), NodeType::Repeat);
    ASSERT_EQ(getType(children[1]), NodeType::End);

    auto& rep = std::get<Repeat>(*children[0]);
    ASSERT_EQ(rep.min, 0);
    ASSERT_EQ(rep.max, 1);
    ASSERT_EQ(getType(rep.child), NodeType::Alternation);
}

TEST(RegexTreeTest, DeeplyNested) {
    regex r("((a|b)c(d|e))*$");
    r.compile();
    auto root = r.tr.getRoot();
    ASSERT_EQ(getType(root), NodeType::Concat);

    auto& concat = std::get<Concat>(*root).children;
    ASSERT_EQ(getType(concat[1]), NodeType::End);
    ASSERT_EQ(getType(concat[0]), NodeType::Repeat);

    auto& repeat = std::get<Repeat>(*concat[0]);
    auto& inner = std::get<Concat>(*repeat.child).children;

    ASSERT_EQ(getType(inner[0]), NodeType::Alternation);
    ASSERT_EQ(getType(inner[1]), NodeType::Literal);
    ASSERT_EQ(getType(inner[2]), NodeType::Alternation);
    ASSERT_EQ(std::get<Literal>(*inner[1]).value, 'c');
}

TEST(RegexTreeTest, RepeatOfComplexGroup) {
    regex r("((a|b)(c|d)){1,3}$");
    r.compile();
    auto root = r.tr.getRoot();
    ASSERT_EQ(getType(root), NodeType::Concat);

    auto& cc = std::get<Concat>(*root).children;
    ASSERT_EQ(getType(cc[0]), NodeType::Repeat);
    ASSERT_EQ(getType(cc[1]), NodeType::End);

    auto& rep = std::get<Repeat>(*cc[0]);
    ASSERT_EQ(rep.min, 1);
    ASSERT_EQ(rep.max, 3);

    auto& group = std::get<Concat>(*rep.child).children;
    ASSERT_EQ(getType(group[0]), NodeType::Alternation);
    ASSERT_EQ(getType(group[1]), NodeType::Alternation);
}

TEST(RegexTreeTest, AlternationGroupAndLiteral) {
    regex r("(ab|c)$");
    r.compile();
    auto root = r.tr.getRoot();
    ASSERT_EQ(getType(root), NodeType::Concat);

    auto& concat = std::get<Concat>(*root).children;
    ASSERT_EQ(getType(concat[1]), NodeType::End);
    ASSERT_EQ(getType(concat[0]), NodeType::Alternation);

    auto& alt = std::get<Alternation>(*concat[0]).children;
    ASSERT_EQ(getType(alt[0]), NodeType::Concat);
    ASSERT_EQ(getType(alt[1]), NodeType::Concat); // было: Literal

    auto& left = std::get<Concat>(*alt[0]).children;
    ASSERT_EQ(getType(left[0]), NodeType::Literal);
    ASSERT_EQ(std::get<Literal>(*left[0]).value, 'a');
    ASSERT_EQ(getType(left[1]), NodeType::Literal);
    ASSERT_EQ(std::get<Literal>(*left[1]).value, 'b');

    auto& right = std::get<Concat>(*alt[1]).children;
    ASSERT_EQ(right.size(), 1);
    ASSERT_EQ(getType(right[0]), NodeType::Literal);
    ASSERT_EQ(std::get<Literal>(*right[0]).value, 'c');
}


TEST(RegexTreeTest, AlternationWithRepeatInGroup) {
    regex r("((a+)|b)$");
    r.compile();
    auto root = r.tr.getRoot();
    ASSERT_EQ(getType(root), NodeType::Concat);

    // concat[0] — Alternation, concat[1] — End
    auto& top = std::get<Concat>(*root).children;
    ASSERT_EQ(getType(top[0]), NodeType::Alternation);
    ASSERT_EQ(getType(top[1]), NodeType::End);

    auto& alt = std::get<Alternation>(*top[0]).children;

    ASSERT_EQ(getType(alt[0]), NodeType::Concat);
    auto& left1 = std::get<Concat>(*alt[0]).children;
    ASSERT_EQ(left1.size(), 1);
    ASSERT_EQ(getType(left1[0]), NodeType::Concat);
    auto& left2 = std::get<Concat>(*left1[0]).children;
    ASSERT_EQ(left2.size(), 1);
    ASSERT_EQ(getType(left2[0]), NodeType::Repeat);
    auto& rep = std::get<Repeat>(*left2[0]);
    ASSERT_EQ(rep.min, 1);
    ASSERT_EQ(rep.max, INFINITY);
    ASSERT_EQ(getType(rep.child), NodeType::Literal);
    ASSERT_EQ(std::get<Literal>(*rep.child).value, 'a');

    ASSERT_EQ(getType(alt[1]), NodeType::Concat);
    auto& right = std::get<Concat>(*alt[1]).children;
    ASSERT_EQ(right.size(), 1);
    ASSERT_EQ(getType(right[0]), NodeType::Literal);
    ASSERT_EQ(std::get<Literal>(*right[0]).value, 'b');
}


TEST(RegexTreeTest, SequentialRepeats) {
    regex r("a?b+c*$");
    r.compile();
    auto root = r.tr.getRoot();
    ASSERT_EQ(getType(root), NodeType::Concat);

    auto& cc = std::get<Concat>(*root).children;
    ASSERT_EQ(cc.size(), 4);
    ASSERT_EQ(getType(cc[3]), NodeType::End);

    ASSERT_EQ(getType(cc[0]), NodeType::Repeat);
    ASSERT_EQ(getType(cc[1]), NodeType::Repeat);
    ASSERT_EQ(getType(cc[2]), NodeType::Repeat);

    auto& r1 = std::get<Repeat>(*cc[0]);
    ASSERT_EQ(r1.min, 0); ASSERT_EQ(r1.max, 1);
    ASSERT_EQ(std::get<Literal>(*r1.child).value, 'a');

    auto& r2 = std::get<Repeat>(*cc[1]);
    ASSERT_EQ(r2.min, 1); ASSERT_EQ(r2.max, INFINITY);
    ASSERT_EQ(std::get<Literal>(*r2.child).value, 'b');

    auto& r3 = std::get<Repeat>(*cc[2]);
    ASSERT_EQ(r3.min, 0); ASSERT_EQ(r3.max, INFINITY);
    ASSERT_EQ(std::get<Literal>(*r3.child).value, 'c');
}

static NodePtr unwrapConcat(const NodePtr& n) {
    NodePtr cur = n;
    while (getType(cur) == NodeType::Concat) {
        auto& kids = std::get<Concat>(*cur).children;
        if (kids.size() != 1) break;
        cur = kids[0];
    }
    return cur;
}

TEST(RegexTreeTest, AlternationWithNestedRepeatConcat) {
    regex r("(((a|b)+c)*|d)$");
    r.compile();
    auto root = r.tr.getRoot();
    ASSERT_EQ(getType(root), NodeType::Concat);

    auto& top = std::get<Concat>(*root).children;
    ASSERT_EQ(top.size(), 2);
    ASSERT_EQ(getType(top[1]), NodeType::End);

    ASSERT_EQ(getType(top[0]), NodeType::Alternation);
    auto& alt = std::get<Alternation>(*top[0]).children;
    ASSERT_EQ(alt.size(), 2);

    NodePtr leftBranch = unwrapConcat(alt[0]);
    ASSERT_EQ(getType(leftBranch), NodeType::Repeat);
    auto& outerRep = std::get<Repeat>(*leftBranch);
    ASSERT_EQ(outerRep.min, 0);
    ASSERT_EQ(outerRep.max, INFINITY);

    NodePtr repChild = unwrapConcat(outerRep.child);
    ASSERT_EQ(getType(repChild), NodeType::Concat);
    auto& repKids = std::get<Concat>(*repChild).children;
    ASSERT_EQ(repKids.size(), 2);

    NodePtr abPlus = unwrapConcat(repKids[0]);
    ASSERT_EQ(getType(abPlus), NodeType::Repeat);
    auto& innerRep = std::get<Repeat>(*abPlus);
    ASSERT_EQ(getType(innerRep.child), NodeType::Alternation);

    NodePtr cLit = unwrapConcat(repKids[1]);
    ASSERT_EQ(getType(cLit), NodeType::Literal);
    ASSERT_EQ(std::get<Literal>(*cLit).value, 'c');

    NodePtr rightBranch = unwrapConcat(alt[1]);
    ASSERT_EQ(getType(rightBranch), NodeType::Literal);
    ASSERT_EQ(std::get<Literal>(*rightBranch).value, 'd');
}

TEST(RegexTreeTest, EscapeCharacter) {
    regex r("a&+b$");
    r.compile();
    auto root = r.tr.getRoot();
    ASSERT_EQ(getType(root), NodeType::Concat);

    auto& cc = std::get<Concat>(*root).children;
    ASSERT_EQ(cc.size(), 4);
    ASSERT_EQ(getType(cc[0]), NodeType::Literal);
    ASSERT_EQ(std::get<Literal>(*cc[0]).value, 'a');

    ASSERT_EQ(getType(cc[1]), NodeType::Literal);
    ASSERT_EQ(std::get<Literal>(*cc[1]).value, '+');

    ASSERT_EQ(getType(cc[2]), NodeType::Literal);
    ASSERT_EQ(std::get<Literal>(*cc[2]).value, 'b');

    ASSERT_EQ(getType(cc[3]), NodeType::End);
}

TEST(RegexTreeTest, RepeatOpenLeftBound) {
    regex r("a{,3}$");
    r.compile();
    auto root = r.tr.getRoot();
    auto& cc = std::get<Concat>(*root).children;
    ASSERT_EQ(getType(cc[0]), NodeType::Repeat);
    auto& rep = std::get<Repeat>(*cc[0]);
    ASSERT_EQ(rep.min, 0);
    ASSERT_EQ(rep.max, 3);
    ASSERT_EQ(getType(rep.child), NodeType::Literal);
    ASSERT_EQ(std::get<Literal>(*rep.child).value, 'a');
}

TEST(RegexTreeTest, RepeatOpenRightBound) {
    regex r("a{2,}$");
    r.compile();
    auto root = r.tr.getRoot();
    auto& cc = std::get<Concat>(*root).children;
    ASSERT_EQ(getType(cc[0]), NodeType::Repeat);
    auto& rep = std::get<Repeat>(*cc[0]);
    ASSERT_EQ(rep.min, 2);
    ASSERT_EQ(rep.max, INFINITY);
    ASSERT_EQ(getType(rep.child), NodeType::Literal);
    ASSERT_EQ(std::get<Literal>(*rep.child).value, 'a');
}

TEST(RegexTreeTest, WildcardRepeat) {
    regex r(".*$");
    r.compile();
    auto root = r.tr.getRoot();
    auto& cc = std::get<Concat>(*root).children;
    ASSERT_EQ(getType(cc[0]), NodeType::Repeat);
    auto& rep = std::get<Repeat>(*cc[0]);
    ASSERT_EQ(getType(rep.child), NodeType::Wildcard);
    ASSERT_EQ(getType(cc[1]), NodeType::End);
}

TEST(RegexTreeTest, ErrorUnmatchedParen) {
    regex r("(a|b$");
    EXPECT_THROW(r.compile(), std::logic_error);
}

TEST(RegexTreeTest, ErrorEmptyGroup) {
    regex r("()$");
    EXPECT_THROW(r.compile(), std::logic_error);
}

using mgr::regex;

/* ---------- helpers ---------- */
static void expect_matches(regex& r,
                           std::initializer_list<std::string> ok,
                           std::initializer_list<std::string> bad)
{
    for (auto& s : ok)   EXPECT_TRUE (r.match(s)) << "should match: " << s;
    for (auto& s : bad)  EXPECT_FALSE(r.match(s)) << "should NOT match: " << s;
}

TEST(DKA_Match, SimpleLiteral)
{
    regex r("a$");         r.compile();
    expect_matches(r, {"a"}, {"", "aa", "b"});
}

TEST(DKA_Match, Concat)
{
    regex r("abc$");       r.compile();
    expect_matches(r, {"abc"}, {"ab", "abcd", "a"});
}

TEST(DKA_Match, ConcatBeforeAlt)
{
    regex r("(ab|cd)$");     r.compile();          // (ab)|(cd)
    expect_matches(r, {"ab", "cd"}, {"ac", "ad", "abcd"});
}

TEST(DKA_Match, ParenthesizedAlt)
{
    regex r("a(bc|d)$");   r.compile();          // "abc" или "ad"
    expect_matches(r, {"abc", "ad"}, {"abd", "ac", "a"});
}

TEST(DKA_Match, RepeatStarPlusQuestion)
{
    regex r1("a*$");       r1.compile();         // 0+ 'a'
    expect_matches(r1, {"", "a", "aaaa"}, {"b", "ab"});

    regex r2("ab+c$");     r2.compile();         // a b{1,∞} c
    expect_matches(r2, {"abc", "abbc", "abbbbbc"}, {"ac", "abb", "abcb"});

    regex r3("colou?r$");  r3.compile();         // британ/US
    expect_matches(r3, {"color", "colour"}, {"colouur", "colr"});
}

TEST(DKA_Match, BoundedRepeat)
{
    regex r("a{2,3}$");    r.compile();          // 2-3 'a'
    expect_matches(r, {"aa", "aaa"}, {"a", "aaaa", ""});
}

TEST(DKA_Match, WildcardDot)
{
    regex r("h.t$");       r.compile();          // h<any>t
    expect_matches(r, {"hat", "h t", "hot"}, {"ht", "haat"});
}

TEST(DKA_Match, NestedRepeat)
{
    regex r("(ab){2,}$");  r.compile();          // (ab)(ab)...
    expect_matches(r, {"abab", "ababab"}, {"ab", "aba", "abba"});
}

TEST(DKA_TreeStructure, EndsWithEnd)
{
    regex r("(a|b)$");     r.compile();
    auto root = r.tr.getRoot();
    ASSERT_EQ(getType(root), mgr::NodeType::Concat);
    const auto& kids = std::get<mgr::Concat>(*root).children;
    ASSERT_FALSE(kids.empty());
    EXPECT_EQ(getType(kids.back()), mgr::NodeType::End);
}

TEST(ParserErrors, MismatchedParens)
{
    regex bad("(ab$");      // нет закрытой скобки
    EXPECT_THROW(bad.compile(), std::logic_error);
}

TEST(ParserErrors, BadRepeatBounds)
{
    regex bad("a{3,2}$");   // min > max
    EXPECT_THROW(bad.compile(), std::invalid_argument);
}

static void expect_matches_regex(regex& r,
                                 std::initializer_list<std::string> ok,
                                 std::initializer_list<std::string> bad)
{
    for (auto& s : ok)   EXPECT_TRUE (r.match(s)) << "should match: " << s;
    for (auto& s : bad)  EXPECT_FALSE(r.match(s)) << "should NOT match: " << s;
}

static void expect_matches_DKA(const DKA& d,
                               std::initializer_list<std::string> ok,
                               std::initializer_list<std::string> bad)
{
    for (auto& s : ok)   EXPECT_TRUE (d.match(s)) << "should match: " << s;
    for (auto& s : bad)  EXPECT_FALSE(d.match(s)) << "should NOT match: " << s;
}

TEST(DKA_MatchExtra, ComplexOptionalGroups)
{
    regex r("(M+(e+)?p+|(h+)?i)$");   r.compile();

    expect_matches_regex(r,
        {"Mp", "MMMepp", "MMeep", "Mpppp",
         "i", "hi", "hhhhi"},
        {"M", "Mpe", "Meei", "hp", "hh", "Mi"});
}

TEST(DKA_Interface, CompleteAddsSink)
{
    regex r("ab$");  r.compile();
    size_t oldStates = r.dka.states.size();

    DKA d = r.dka;
    d.complete();
    size_t newStates = d.states.size();

    ASSERT_EQ(newStates, oldStates + 1);

    EXPECT_FALSE(d.match("azb"));
}

TEST(DKA_Interface, Complement)
{
    regex r("ab$");  r.compile();
    DKA comp = r.dka.complement();

    EXPECT_TRUE (r.match("ab"));
    EXPECT_FALSE(comp.match("ab"));

    EXPECT_FALSE(r.match("ac"));
    EXPECT_TRUE (comp.match("ac"));
}

TEST(DKA_Interface, IntersectAndDifference)
{
    regex r1("a+$");   r1.compile();
    regex r2("aa$");   r2.compile();

    DKA i   = r1.dka.intersect(r2.dka);
    DKA diff= r1.dka - r2.dka;
    expect_matches_DKA(i,
        {"aa"},
        {"a", "aaa", ""});

    expect_matches_DKA(diff,
        {"a", "aaa"},
        {"", "aa"});
}

TEST(DKA_Interface, ToRegexReturnsNonEmpty)
{
    regex r("a(b|c)$");
    r.compile();

    std::string gen = r.dka.to_regex();
    ASSERT_FALSE(gen.empty());
    ASSERT_NE(gen.find_first_of("*+?|()"), std::string::npos);
}

TEST(DKA_Interface, MinimizeReducesStates)
{
    regex r("(a|a|a)(b|b)$");
    r.compile();

    size_t after = r.dka.states.size();

    regex r_raw("(a|a|a)(b|b)$");
    r_raw.tk.Tokenize("(a|a|a)(b|b)$");
    r_raw.TokenToTree();
    r_raw.dka.TreeToDKA(r_raw.tr);
    size_t before = r_raw.dka.states.size();

    ASSERT_GT(before, after);
}