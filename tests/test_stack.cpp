//
// Created by cycastic on 7/22/2023.
//

#include <gtest/gtest.h>
#include "../core/types/stack.h"

class StackTestFixture: public ::testing::Test {
private:
    Stack<int> stack1{};
    Stack<int> stack2{};
public:
    StackTestFixture() = default;

    void SetUp() override {

    }

    void TearDown() override {

    }

    ~StackTestFixture() override = default;

    bool empty_pop(){
        try {
            if (!stack1.empty()) return false;
            stack1.pop();
            return false;
        } catch (const std::exception&){
            return true;
        }
    }
    bool general_functionalities_test(){
        try {
            if (!stack1.empty()) return false;
            stack1.push(1);
            stack1.push(2);
            stack1.push(3);
            if (stack1.empty()) return false;
            if (stack1.size() != 3) return false;
            if (stack1.peek_last() != 3) return false;
            if (stack1.size() != 3) return false;
            if (stack1.pop() != 3) return false;
            if (stack1.pop() != 2) return false;
            if (stack1.pop() != 1) return false;
            if (!stack1.empty()) return false;
            return true;
        } catch (const std::exception&){
            return false;
        }
    }
    bool replication_test(){
        try {
            if (!stack1.empty()) return false;
            stack1.push(1);
            stack1.push(2);
            stack1.push(3);
            stack2 = stack1;

            if (stack1.peek_last() != 3) return false;
            if (stack1.size() != 3) return false;
            if (stack1.pop() != 3) return false;
            if (stack1.pop() != 2) return false;
            if (stack1.pop() != 1) return false;
            if (!stack1.empty()) return false;

            if (stack2.peek_last() != 3) return false;
            if (stack2.size() != 3) return false;
            if (stack2.pop() != 3) return false;
            if (stack2.pop() != 2) return false;
            if (stack2.pop() != 1) return false;
            if (!stack2.empty()) return false;
            return true;
        } catch (const std::exception&){
            return false;
        }
    }
    void clear_all_stacks(){
        stack1.clear();
    }
};

TEST_F(StackTestFixture, TestStack){
    EXPECT_TRUE(empty_pop());
    EXPECT_TRUE(general_functionalities_test());
    EXPECT_TRUE(replication_test());
}
