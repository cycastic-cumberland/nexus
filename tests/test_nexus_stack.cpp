//
// Created by cycastic on 8/4/2023.
//

#include <gtest/gtest.h>
#include "../runtime/nexus_stack.h"
#include "../core/types/interned_string.h"

struct CustomStackStruct {
    uint64_t i64;
    double f64;
    VString string;
};

class NexusStackTestFixture: public ::testing::Test{
    NexusTypeInfoServer* type_info_server;
    NexusStack* stack;
    StackItemMetadata::ID custom_stack_struct_id{};
    const StackItemMetadata* struct_metadata;
public:
    void SetUp() override {
        // This would not work if there's any padding
        static_assert(sizeof(CustomStackStruct) == sizeof(CustomStackStruct::i64) + sizeof(CustomStackStruct::f64) + sizeof(CustomStackStruct::string));
        InternedString::configure();
        type_info_server = new NexusTypeInfoServer(false);
        auto struct_vtable = type_info_server->get_primitive_vtable(NexusSerializedBytecode::STACK_STRUCT);
        // 1 MiB
        stack = new NexusStack(type_info_server, 1024 * 1024, 16);
        LinkedList<const StackItemMetadata*> custom_stack_struct_metadata = {
            type_info_server->get_primitive_metadata(NexusBytecodeMetadata::UNSIGNED_64_BIT_INTEGER),
            type_info_server->get_primitive_metadata(NexusBytecodeMetadata::DOUBLE_PRECISION_FLOATING_POINT),
            type_info_server->get_primitive_metadata(NexusBytecodeMetadata::STRING)
        };
        custom_stack_struct_id = type_info_server->add_struct_type(struct_vtable, std::move(custom_stack_struct_metadata));
        struct_metadata = type_info_server->get_metadata_by_id(custom_stack_struct_id);
    }
    void TearDown() override {
        delete stack;
        delete type_info_server;
        InternedString::cleanup();
    }
    void add_frame(){
        stack->push_stack_frame();
    }
    void pop_frame(){
        stack->pop_stack_frame();
    }
    void add_objects_1(){
        auto& frame = stack->get_last_frame();
        frame->push(12);
        frame->push(32.0f);
        frame->push(L"Hello World!");
        auto struct_type = CustomStackStruct{
            33, 4.5, "This is a string"
        };
        frame->push(struct_metadata, &struct_type);
    }
    void add_objects_2(){
        auto& frame = stack->get_last_frame();
        frame->push(22);
        frame->push(42.0f);
        frame->push(L"Goodbye world!");
        auto struct_type = CustomStackStruct{
                23, 3.5, "This is a vector string"
        };
        frame->push(struct_metadata, &struct_type);
    }
    bool check_objects_1(int64_t p_idx){
#define CAST(value, type) *(type*)(value.data)
        const auto& frame = stack->get_frame_at(p_idx);
        if (frame->object_count() != 4) return false;

        auto i32 = frame->get_at(-4);
        auto f32 = frame->get_at(-3);
        auto str = frame->get_at(-2);
        auto obj = frame->get_at(-1);

        if (i32.type->type != NexusSerializedBytecode::SIGNED_32_BIT_INTEGER) return false;
        if (f32.type->type != NexusSerializedBytecode::SINGLE_PRECISION_FLOATING_POINT) return false;
        if (str.type->type != NexusSerializedBytecode::STRING_LITERAL) return false;

        if (CAST(i32, int32_t) != 12) return false;
        if (CAST(f32, float) != 32.0f) return false;
        if (CAST(str, InternedString) != L"Hello World!") return false;

        {
            auto &as_custom_stack_struct = *(CustomStackStruct *) obj.data;

            if (as_custom_stack_struct.i64 != 33) return false;
            if (as_custom_stack_struct.f64 != 4.5) return false;
            if (as_custom_stack_struct.string != "This is a string") return false;
        }
        auto struct_type = CustomStackStruct{
                23, 3.5, "This is a vector string"
        };
        frame->set(-1, struct_metadata, &struct_type);
        {
            auto &as_custom_stack_struct = *(CustomStackStruct *) obj.data;

            if (as_custom_stack_struct.i64 != 23) return false;
            if (as_custom_stack_struct.f64 != 3.5) return false;
            if (as_custom_stack_struct.string != "This is a vector string") return false;
        }

        frame->set(-4, 9);
        if (CAST(i32, int32_t) != 9) return false;

        return true;
#undef CAST
    }
    bool check_objects_2(int64_t p_idx){
#define CAST(value, type) *(type*)(value.data)
        const auto& frame = stack->get_frame_at(p_idx);
        if (frame->object_count() != 4) return false;

        auto i32 = frame->get_at(-4);
        auto f32 = frame->get_at(-3);
        auto str = frame->get_at(-2);
        auto obj = frame->get_at(-1);

        if (i32.type->type != NexusSerializedBytecode::SIGNED_32_BIT_INTEGER) return false;
        if (f32.type->type != NexusSerializedBytecode::SINGLE_PRECISION_FLOATING_POINT) return false;
        if (str.type->type != NexusSerializedBytecode::STRING_LITERAL) return false;

        if (CAST(i32, int32_t) != 22) return false;
        if (CAST(f32, float) != 42.0f) return false;
        if (CAST(str, InternedString) != L"Goodbye world!") return false;

        {
            auto &as_custom_stack_struct = *(CustomStackStruct *) obj.data;

            if (as_custom_stack_struct.i64 != 23) return false;
            if (as_custom_stack_struct.f64 != 3.5) return false;
            if (as_custom_stack_struct.string != "This is a vector string") return false;
        }

        auto struct_type = CustomStackStruct{
                33, 4.5, "This is a string"
        };
        frame->set(-1, struct_metadata, &struct_type);

        {
            auto &as_custom_stack_struct = *(CustomStackStruct *) obj.data;

            if (as_custom_stack_struct.i64 != 33) return false;
            if (as_custom_stack_struct.f64 != 4.5) return false;
            if (as_custom_stack_struct.string != "This is a string") return false;
        }

        frame->set(-4, 9);
        if (CAST(i32, int32_t) != 9) return false;

        return true;
#undef CAST
    }
};

TEST_F(NexusStackTestFixture, TestNexusStack){
    add_frame();
    add_objects_1();
    add_frame();
    add_objects_2();
    EXPECT_TRUE(check_objects_2(-1));
    EXPECT_TRUE(check_objects_1(-2));
    pop_frame();
    pop_frame();

    // Edge case
    add_frame();
    add_frame();
    add_objects_1();
    EXPECT_TRUE(check_objects_1(-1));
    pop_frame();
    pop_frame();

//    add_frame();
//    add_objects_1();
}