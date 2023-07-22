//
// Created by cycastic on 7/17/2023.
//

#ifndef NEXUS_STACK_H
#define NEXUS_STACK_H

#include <stdexcept>
#include "../typedefs.h"

template <typename T>
class Stack {
public:
    struct StackNode {
    private:
        T value;
        StackNode* next{};
        friend class Stack<T>;
    public:
        const StackNode* unwind() const { return next; }
        const T& get_value() const { return value; }
    };
private:
    StackNode* latest{};
    StackNode* genesis{};
    size_t size_cache{};
public:
    _FORCE_INLINE_ void push(const T& p_value){
        auto new_node = new StackNode();
        new_node->value = p_value;
        if (!latest) {
            latest = new_node;
            genesis = new_node;
        } else {
            new_node->next = latest;
            latest = new_node;
        }
        size_cache++;
    }
    _FORCE_INLINE_ T pop(){
        if (!latest) throw std::out_of_range("Stack is empty");
        auto re = latest->value;
        auto removal_target = latest;
        latest = latest->next;
        delete removal_target;
        size_cache--;
        if (latest == nullptr) genesis = nullptr;
        return re;
    }
    _FORCE_INLINE_ const T& peek_last() const {
        if (!latest) throw std::out_of_range("Stack is empty");
        return latest->value;
    }
    _FORCE_INLINE_ void clear(){
        while (!empty()){
            pop();
        }
    }
    _NO_DISCARD_ _FORCE_INLINE_ size_t size() const { return size_cache; }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return size() == 0; }
    _NO_DISCARD_ _FORCE_INLINE_ const StackNode* last() const { return latest; }
    Stack() = default;
    Stack(const Stack& p_other){
        auto buffer = new Stack();
        for (auto it = p_other.last(); it != nullptr; it = it->unwind()){
            buffer->push(it->get_value());
        }
        while (!buffer->empty()){
            push(buffer->pop());
        }
    }
    ~Stack(){
        clear();
    }
};

#endif //NEXUS_STACK_H
