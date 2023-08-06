//
// Created by cycastic on 8/5/2023.
//

#ifndef NEXUS_UTILS_H
#define NEXUS_UTILS_H

#include "../core/typedefs.h"
#include "thread_pool.h"
#include <chrono>
#include <thread>

namespace NexusUtils {
    static void batch_copy(uint8_t p_thread_no, uint8_t p_thread_count, void* p_dst, const void* p_src, size_t p_size){
        size_t partition_size = p_size / p_thread_count;
        size_t offset = partition_size * p_thread_no;
        if (p_thread_no == p_thread_count - 1)
            memcpy((void*)((size_t)p_dst + offset), (const void*)((size_t)p_src + offset), p_size - offset);
        else
            memcpy((void*)((size_t)p_dst + offset), (const void*)((size_t)p_src + offset), partition_size);
    }
    static void batch_copy(ThreadPool *p_pool, ThreadPool::Priority p_priority, uint8_t p_thread_count, void* p_dst, const void* p_src, size_t p_size){
        static constexpr auto wait_time = 10;
        auto flags = new bool[p_thread_count];
        for (uint8_t thread_no = 0; thread_no < p_thread_count; thread_no++){
            auto flag = &flags[thread_no];
            *flag = false;
            (p_pool->queue_task(p_priority, [flag](uint8_t p_thread_no, uint8_t p_thread_count, void* p_dst, const void* p_src, size_t p_size) -> void {
                batch_copy(p_thread_no, p_thread_count, p_dst, p_src, p_size);
                *flag = true;
            }, thread_no, p_thread_count, p_dst, p_src, p_size));
        }
        for (uint8_t thread_no = 0; thread_no < p_thread_count; thread_no++){
            while (!flags[thread_no]) {}
        }
        delete[] flags;
    }
}


#endif //NEXUS_UTILS_H
