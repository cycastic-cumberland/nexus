//
// Created by cycastic on 6/17/2023.
//

#ifndef NEXUS_COW_H
#define NEXUS_COW_H

#include "safe_refcount.h"
#include "../lock.h"

template <typename T>
class CowArray {
private:
    struct CowData {
        T* data{};
        size_t capacity{};
        SafeRefCount refcount{};
        CowData(){
            refcount.init();
        }
    };
    CowData *cow_data{};
    void unref(){
        if (cow_data && cow_data->refcount.unref()) {
            free(cow_data->data);
            memdelete(cow_data);
        }
    }
    void ref(CowData* p_data){
        // Does not lock
        if (p_data == cow_data) return;
        unref();
        cow_data = p_data;
        if (cow_data){
            cow_data->refcount.ref();
        }
    }
    void move() {
        if (cow_data->refcount.get() == 1) return;
        auto new_data = new CowData();
        new_data->data = (T*) malloc(sizeof(T) * cow_data->capacity);
        memcpy(new_data->data, cow_data->data, cow_data->capacity * sizeof(T));
        new_data->capacity = cow_data->capacity;
        unref();
        cow_data = new_data;
    }
public:
    _FORCE_INLINE_ CowArray& operator=(const CowArray& p_other) {
        ref(const_cast<CowData*>(p_other.cow_data));
        return *this;
    }
    _FORCE_INLINE_ CowArray(){ cow_data = new CowData(); }
    _FORCE_INLINE_ CowArray(const CowArray& p_other) {
        *this = p_other;
    }
    _FORCE_INLINE_ ~CowArray(){
        unref();
    }
    _FORCE_INLINE_ void clear(){
        move();
        free(cow_data->data);
        cow_data->data = nullptr;
        cow_data->capacity = 0;
    }
    _FORCE_INLINE_ void resize(const size_t& new_capacity){
        if (new_capacity == cow_data->capacity) return;
        if (new_capacity == 0){
            clear(); return;
        }
        move();
        auto new_data = (T*)malloc(sizeof(T) * new_capacity);
        memcpy(new_data, cow_data->data,
               (new_capacity < cow_data->capacity ? new_capacity : cow_data->capacity) * sizeof(T));
        free(cow_data->data);
        cow_data->data = new_data;
        cow_data->capacity = new_capacity;
    }
    _FORCE_INLINE_ void push_back(const T& p_value) {
        move();
        const auto new_capacity = cow_data->capacity + 1;
        resize(new_capacity);
        cow_data->data[new_capacity - 1] = p_value;
    }
    _FORCE_INLINE_ void pop_back() {
        move();
        if (!cow_data->length || !cow_data->data) {
            return;
        }
        cow_data->length--;
        const auto new_capacity = cow_data->capacity - 1;
        resize(new_capacity);
    }
    _FORCE_INLINE_ void arr_copy(const T* p_from, const size_t& p_size, const size_t idx){
        move();
        resize(p_size + idx);
        memcpy(&ptrw()[idx], p_from, sizeof(T) * p_size);
    }
    _NO_DISCARD_ _ALWAYS_INLINE_ size_t capacity() const { return cow_data->capacity; }
    _NO_DISCARD_ _ALWAYS_INLINE_ const T* ptr() const { return cow_data->data; }
    _NO_DISCARD_ _ALWAYS_INLINE_ T* ptrw() { move(); return cow_data->data; }
};

#endif //NEXUS_COW_H
