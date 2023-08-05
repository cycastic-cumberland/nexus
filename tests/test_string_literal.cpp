//
// Created by cycastic on 7/31/2023.
//

#include <gtest/gtest.h>
#include "../core/types/interned_string.h"

class StringLiteralTestFixture : public ::testing::Test {
public:
    void SetUp() override {
        InternedString::configure();
    }

    void TearDown() override {
        InternedString::cleanup();
    }

    static bool general_test(){
        VString vector_string_from_static_c_string = "Hello World";
        InternedString from_static_c_string = "Hello World";
        InternedString from_vector_string = vector_string_from_static_c_string;
        InternedString from_wide_char_string_literal = L"Hello World";
        InternedString from_other_string_literal = from_static_c_string;
        if (from_static_c_string != from_vector_string) return false;
        if (from_static_c_string != from_wide_char_string_literal) return false;
        if (from_static_c_string != from_other_string_literal) return false;
        return true;
    }
};

TEST_F(StringLiteralTestFixture, TestStringLiteral){
    EXPECT_TRUE(general_test());
}