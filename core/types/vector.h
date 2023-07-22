//
// Created by cycastic on 7/17/2023.
//

#ifndef NEXUS_VECTOR_H
#define NEXUS_VECTOR_H

#include "vstring.h"

template <typename T>
class Vector {
    CowArray<T> data;
    size_t usage = 0;
public:
    // Stolen from Godot
    struct Iterator {
        _FORCE_INLINE_ T &operator*() const {
            return *elem_ptr;
        }
        _FORCE_INLINE_ T *operator->() const { return elem_ptr; }
        _FORCE_INLINE_ Iterator &operator++() {
            elem_ptr++;
            return *this;
        }
        _FORCE_INLINE_ Iterator &operator--() {
            elem_ptr--;
            return *this;
        }

        _FORCE_INLINE_ bool operator==(const Iterator &b) const { return elem_ptr == b.elem_ptr; }
        _FORCE_INLINE_ bool operator!=(const Iterator &b) const { return elem_ptr != b.elem_ptr; }

        explicit Iterator(T *p_ptr) { elem_ptr = p_ptr; }
        Iterator() = default;
        Iterator(const Iterator &p_it) { elem_ptr = p_it.elem_ptr; }

    private:
        T *elem_ptr = nullptr;
    };
    // Stolen from Godot
    struct ConstIterator {
        _FORCE_INLINE_ const T &operator*() const {
            return *elem_ptr;
        }
        _FORCE_INLINE_ const T *operator->() const { return elem_ptr; }
        _FORCE_INLINE_ ConstIterator &operator++() {
            elem_ptr++;
            return *this;
        }
        _FORCE_INLINE_ ConstIterator &operator--() {
            elem_ptr--;
            return *this;
        }

        _FORCE_INLINE_ bool operator==(const ConstIterator &b) const { return elem_ptr == b.elem_ptr; }
        _FORCE_INLINE_ bool operator!=(const ConstIterator &b) const { return elem_ptr != b.elem_ptr; }

        explicit ConstIterator(const T *p_ptr) { elem_ptr = p_ptr; }
        ConstIterator() = default;
        ConstIterator(const ConstIterator &p_it) { elem_ptr = p_it.elem_ptr; }

    private:
        const T *elem_ptr = nullptr;
    };
    _NO_DISCARD_ _FORCE_INLINE_ T* ptrw() { return data.ptrw(); }
    _NO_DISCARD_ _FORCE_INLINE_ const T* ptr() const { return data.ptr(); }
    _FORCE_INLINE_ Iterator begin() {
        return Iterator(ptrw());
    }
    _FORCE_INLINE_ Iterator end() {
        return Iterator(ptrw() + size());
    }
    _FORCE_INLINE_ ConstIterator begin() const {
        return ConstIterator(ptr());
    }
    _FORCE_INLINE_ ConstIterator end() const {
        return ConstIterator(ptr() + size());
    }
public:
    _NO_DISCARD_ _FORCE_INLINE_ size_t capacity() const { return data.capacity(); }
    _NO_DISCARD_ _FORCE_INLINE_ size_t size() const { return usage; }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return size() == 0; }
    _FORCE_INLINE_ void clear() {
        // Manually object_destroy every element;
        while (!empty()) pop_back();
        usage = 0;
    }
    _FORCE_INLINE_ void range_check(const size_t& idx) const { if (idx >= size()) throw std::out_of_range("Index out of range"); }
    _FORCE_INLINE_ const T& operator[](const size_t& idx) const { range_check(idx); return data.ptr()[idx]; }
    _FORCE_INLINE_ T& operator[](const size_t& idx) { range_check(idx); return data.ptrw()[idx]; }
    _FORCE_INLINE_ const T& last() const { return operator[](size() - 1); }
    _FORCE_INLINE_ void push_back(const T& p_value){
        if (size() == capacity()) data.resize(capacity() == 0 ? 1 : capacity() * 2);
        // Call copy constructor
        new (&data.ptrw()[usage++]) T(p_value);
    }
    _FORCE_INLINE_ void append(const Vector<T>& p_other){
        for (int i = 0; i < p_other.size(); i++){
            push_back(p_other[i]);
        }
    }
    _FORCE_INLINE_ void append(const std::initializer_list<T>& p_init_list){
        for (const auto& item : p_init_list){
            push_back(item);
        }
    }
    _FORCE_INLINE_ void pop_back(){
        if (size() == 0) throw std::out_of_range("Vector is empty");
        --usage;
        // Call deconstruct
        // Does not actually deallocate for... obvious reason
        if (!std::is_trivially_destructible<T>::value) data.ptrw()[size()].~T();
        if (size() <= capacity() / 2) data.resize(capacity() / 2);
    }

    _FORCE_INLINE_ Vector() { data = CowArray<T>(); }
    _FORCE_INLINE_ explicit Vector(const size_t& starting_capacity) : Vector() { data.resize(starting_capacity); }
    _FORCE_INLINE_ Vector(const std::initializer_list<T>& p_init_list) : Vector(p_init_list.size()) {
        for (const auto& item : p_init_list){
            push_back(item);
        }
    }
    _FORCE_INLINE_ Vector(const Vector& p_other) { data = p_other.data; usage = p_other.usage; }
    _FORCE_INLINE_ Vector& operator=(const Vector& p_other) { clear(); data = p_other.data; usage = p_other.usage; return *this; }
    _FORCE_INLINE_ Vector& operator=(const std::initializer_list<T>& p_other) {
        *this = Vector(p_other);
        return *this;
    }
    _FORCE_INLINE_ Vector& operator+=(const T& p_item){
        push_back(p_item);
        return *this;
    }
    _FORCE_INLINE_ Vector& operator+=(const Vector& p_other){
        push_back(p_other);
        return *this;
    }
    _FORCE_INLINE_ Vector& operator+=(const std::initializer_list<T>& p_other){
        push_back(p_other);
        return *this;
    }
    _FORCE_INLINE_ Vector operator+(const T& p_item){
        Vector re{*this};
        re += p_item;
        return re;
    }
    _FORCE_INLINE_ Vector operator+(const Vector& p_other){
        Vector re{*this};
        re += p_other;
        return re;
    }
    _FORCE_INLINE_ ~Vector() {
        clear();
    }
    friend std::wostream& operator<<(std::wostream& p_ostream, const Vector& p_vec){
        p_ostream << VString("[ ");
        if (p_vec.size() == 1){
            p_ostream << p_vec[0];
        } else if (p_vec.size() > 1){
            size_t s = p_vec.size();
            for (size_t i = 0; i < s - 1; i++){
                p_ostream << p_vec[i] << VString(", ");
            }
            p_ostream << p_vec[s - 1];
        }
        p_ostream << VString(" ]");
        return p_ostream;
    }
    friend std::ostream& operator<<(std::ostream& p_ostream, const Vector& p_vec){
        p_ostream << "[ ";
        if (p_vec.size() == 1){
            p_ostream << p_vec[0];
        } else if (p_vec.size() > 1){
            size_t s = p_vec.size();
            for (size_t i = 0; i < s - 1; i++){
                p_ostream << p_vec[i] << ", ";
            }
            p_ostream << p_vec[s - 1];
        }
        p_ostream << " ]";
        return p_ostream;
    }
};

#endif //NEXUS_VECTOR_H
