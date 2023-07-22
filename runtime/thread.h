//
// Created by cycastic on 7/22/2023.
//

#ifndef NEXUS_THREAD_H
#define NEXUS_THREAD_H

#include <thread>
#include "memory_stack.h"

// Ideal operation
// Start: create thread and stack, not busy
// Insert process: mark busy, start executing
// Process leave: return stack data and exit info (thread left safely/exception thrown/await called)
// If shared: do not object_destroy thread on leave
class PrimitiveThread {
public:
    enum ThreadAttribute : unsigned int {
        ENCLOSED = 0,       // Standard thread
        SHARED = 1,         // Async thread,
        IS_BUSY = 2         // Currently executing
    };
private:
    MemoryStackException* handling_stack;
};

#endif //NEXUS_THREAD_H
