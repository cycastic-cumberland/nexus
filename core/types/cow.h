//
// Created by cycastic on 6/17/2023.
//

#ifndef NEXUS_COW_H
#define NEXUS_COW_H

#include "safe_refcount.h"
#include "lock.h"

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
    CowData *data{};
    void unref(){
        if (data && data->refcount.unref()) {
            free(data->data);
            memdelete(data);
        }
    }
    void ref(CowData* p_data){
        // Does not lock
        if (p_data == data) return;
        unref();
        data = p_data;
        if (data){
            data->refcount.ref();
        }
    }
    void move() {
        if (data->refcount.get() == 1) return;
        auto new_data = new CowData();
        new_data->data = (T*) malloc(sizeof(T) * data->capacity);
        memcpy(new_data->data, data->data, data->capacity * sizeof(T));
        new_data->capacity = data->capacity;
        unref();
        data = new_data;
    }
public:
    _FORCE_INLINE_ CowArray& operator=(const CowArray& p_other) {
        ref(const_cast<CowData*>(p_other.data));
        return *this;
    }
    _FORCE_INLINE_ CowArray(){ data = new CowData(); }
    _FORCE_INLINE_ CowArray(const CowArray& p_other) {
        *this = p_other;
    }
    _FORCE_INLINE_ ~CowArray(){
        unref();
    }
    _FORCE_INLINE_ void clear(){
        move();
        free(data->data);
        data->capacity = 0;
    }
    _FORCE_INLINE_ void resize(const size_t& new_capacity){
        if (new_capacity == data->capacity) return;
        if (new_capacity == 0){
            clear(); return;
        }
        move();
        auto new_data = (T*)malloc(sizeof(T) * new_capacity);
        memcpy(new_data, data->data,
               (new_capacity < data->capacity ? new_capacity : data->capacity) * sizeof(T));
        free(data->data);
        data->data = new_data;
        data->capacity = new_capacity;
    }
    _FORCE_INLINE_ void push_back(const T& p_value) {
        move();
        // Already exclusively own the internal data, no need to guard
        if (data->length >= data->capacity){
            const auto new_capacity = data->capacity == 0 ? 1 : data->capacity + 1;
            resize(new_capacity);
        }
        data->data[data->length++] = p_value;
    }
    _FORCE_INLINE_ void pop_back() {
        move();
        if (!data->length || !data->data) {
            return;
        }
        data->length--;
        const auto new_capacity = data->capacity - 1;
        resize(new_capacity);
    }
    _FORCE_INLINE_ void arr_copy(const T* p_from, const size_t& p_size, const size_t idx){
        move();
        resize(p_size + capacity());
        memcpy(&ptrw()[idx], p_from, sizeof(T) * p_size);
    }
    _NO_DISCARD_ _ALWAYS_INLINE_ size_t capacity() const { return data->capacity; }
    _NO_DISCARD_ _ALWAYS_INLINE_ const T* ptr() const { return data->data; }
    _NO_DISCARD_ _ALWAYS_INLINE_ T* ptrw() { return data->data; }
};

#endif //NEXUS_COW_H
