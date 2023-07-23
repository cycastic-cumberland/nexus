//
// Created by cycastic on 7/19/2023.
//

#ifndef NEXUS_OBJECT_H
#define NEXUS_OBJECT_H

#include "safe_refcount.h"
#include "../lock.h"
#include "vector.h"
#include "hashmap.h"
#include "reference.h"

class ObjectDB;

class Object {
private:
    mutable SafeRefCount refcount{};
    uint64_t object_id{};

    friend class ObjectDB;
public:
    _ALWAYS_INLINE_ void init_ref() { refcount.init(); }
    _ALWAYS_INLINE_ bool ref() const { return refcount.ref(); }
    _ALWAYS_INLINE_ bool unref() const { return refcount.unref(); }
    _ALWAYS_INLINE_ unsigned int get_reference_count() const { return refcount.get(); }
    _ALWAYS_INLINE_ uint64_t get_object_id() const { return object_id; }
    Object();
    virtual ~Object();
};

class ObjectDB {
private:
    static HashMap<uint64_t, Object*> objects_registry;
    static SafeNumeric<uint64_t> refcount;
    static RWLock lock;

    friend class Object;
private:
    static void register_object(Object* obj);
    static void remove_object(Object* obj);
public:
    template<class T>
    static _ALWAYS_INLINE_ Ref<T> create_instance(T* p_from_pointer = nullptr){
        return Ref<T>::init(p_from_pointer);
    }
    static Object* get_instance(const uint64_t& p_id);
    static Vector<uint64_t> get_all_objects_id();
};

template <typename T>
class Box {
private:
    class InnerPointer : public Object {
        T data;
        explicit InnerPointer(const T& p_data) { data = p_data; }

        friend class Box<T>;
    };
    Ref<InnerPointer> inner_ptr{};
public:
    _FORCE_INLINE_ bool operator<(const Box<T> &p_r) const {
        return inner_ptr < p_r.inner_ptr;
    }
    _FORCE_INLINE_ bool operator==(const Box<T> &p_r) const {
        return inner_ptr == p_r.inner_ptr;
    }
    _FORCE_INLINE_ bool operator!=(const Box<T> &p_r) const {
        return inner_ptr != p_r.inner_ptr;
    }
    _FORCE_INLINE_ T *operator->() {
        return &inner_ptr.ptr()->data;
    }
    _FORCE_INLINE_ T *operator*() {
        return &inner_ptr.ptr()->data;
    }
    _FORCE_INLINE_ const T *operator->() const {
        return &inner_ptr.ptr()->data;
    }
    _FORCE_INLINE_ const T *ptr() const {
        return &inner_ptr.ptr()->data;
    }
    _FORCE_INLINE_ T *ptr() {
        return &inner_ptr.ptr()->data;
    }
    _FORCE_INLINE_ const T *operator*() const {
        return &inner_ptr.ptr()->data;
    }
    _NO_DISCARD_ _FORCE_INLINE_ bool is_null() const { return inner_ptr.is_null(); }
    _NO_DISCARD_ _FORCE_INLINE_ bool is_valid() const { return inner_ptr.is_valid(); }
    _FORCE_INLINE_ Box& operator=(const T& p_data){
        if (is_valid()) *(ptr()) = p_data;
        else inner_ptr = new InnerPointer(p_data);
        return *this;
    }
    _FORCE_INLINE_ Box& operator=(const Box& p_other){
        inner_ptr = p_other.inner_ptr;
        return *this;
    }
    _FORCE_INLINE_ void clear_reference() { inner_ptr = nullptr; }
    Box() : inner_ptr() {}
    Box(const T& p_data) {
        inner_ptr = new InnerPointer(p_data);
    }
    Box(const Box& p_other) {
        inner_ptr = p_other.inner_ptr;
    }
    template<class... Args >
    static Box<T> make_box(Args&&... args){
        return Box(T(args...));
    }
};
#endif //NEXUS_OBJECT_H
