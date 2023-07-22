//
// Created by cycastic on 7/21/2023.
//

#ifndef NEXUS_LINKED_LIST_H
#define NEXUS_LINKED_LIST_H

#include "../typedefs.h"
#include "../exception.h"

class LinkedListException : public Exception {
public:
    explicit LinkedListException(const char* p_msg) : Exception(p_msg) {}
};

template <typename T>
class LinkedList {
public:
    struct LinkedListNode {
    public:
        T data;
    private:
        LinkedListNode *next_ptr;
        LinkedListNode *prev_ptr;
        friend class LinkedList<T>;

        const LinkedList* master;

        explicit LinkedListNode(const T& p_data, const LinkedList* p_master) {
            data = p_data;
            master = p_master;
            next_ptr = nullptr;
            prev_ptr = nullptr;
        }
    public:
        _FORCE_INLINE_ const LinkedListNode* next() const { return next_ptr; }
        _FORCE_INLINE_ const LinkedListNode* prev() const { return prev_ptr; }
        _FORCE_INLINE_ LinkedListNode* next() { return next_ptr; }
        _FORCE_INLINE_ LinkedListNode* prev() { return prev_ptr; }
    };
private:
    LinkedListNode* first_ptr{};
    LinkedListNode* last_ptr{};
    size_t size_cache{};
public:
    _FORCE_INLINE_ const LinkedListNode* first() const { return first_ptr; }
    _FORCE_INLINE_ const LinkedListNode* last() const { return last_ptr; }
    _FORCE_INLINE_ LinkedListNode* first() { return first_ptr; }
    _FORCE_INLINE_ LinkedListNode* last() { return last_ptr; }
    _NO_DISCARD_ _FORCE_INLINE_ size_t size() const { return size_cache; }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return size_cache == 0; }
    _FORCE_INLINE_ void clear(){
        auto it = first_ptr;
        while (it){
            auto target = it;
            it = it->next_ptr;
            delete target;
        }
        size_cache = 0;
    }
    _FORCE_INLINE_ void add_last(const T& p_data){
        auto new_node = new LinkedListNode(p_data, this);
        if (last_ptr == nullptr){
            last_ptr = new_node;
            first_ptr = new_node;
        } else {
            last_ptr->next_ptr = new_node;
            new_node->prev_ptr = last_ptr;
            last_ptr = new_node;
        }
        size_cache++;
    }
    _FORCE_INLINE_ void add_front(const T& p_data){
        auto new_node = new LinkedListNode(p_data, this);
        if (first_ptr == nullptr){
            first_ptr = new_node;
            last_ptr = new_node;
        } else {
            first_ptr->prev_ptr = new_node;
            new_node->next_ptr = first_ptr;
            first_ptr = new_node;
        }
        size_cache++;
    }
    _FORCE_INLINE_ void insert_after(const T& p_data, const LinkedListNode* p_node){
        if (!p_node) throw LinkedListException("Linked list node is null");
        if (p_node->master != this) throw LinkedListException("Linked list does not own this node");
        auto new_node = new LinkedListNode(p_data, this);
        if (p_node->next_ptr){
            new_node->next_ptr = p_node->next_ptr;
            p_node->next_ptr->prev_ptr = new_node;
            p_node->next_ptr = new_node;
        } else {
            p_node->next_ptr = new_node;
            new_node->prev_ptr = p_node;
        }
        size_cache++;
    }
    _FORCE_INLINE_ bool erase(const LinkedListNode* p_node){
        if (!p_node) return false;
        if (p_node->master != this) return false;
        if (first_ptr == p_node){
            first_ptr = p_node->next_ptr;
        }
        if (last_ptr == p_node){
            last_ptr = p_node->prev_ptr;
        }
        if (p_node->prev_ptr){
            p_node->prev_ptr->next_ptr = p_node->next_ptr;
        }
        if (p_node->next_ptr){
            p_node->next_ptr->prev_ptr = p_node->prev_ptr;
        }
        delete const_cast<LinkedListNode*>(p_node);
        size_cache--;
        return true;
    }
    _FORCE_INLINE_ void copy(const LinkedList& p_other){
        clear();
        for (const auto* it = p_other.first(); it; it = it->next()){
            add_last(it->data);
        }
    }
    LinkedList() = default;
    LinkedList(const LinkedList& p_other) : LinkedList() { copy(p_other); }
    ~LinkedList() { clear(); }
};

#endif //NEXUS_LINKED_LIST_H
