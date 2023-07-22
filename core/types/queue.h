//
// Created by cycastic on 7/17/2023.
//

#ifndef NEXUS_QUEUE_H
#define NEXUS_QUEUE_H

#include <stdexcept>
#include "../typedefs.h"

template <typename T>
class Queue {
public:
    struct QueueNode {
    private:
        T value;
        QueueNode* next{};
        friend class Queue<T>;
    public:
        const QueueNode* forward() const { return next; }
        const T& get_value() const { return value; }
    };
private:
    QueueNode* genesis{};
    QueueNode* last{};
    size_t size_cache{};
public:
    _FORCE_INLINE_ void enqueue(const T& p_value){
        auto new_node = new QueueNode();
        new_node->value = p_value;
        if (!last){
            genesis = new_node;
            last = new_node;
        } else {
            last->next = new_node;
            last = new_node;
        }
        size_cache++;
    }
    _FORCE_INLINE_ T dequeue(){
        if (!genesis) throw std::out_of_range("Queue is empty");
        auto re = genesis->value;
        auto removal_target = genesis;
        genesis = genesis->next;
        delete removal_target;
        if (genesis == nullptr) last = nullptr;
        size_cache--;
        return re;
    }
    _FORCE_INLINE_ void clear(){
        while (!empty()){
            dequeue();
        }
    }
    _NO_DISCARD_ _FORCE_INLINE_ size_t size() const { return size_cache; }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return size() == 0; }
    _NO_DISCARD_ _FORCE_INLINE_ const QueueNode* first() const { return genesis; }
    Queue() = default;
    Queue(const Queue& p_other){
        for (auto it = p_other.first(); it != nullptr; it = it->forward()){
            enqueue(it->get_value());
        }
    }
    ~Queue(){
        clear();
    }
};

#endif //NEXUS_QUEUE_H
