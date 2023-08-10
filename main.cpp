//
// Created by cycastic on 7/22/2023.
//
#ifdef BENCHMARKS_ENABLED
#include <benchmark/benchmark.h>
#endif
#ifdef TESTS_ENABLED
#include <gtest/gtest.h>
#endif

#include "core/cmd_handler.h"

int main(int argc, char** argv) {
    CmdHandler::load_cmd_arguments(argc, argv);
    int result{};
#ifdef BENCHMARKS_ENABLED
    std::cout << "-------------------------------- BENCHMARK --------------------------------\n";
    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
#endif
#ifdef TESTS_ENABLED
    std::cout << "-------------------------------- UNIT TEST --------------------------------\n";
    ::testing::InitGoogleTest(&argc, argv);
    result = RUN_ALL_TESTS();
#endif
    CmdHandler::cleanup();
    return result;
}