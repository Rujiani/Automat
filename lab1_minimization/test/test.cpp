#include <gtest/gtest.h>
#include "../Machines/A_Mealy.hpp"
#include "../Machines/A_Moore.hpp"

#define w1 0
#define w2 1
#define z1 0
#define z2 1

vector<size_t> test_input = {z1, z2, z1, z2, z2, z1, z2, z1},
    test_output = {w1, w2, w1, w2, w2, w1, w2, w1},
    test_input2 = {z2, z2, z1, z2, z1, z1, z1, z2},
    test_output2 = {w2, w2, w1, w2, w1, w1, w1, w2},
    test_input3 = {z2, z1, z1, z1, z2, z2, z2, z1, z1, z2},
    test_output3 = {w2, w1, w1, w1, w2, w2, w2, w1, w1, w2},
    test_input4 = {z1, z1, z2, z1, z2, z2},
    test_output4 = {w1, w1, w2, w1, w2, w2},
    test_input5 = {z1, z2, z2, z2, z1, z1, z2, z1, z2, z1, z2, z2, z1},
    test_output5 = {w1, w2, w2, w2, w1, w1, w2, w1, w2, w1, w2, w2, w1};

vector<std::pair<size_t, size_t>> Mealy_Machine1 = 
    {{5, w1}, {8, w2}, {6, w2}, {4, w1}, {3, w1}, {6, w2}, {0, w2}, {2, w1}, {2, w1}, {3, w2}, {5, w1}, {7, w2}, {6, w2}, {1, w1}, {8, w1}, {5, w2}, {7, w1}, {0, w2}},
    Mealy_Machine2 =
    {{5, w1}, {5, w2}, {6, w2}, {4, w1}, {3, w1}, {6, w2}, {0, w2}, {2, w1}, {2, w1}, {3, w2}, {5, w1}, {0, w2}, {6, w2}, {1, w1}};

vector<size_t> Moore_Machine =
    {1, 2, w1, 1, 6, w1, 1, 6, w2, 3, 9, w2, 7, 8, w1, 6, 7, w1, 1, 2, w2, 5, 3, w1, 6, 7, w2, 3, 4, w1};

ATM::Mealy_Machine M1(9, 2, Mealy_Machine1);
ATM::Mealy_Machine M2(7, 2, Mealy_Machine2);

ATM::Moore_Machine M3(10, 2, Moore_Machine);

TEST(Test1_Mealy, one){
    EXPECT_EQ(M1.Process_sequence(test_input), test_output);
    EXPECT_EQ(M1.Process_sequence(test_input2), test_output2);
    EXPECT_EQ(M1.Process_sequence(test_input3), test_output3);
    EXPECT_EQ(M1.Process_sequence(test_input4), test_output4);
    EXPECT_EQ(M1.Process_sequence(test_input5), test_output5);
}

TEST(Test2_Mealy, two){
    EXPECT_EQ(M2.Process_sequence(test_input), test_output);
    EXPECT_EQ(M2.Process_sequence(test_input2), test_output2);
    EXPECT_EQ(M2.Process_sequence(test_input3), test_output3);
    EXPECT_EQ(M2.Process_sequence(test_input4), test_output4);
    EXPECT_EQ(M2.Process_sequence(test_input5), test_output5);
}

TEST(Test3_Moore, three){
    EXPECT_EQ(M3.Process_sequence(test_input), test_output);
    EXPECT_EQ(M3.Process_sequence(test_input2), test_output2);
    EXPECT_EQ(M3.Process_sequence(test_input3), test_output3);
    EXPECT_EQ(M3.Process_sequence(test_input4), test_output4);
    EXPECT_EQ(M3.Process_sequence(test_input5), test_output5);
}