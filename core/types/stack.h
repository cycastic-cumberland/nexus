//
// Created by cycastic on 7/17/2023.
//

#ifndef NEXUS_STACK_H
#define NEXUS_STACK_H

#include <stdexcept>
#include "vector.h"
#include "../typedefs.h"

template<class T>
class AbstractStack {
public:
    virtual void push(const T& p_data) = 0;
    virtual T pop() = 0;
    virtual const T& peek_last() const = 0;
    _NO_DISCARD_ virtual size_t size() const = 0;
    _NO_DISCARD_ virtual bool empty() const = 0;
    virtual void clear() = 0;
};

// Virtual objects friendly
template <typename T>
class Stack : public AbstractStack<T> {
public:
    struct StackNode {
    private:
        T value;
        StackNode* next{};
        friend class Stack<T>;
    public:
        explicit StackNode(const T& p_data) : value(p_data) {}
        explicit StackNode(T&& p_data) : value(p_data) {}
        const StackNode* unwind() const { return next; }
        const T& get_value() const { return value; }
    };
private:
    StackNode* latest{};
    StackNode* genesis{};
    size_t size_cache{};
    Vector<T> storage;
    _FORCE_INLINE_ void copy(const Stack& p_other){
        auto buffer = new Stack();
        for (auto it = p_other.last(); it != nullptr; it = it->unwind()){
            buffer->push(it->get_value());
        }
        while (!buffer->empty()){
            push(buffer->pop());
        }
        delete buffer;
    }
public:
    void push(const T& p_value) override {
        auto new_node = new StackNode(p_value);
        if (!latest) {
            latest = new_node;
            genesis = new_node;
        } else {
            new_node->next = latest;
            latest = new_node;
        }
        size_cache++;
    }

    template<class ...Args>
    _FORCE_INLINE_ void emplace(Args&& ...args){
        auto new_node = new StackNode(T(args...));
        if (!latest) {
            latest = new_node;
            genesis = new_node;
        } else {
            new_node->next = latest;
            latest = new_node;
        }
        size_cache++;
    }
    T pop() override {
        if (!latest) throw std::out_of_range("Stack is empty");
        auto re = latest->value;
        auto removal_target = latest;
        latest = latest->next;
        delete removal_target;
        size_cache--;
        if (latest == nullptr) genesis = nullptr;
        return re;
    }
    const T& peek_last() const override {
        if (!latest) throw std::out_of_range("Stack is empty");
        return latest->value;
    }
    void clear() override {
        while (!empty()){
            pop();
        }
    }
    _NO_DISCARD_ size_t size() const override { return size_cache; }
    _NO_DISCARD_ bool empty() const override { return size() == 0; }
    _NO_DISCARD_ _FORCE_INLINE_ const StackNode* last() const { return latest; }
    _FORCE_INLINE_ Stack& operator=(const Stack& p_other){
        clear();
        copy(p_other);
        return *this;
    }
    Stack() = default;
    Stack(const Stack& p_other) : Stack() {
        copy(p_other);
    }
    ~Stack(){
        clear();
    }
};

// Unsafe for virtual objects
template <typename T>
class VectorStack : public AbstractStack<T> {
private:
    Vector<T> storage;
public:
    void push(const T& p_value) override {

    }

    template<class ...Args>
    _FORCE_INLINE_ void emplace(Args&& ...args){
        storage.emplace(args...);
    }
    T pop() override {
        T re(std::move(storage.last()));
        storage.pop_back();
        return re;
    }
    void safe_pop() {
        storage.pop_back();
    }
    const T& peek_last() const override {
        return storage.last();
    }
    T& modify_last() {
        return storage.last();
    }
    const T& peek_at(const int64_t& p_idx) const {
        return p_idx >= int64_t(0) ? storage[p_idx] : storage[storage.size() + p_idx];
    }
    _NO_DISCARD_ size_t size() const override { return storage.size(); }
    _NO_DISCARD_ bool empty() const override { return size() == 0; }
    _FORCE_INLINE_ VectorStack& operator=(const VectorStack& p_other){
        storage = p_other.storage;
        return *this;
    }
    void clear() override {
        while (!empty()) safe_pop();
    }
    explicit VectorStack(const size_t& p_starting_capacity = 4) : storage(4) {};
    VectorStack(const VectorStack& p_other) : storage(p_other.storage) {}
    ~VectorStack(){
        clear();
    }
};

#endif //NEXUS_STACK_H
