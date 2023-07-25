//
// Created by cycastic on 7/22/2023.
//

#include <gtest/gtest.h>
#include "runtime/config.h"

int main(int argc, char** argv) {
    initialize_nexus_runtime();
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    destroy_nexus_runtime();
    return result;
}