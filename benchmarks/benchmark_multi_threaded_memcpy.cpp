//
// Created by cycastic on 8/5/2023.
//

#include <benchmark/benchmark.h>
#include <cstring>
#include "../core/io/file_access_server.h"
#include "../runtime/utils.h"
#include "../core/types/box.h"

static void BM_BatchCopy(benchmark::State& state) {
    const auto allocation_size = state.range(0);
    const auto thread_count = state.range(1);
    Box<ThreadPool, ThreadUnsafeObject> thread_pool= Box<ThreadPool, ThreadUnsafeObject>::make_box(thread_count);
    char* from = new char[state.range(0)];
    char* to = new char[state.range(0)];
    for (size_t i = 0; i < allocation_size; i++){
        from[i] = 42;
    }
    for (auto _ : state) {
        NexusUtils::batch_copy(thread_pool.ptr(), ThreadPool::MEDIUM, thread_count, to, from, allocation_size);
    }
    state.SetBytesProcessed(int64_t(state.iterations()) *
                            int64_t(allocation_size));
    delete[] from;
    delete[] to;
}


static void BM_memcpy(benchmark::State& state) {
    char* src = new char[state.range(0)];
    char* dst = new char[state.range(0)];
    memset(src, 'x', state.range(0));
    for (auto _ : state)
        memcpy(dst, src, state.range(0));
    state.SetBytesProcessed(int64_t(state.iterations()) *
                            int64_t(state.range(0)));
    delete[] src;
    delete[] dst;
}

// 1MiB on 4 threads reaches 2 TiB/s, WHAT THE HELL???
// ... it should have been around 3 GiB/s
BENCHMARK(BM_BatchCopy)->Args({1024, 4})->Args({1'048'576, 4})->Args({536'870'912, 4})
                       ->Args({1024, 8})->Args({1'048'576, 8})->Args({536'870'912, 8});
BENCHMARK(BM_memcpy)->Arg(1024)->Arg(1'048'576)->Arg(536'870'912);

//BENCHMARK_F(MultiThreadedCopyBenchmarkFixture, BenchmarkMultiThreadedCopy)(benchmark::State& state){
//    make_copy();
//
//}
