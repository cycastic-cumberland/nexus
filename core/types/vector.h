//
// Created by cycastic on 7/17/2023.
//

#ifndef NEXUS_VECTOR_H
#define NEXUS_VECTOR_H

#include "vstring.h"
#include "../comparator.h"

template<class T>
class Vector {
private:
    struct CowData {
        T* data{};
        size_t capacity{};
        size_t size{};
        SafeRefCount refcount{};
        CowData(){
            refcount.init();
        }
    };
    CowData *cow_data{};
public:
    // If there are future problems, then this will probably be the cause
    _FORCE_INLINE_ void clear(){
        unref();
        cow_data = new CowData();
    }
private:
    void unref(){
        if (cow_data && cow_data->refcount.unref()) {
            if (!std::is_trivially_destructible<T>::value)
                for (size_t i = 0; i < cow_data->size; i++){
                    cow_data->data[i].~T();
                }
            free(cow_data->data);
            delete cow_data;
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
    void internal_copy(const size_t& p_cap){
        auto new_data = new CowData();
        // Allocate the new array with set capacity
        new_data->data = (T*) malloc(sizeof(T) * p_cap);
        auto copy_ceiling = p_cap > cow_data->size ? cow_data->size : p_cap;
        // If primitive, use memcpy
        if (std::is_trivially_copy_constructible<T>::value && cow_data->data)
            memcpy(new_data->data, cow_data->data, copy_ceiling * sizeof(T));
        else for (size_t i = 0; i < copy_ceiling; i++){
                new (&new_data->data[i]) T(cow_data->data[i]);
            }
        new_data->capacity = p_cap;
        new_data->size = cow_data->size;
        unref();
        cow_data = new_data;
    }
    void move() {
        // Already own this cow_data
        if (cow_data->refcount.get() == 1) return;
        internal_copy(cow_data->capacity);
    }
    _FORCE_INLINE_ void internal_resize(const size_t& new_capacity){
        if (new_capacity == capacity()) return;
        if (new_capacity == 0) {
            clear();
            return;
        }
//        move();
        internal_copy(new_capacity);
    }
public:
    _NO_DISCARD_ _FORCE_INLINE_ size_t capacity() const { return cow_data->capacity; }
    _NO_DISCARD_ _FORCE_INLINE_ size_t size() const { return cow_data->size; }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return size() == 0; }
    _NO_DISCARD_ _FORCE_INLINE_ const T* ptr() const { return cow_data->data; }
    _FORCE_INLINE_ T* ptrw() { move(); return cow_data->data; }
    // No range check for extra performance
    _FORCE_INLINE_ const T& operator[](const size_t& idx) const { return ptr()[idx]; }
    _FORCE_INLINE_ T& operator[](const size_t& idx) { return ptrw()[idx]; }
    _FORCE_INLINE_ const T& last() const { return operator[](size() - 1); }
    _FORCE_INLINE_ T& last() { return operator[](size() - 1); }
public:
    struct Iterator {
    private:
        T *elem_ptr = nullptr;
    public:
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
    };
    struct ConstIterator {
    private:
        const T *elem_ptr = nullptr;
    public:
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
    };
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
    _FORCE_INLINE_ void push_back(const T& p_value){
        if (size() == capacity()) internal_resize(capacity() ? capacity() * 2 : 1);
        new (&ptrw()[cow_data->size++]) T(p_value);
    }
    template<class... Args>
    _FORCE_INLINE_ void emplace(Args&& ...args){
        if (size() == capacity()) internal_resize(capacity() ? capacity() * 2 : 1);
        new (&ptrw()[cow_data->size++]) T(args...);
    }
    _FORCE_INLINE_ void move_back(T&& p_value){
        if (size() == capacity()) internal_resize(capacity() ? capacity() * 2 : 1);
        new (&ptrw()[cow_data->size++]) T(p_value);
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
    _FORCE_INLINE_ void insert(const size_t& idx, const T& p_item){
        if (idx > size()) throw std::out_of_range("idx must be lower or equal to size");
        if (size() == idx) { push_back(p_item); return; }
        if (size() == capacity()) internal_resize(capacity() ? capacity() * 2 : 1);
        auto arr = ptrw();
        memmove(&arr[idx + 1], &arr[idx], sizeof(T) * (size() - idx));
        new (&arr[idx]) T(p_item);
        cow_data->size++;
    }
    template<class... Args>
    _FORCE_INLINE_ void emplace_at(const size_t& idx, Args&& ...args){
        if (idx > size()) throw std::out_of_range("idx must be lower or equal to size");
        if (size() == idx) { emplace(args...); return; }
        if (size() == capacity()) internal_resize(capacity() ? capacity() * 2 : 1);
        auto arr = ptrw();
        memmove(&arr[idx + 1], &arr[idx], sizeof(T) * (size() - idx));
        new (&arr[idx]) T(args...);
        cow_data->size++;
    }
    _FORCE_INLINE_ void remove(const size_t& idx){
        if (idx >= size()) throw std::out_of_range("idx must be lower than size");
        auto arr = ptrw();
        if (!std::is_trivially_destructible<T>::value) arr[idx].~T();
        memmove(&arr[idx], &arr[idx + 1], sizeof(T) * (size() - idx - 1));
        cow_data->size--;
        if (size() <= capacity() / 2) internal_resize(capacity() / 2);
    }
    template <class Comparator = StandardComparator<T>>
    _FORCE_INLINE_ int64_t find(const T& p_item){
        for (size_t i = 0; i < size(); i++){
            if (Comparator::compare(ptr()[i], p_item)) return int64_t(i);
        }
        return -1;
    }
    _FORCE_INLINE_ bool erase(const T& p_item){
        auto idx = find(p_item);
        if (idx == -1) return false;
        remove(int64_t(idx));
        return true;
    }
    _FORCE_INLINE_ void pop_back(){
        if (size() == 0) throw std::out_of_range("Vector is empty");
        cow_data->size--;
        // Call deconstruct
        // Does not actually deallocate for... obvious reason
        if (!std::is_trivially_destructible<T>::value) ptrw()[size()].~T();
        if (size() <= capacity() / 2) internal_resize(capacity() / 2);
    }
    _FORCE_INLINE_ Vector& operator=(const Vector& p_other) {
        ref(const_cast<CowData*>(p_other.cow_data));
        return *this;
    }
    _FORCE_INLINE_ Vector() { cow_data = new CowData(); }
    _FORCE_INLINE_ explicit Vector(const size_t& p_capacity) : Vector() {
        cow_data->capacity = p_capacity;
        cow_data->data = (T*)malloc(sizeof(T) * p_capacity);
    }
    _FORCE_INLINE_ Vector(const Vector& p_other){
        ref(const_cast<CowData*>(p_other.cow_data));
    }
    _FORCE_INLINE_ ~Vector(){
        unref();
    }
    _FORCE_INLINE_ Vector(const std::initializer_list<T>& p_init_list) : Vector(p_init_list.size()) {
        for (const auto& item : p_init_list){
            push_back(item);
        }
    }
    _FORCE_INLINE_ Vector& operator=(const std::initializer_list<T>& p_init_list) {
        this->operator=(Vector(p_init_list));
        return *this;
    }
};

//template <typename T>
//class Vector {
//    CowArray<T> data;
//    size_t usage = 0;
//public:
//    _NO_DISCARD_ _FORCE_INLINE_ size_t capacity() const { return data.capacity(); }
//    _NO_DISCARD_ _FORCE_INLINE_ size_t size() const { return usage; }
//    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return size() == 0; }
//    // Stolen from Godot
//    struct Iterator {
//        _FORCE_INLINE_ T &operator*() const {
//            return *elem_ptr;
//        }
//        _FORCE_INLINE_ T *operator->() const { return elem_ptr; }
//        _FORCE_INLINE_ Iterator &operator++() {
//            elem_ptr++;
//            return *this;
//        }
//        _FORCE_INLINE_ Iterator &operator--() {
//            elem_ptr--;
//            return *this;
//        }
//
//        _FORCE_INLINE_ bool operator==(const Iterator &b) const { return elem_ptr == b.elem_ptr; }
//        _FORCE_INLINE_ bool operator!=(const Iterator &b) const { return elem_ptr != b.elem_ptr; }
//
//        explicit Iterator(T *p_ptr) { elem_ptr = p_ptr; }
//        Iterator() = default;
//        Iterator(const Iterator &p_it) { elem_ptr = p_it.elem_ptr; }
//
//    private:
//        T *elem_ptr = nullptr;
//    };
//    // Stolen from Godot
//    struct ConstIterator {
//        _FORCE_INLINE_ const T &operator*() const {
//            return *elem_ptr;
//        }
//        _FORCE_INLINE_ const T *operator->() const { return elem_ptr; }
//        _FORCE_INLINE_ ConstIterator &operator++() {
//            elem_ptr++;
//            return *this;
//        }
//        _FORCE_INLINE_ ConstIterator &operator--() {
//            elem_ptr--;
//            return *this;
//        }
//
//        _FORCE_INLINE_ bool operator==(const ConstIterator &b) const { return elem_ptr == b.elem_ptr; }
//        _FORCE_INLINE_ bool operator!=(const ConstIterator &b) const { return elem_ptr != b.elem_ptr; }
//
//        explicit ConstIterator(const T *p_ptr) { elem_ptr = p_ptr; }
//        ConstIterator() = default;
//        ConstIterator(const ConstIterator &p_it) { elem_ptr = p_it.elem_ptr; }
//
//    private:
//        const T *elem_ptr = nullptr;
//    };
//    _NO_DISCARD_ _FORCE_INLINE_ T* ptrw() { return data.ptrw(); }
//    _NO_DISCARD_ _FORCE_INLINE_ const T* ptr() const { return data.ptr(); }
//    _FORCE_INLINE_ Iterator begin() {
//        return Iterator(ptrw());
//    }
//    _FORCE_INLINE_ Iterator end() {
//        return Iterator(ptrw() + size());
//    }
//    _FORCE_INLINE_ ConstIterator begin() const {
//        return ConstIterator(ptr());
//    }
//    _FORCE_INLINE_ ConstIterator end() const {
//        return ConstIterator(ptr() + size());
//    }
//public:
//    _FORCE_INLINE_ void range_check(const size_t& idx) const { if (idx >= size()) throw std::out_of_range("Index out of range"); }
//    _FORCE_INLINE_ const T& operator[](const size_t& idx) const { range_check(idx); return data.ptr()[idx]; }
//    _FORCE_INLINE_ T& operator[](const size_t& idx) { range_check(idx); return data.ptrw()[idx]; }
//    _FORCE_INLINE_ const T& last() const { return operator[](size() - 1); }
//    _FORCE_INLINE_ T& last() { return operator[](size() - 1); }
//    _FORCE_INLINE_ void push_back(const T& p_value){
//        if (size() == capacity()) data.resize(capacity() == 0 ? 1 : capacity() * 2);
//        new (&data.ptrw()[usage++]) T(p_value);
//    }
//    template<class... Args>
//    _FORCE_INLINE_ void emplace(Args&& ...args){
//        if (size() == capacity()) data.resize(capacity() == 0 ? 1 : capacity() * 2);
//        new (&data.ptrw()[usage++]) T(args...);
//    }
//    _FORCE_INLINE_ void move_back(T&& p_value){
//        if (size() == capacity()) data.resize(capacity() == 0 ? 1 : capacity() * 2);
//        new (&data.ptrw()[usage++]) T(p_value);
//    }
//    _FORCE_INLINE_ void append(const Vector<T>& p_other){
//        for (int i = 0; i < p_other.size(); i++){
//            push_back(p_other[i]);
//        }
//    }
//    _FORCE_INLINE_ void append(const std::initializer_list<T>& p_init_list){
//        for (const auto& item : p_init_list){
//            push_back(item);
//        }
//    }
//    _FORCE_INLINE_ void insert(const size_t& idx, const T& p_item){
//        if (idx > size()) throw std::out_of_range("idx must be lower or equal to size");
//        if (size() == idx) { push_back(p_item); return; }
//        memmove(&data.ptrw()[idx + 1], &data.ptrw()[idx], sizeof(T) * (size() - idx));
//        new (&data[idx]) T(p_item);
//        usage++;
//    }
//    template<class... Args>
//    _FORCE_INLINE_ void emplace_at(const size_t& idx, Args&& ...args){
//        if (idx > size()) throw std::out_of_range("idx must be lower or equal to size");
//        if (size() == idx) { emplace(args...); return; }
//        memmove(&data.ptrw()[idx + 1], &data.ptrw()[idx], sizeof(T) * (size() - idx));
//        new (&data.ptrw()[idx]) T(args...);
//        usage++;
//    }
//    _FORCE_INLINE_ void remove(const size_t& idx){
//        if (idx >= size()) throw std::out_of_range("idx must be lower than size");
//        if (!std::is_trivially_destructible<T>::value) data.ptrw()[idx].~T();
//        memmove(&data.ptrw()[idx], &data.ptrw()[idx + 1], sizeof(T) * (size() - idx - 1));
//        usage--;
//        if (size() <= capacity() / 2) data.resize(capacity() / 2);
//    }
//    template <class Comparator = StandardComparator<T>>
//    _FORCE_INLINE_ int64_t find(const T& p_item){
//        for (size_t i = 0; i < size(); i++){
//            if (Comparator::compare((*this)[i], p_item)) return int64_t(i);
//        }
//        return -1;
//    }
//    _FORCE_INLINE_ bool erase(const T& p_item){
//        auto idx = find(p_item);
//        if (idx == -1) return false;
//        remove(int64_t(idx));
//        return true;
//    }
//    _FORCE_INLINE_ void pop_back(){
//        if (size() == 0) throw std::out_of_range("Vector is empty");
//        --usage;
//        // Call deconstruct
//        // Does not actually deallocate for... obvious reason
//        if (!std::is_trivially_destructible<T>::value) data.ptrw()[size()].~T();
//        if (size() <= capacity() / 2) data.resize(capacity() / 2);
//    }
//    _FORCE_INLINE_ void clear() {
//        // Manually object_destroy every element;
//        while (!empty()) pop_back();
//        usage = 0;
//    }
//    _FORCE_INLINE_ Vector() { data = CowArray<T>(); }
//    _FORCE_INLINE_ explicit Vector(const size_t& starting_capacity) : Vector() { data.resize(starting_capacity); }
//    _FORCE_INLINE_ Vector(const std::initializer_list<T>& p_init_list) : Vector(p_init_list.size()) {
//        for (const auto& item : p_init_list){
//            push_back(item);
//        }
//    }
//    _FORCE_INLINE_ Vector(const Vector& p_other) { data = p_other.data; usage = p_other.usage; }
//    _FORCE_INLINE_ Vector& operator=(const Vector& p_other) { clear(); data = p_other.data; usage = p_other.usage; return *this; }
//    _FORCE_INLINE_ Vector& operator=(const std::initializer_list<T>& p_other) {
//        *this = Vector(p_other);
//        return *this;
//    }
//    _FORCE_INLINE_ Vector& operator+=(const T& p_item){
//        push_back(p_item);
//        return *this;
//    }
//    _FORCE_INLINE_ Vector& operator+=(const Vector& p_other){
//        push_back(p_other);
//        return *this;
//    }
//    _FORCE_INLINE_ Vector& operator+=(const std::initializer_list<T>& p_other){
//        push_back(p_other);
//        return *this;
//    }
//    _FORCE_INLINE_ Vector operator+(const T& p_item){
//        Vector re{*this};
//        re += p_item;
//        return re;
//    }
//    _FORCE_INLINE_ Vector operator+(const Vector& p_other){
//        Vector re{*this};
//        re += p_other;
//        return re;
//    }
//    _FORCE_INLINE_ ~Vector() {
//        clear();
//    }
//    friend std::wostream& operator<<(std::wostream& p_ostream, const Vector& p_vec){
//        p_ostream << VString("[ ");
//        if (p_vec.size() == 1){
//            p_ostream << p_vec[0];
//        } else if (p_vec.size() > 1){
//            size_t s = p_vec.size();
//            for (size_t i = 0; i < s - 1; i++){
//                p_ostream << p_vec[i] << VString(", ");
//            }
//            p_ostream << p_vec[s - 1];
//        }
//        p_ostream << VString(" ]");
//        return p_ostream;
//    }
//    friend std::ostream& operator<<(std::ostream& p_ostream, const Vector& p_vec){
//        p_ostream << "[ ";
//        if (p_vec.size() == 1){
//            p_ostream << p_vec[0];
//        } else if (p_vec.size() > 1){
//            size_t s = p_vec.size();
//            for (size_t i = 0; i < s - 1; i++){
//                p_ostream << p_vec[i] << ", ";
//            }
//            p_ostream << p_vec[s - 1];
//        }
//        p_ostream << " ]";
//        return p_ostream;
//    }
//};

#endif //NEXUS_VECTOR_H
