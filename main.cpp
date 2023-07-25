//
// Created by cycastic on 7/22/2023.
//

#include <gtest/gtest.h>
#include "runtime/config.h"
#include "core/types/priority_queue.h"

int main(int argc, char** argv) {
    initialize_nexus_runtime(false);
//    ::testing::InitGoogleTest(&argc, argv);
//    int result = RUN_ALL_TESTS();
    destroy_nexus_runtime();
//    return result;
    return 0;
}