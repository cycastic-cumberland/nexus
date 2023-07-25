//
// Created by cycastic on 7/25/2023.
//

#ifndef NEXUS_PRIORITY_QUEUE_H
#define NEXUS_PRIORITY_QUEUE_H

#include "vector.h"

template <typename T>
struct HighCompare {
    static _ALWAYS_INLINE_ bool compare(const T& p_lhs, const T& p_rhs){
        return p_lhs > p_rhs;
    }
};

template <typename T>
struct LowCompare {
    static _ALWAYS_INLINE_ bool compare(const T& p_lhs, const T& p_rhs){
        return p_lhs < p_rhs;
    }
};

template <typename T, class Comparator = LowCompare<uint8_t>>
class PriorityQueue {
public:
    struct Node {
        T data;
        uint8_t priority;
        Node(const Node& p_other) : data(p_other.data), priority(p_other.priority) {}
        Node(const T& p_data, const uint8_t& p_priority) : data(p_data), priority(p_priority) {}
    };
private:
    Vector<Node> heap{};

    _FORCE_INLINE_ void copy_from(const PriorityQueue& p_other){
        heap = p_other.heap;
    }

    void heapify_up(size_t idx){
        size_t parent = (idx - 1) / 2;
        while (idx > 0 && Comparator::compare(heap[idx].priority, heap[parent].priority)){
            SWAP(heap[idx], heap[parent]);
            idx = parent;
            parent = (idx - 1) / 2;
        }
    }
    void heapify_down(size_t idx){
        while (true) {
            auto left = 2 * idx + 1;
            auto right = 2 * idx + 2;
            auto largest = idx;

            if (left < size() && Comparator::compare(heap[left].priority, heap[largest].priority))
                largest = left;
            if (right < size() && Comparator::compare(heap[right].priority, heap[largest].priority))
                largest = right;
            if (largest == idx) break;
            SWAP(heap[idx], heap[largest]);
            idx = largest;
        }
    }
public:
    _NO_DISCARD_ _ALWAYS_INLINE_ size_t size() const { return heap.size(); }
    _NO_DISCARD_ _ALWAYS_INLINE_ bool empty() const { return size() == 0; }

    _FORCE_INLINE_ void push(const T& p_value, const uint8_t& p_priority){
//        auto old_size = size();
        heap.push_back(Node(p_value, p_priority));
        heapify_up(size() - 1);
//        if (old_size != 0){
//            for (int64_t i = old_size / 2 - 1; i >= 0; i--){
//                heapify_down(i);
//            }
//        }
    }
    _FORCE_INLINE_ T pop(){
        if (empty()) throw std::out_of_range("PriorityQueue is empty");
        auto re = heap[0].data;
        SWAP(heap[0], heap[size() - 1]);
        heap.pop_back();
        heapify_down(0);
        return re;
    }

    PriorityQueue() = default;
    PriorityQueue(const PriorityQueue& p_other) : PriorityQueue() { copy_from(p_other); }
    _FORCE_INLINE_ PriorityQueue& operator=(const PriorityQueue& p_other){
        copy_from(p_other);
        return *this;
    }
};

#endif //NEXUS_PRIORITY_QUEUE_H
