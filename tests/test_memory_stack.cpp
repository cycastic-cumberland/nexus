//
// Created by cycastic on 7/22/2023.
//

#include <gtest/gtest.h>
#include "../runtime/memory_stack.h"


class MemoryStackTestFixture: public ::testing::Test{
    MemoryStack* mem;
public:
    void SetUp() override {
        mem = new MemoryStack();
    }
    void TearDown() override {
        delete mem;
    }
    void new_stack_frame(){
        mem->allocate_stack_frame();
    }
    bool value_push_test(){
        if (!mem->empty_stack()) return false;
        try {
            mem->push((uint32_t)15);
            mem->push((float)32.0f);
            mem->push(VString("Hello World!"));
        } catch (const MemoryStackException&){
            return false;
        }
        auto re = (mem->allocated_objects_in_this_frame() == 3);
        return re;
    }
    bool value_get_test(){
        if (mem->empty_stack()) return false;
        try {
            {
                mem->copy_to_top(-1);
                auto item = mem->get_top().to_ambiguous();
                if (item.get_string() == nullptr) return false;
                if (*item.get_string() != "Hello World!") return false;
                mem->pop();
            }
            {
                mem->copy_to_top(-2);
                auto item = mem->get_top().to_ambiguous();
                if (item.get_fp32() == nullptr) return false;
                if (*item.get_fp32() != 32.0f) return false;
                mem->pop();
            }
            {
                mem->copy_to_top(-3);
                auto item = mem->get_top().to_ambiguous();
                if (item.get_u32() == nullptr) return false;
                if (*item.get_u32() != 15) return false;
                mem->pop();
            }
        } catch (const MemoryStackException&){
            return false;
        }
        auto re = (mem->allocated_objects_in_this_frame() == 3);
        return re;
    }
    bool pop_test(){
        try {
            while (!mem->empty_stack())
                mem->pop();
        } catch (const MemoryStackException&) {
            return false;
        }
        auto re = (mem->empty_stack());
        return re;
    }
    bool frame_deallocate_test(){
        try {
            auto frame_objects = mem->allocated_objects_in_this_frame();
            auto total_objects = mem->objects_count();
            mem->deallocate_stack_frame();
            if (total_objects - frame_objects != 3) return false;
        } catch (const MemoryStackException&) {
            return false;
        }
        mem->allocate_stack_frame();
        return true;
    }
};

TEST_F(MemoryStackTestFixture, TestMemoryStack){
    EXPECT_TRUE(value_push_test());
    EXPECT_TRUE(value_get_test());
    EXPECT_TRUE(pop_test());
    EXPECT_TRUE(value_push_test());
    new_stack_frame();
    EXPECT_TRUE(value_push_test());
    EXPECT_TRUE(frame_deallocate_test());
    EXPECT_TRUE(frame_deallocate_test());
}