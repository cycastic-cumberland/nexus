//
// Created by cycastic on 8/1/2023.
//

#ifndef NEXUS_SAFE_PRIORITY_QUEUE_H
#define NEXUS_SAFE_PRIORITY_QUEUE_H

#include "priority_queue.h"
#include "../lock.h"

template<typename T, class Comparator = LowCompare<uint8_t>>
class SafePriorityQueue : public PriorityQueue<T, Comparator> {
    mutable RWLock lock{};
protected:
    void copy_from(const PriorityQueue<T, Comparator>& p_other) override {
        W_GUARD(lock);
        PriorityQueue<T, Comparator>::copy_from(p_other);
    }
public:
    void push(const T& p_value, const uint8_t& p_priority) override {
        W_GUARD(lock);
        PriorityQueue<T, Comparator>::push(p_value, p_priority);
    }
    T pop() override {
        W_GUARD(lock);
        return PriorityQueue<T, Comparator>::pop();
    }
    bool try_pop(T& p_result) override {
        W_GUARD(lock);
        return PriorityQueue<T, Comparator>::try_pop(p_result);
    }
    size_t size() const override {
        R_GUARD(lock);
        return PriorityQueue<T, Comparator>::size();
    }
};

#endif //NEXUS_SAFE_PRIORITY_QUEUE_H
