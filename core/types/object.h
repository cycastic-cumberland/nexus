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

class BaseObject {
public:
    virtual void init_ref() const = 0;
    virtual bool ref() const = 0;
    virtual bool unref() const = 0;
    _NO_DISCARD_ virtual uint32_t get_reference_count() const = 0;

    virtual ~BaseObject() = default;
};

// Decentralized, Safe Object
class SafeObject : public BaseObject {
private:
    mutable SafeRefCount refcount{};
public:
    void init_ref() const override { refcount.init(); }
    bool ref() const override { return refcount.ref(); }
    bool unref() const override { return refcount.unref(); }
    uint32_t get_reference_count() const override { return refcount.get(); }

    ~SafeObject() override = default;
};

// Decentralized, Unsafe Object
class UnsafeObject : public BaseObject {
private:
    mutable uint32_t refcount{};
public:
    void init_ref() const override { refcount = 1; }
    bool ref() const override { return (refcount == 0 ? 0 : ++refcount) != 0; }
    bool unref() const override { return --refcount == 0; }
    uint32_t get_reference_count() const override { return refcount; }

    ~UnsafeObject() override = default;
};

// Centralized, Safe Object
class ManagedObject : public SafeObject {
private:
    uint64_t object_id{};

    friend class ObjectDB;
public:
    uint64_t get_object_id() const { return object_id; }
    ManagedObject();
    ~ManagedObject() override;
};

class ObjectDB {
private:
    static HashMap<uint64_t, ManagedObject*> objects_registry;
    static SafeNumeric<uint64_t> refcount;
    static RWLock lock;

    friend class ManagedObject;
private:
    static void register_object(ManagedObject* obj);
    static void remove_object(ManagedObject* obj);
public:
    static Ref<ManagedObject> get_instance(const uint64_t& p_id);
    static size_t get_object_count();
    static Vector<uint64_t> get_all_objects_id();

#ifdef DEBUG_ENABLED
    static Vector<ManagedObject*> get_all_instances();
#endif
};

template <typename T, class RefCounter = SafeObject>
class Box {
private:
    class InnerPointer : public RefCounter {
        T data;
    public:
        template<class... Args >
        explicit InnerPointer(Args&& ...args) : data(args...) {}

        friend class Box<T, RefCounter>;
    };
    Ref<InnerPointer> inner_ptr{};

public:
    _FORCE_INLINE_ bool operator==(const Box<T, RefCounter> &p_r) const {
        return inner_ptr == p_r.inner_ptr;
    }
    _FORCE_INLINE_ bool operator!=(const Box<T, RefCounter> &p_r) const {
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
    _FORCE_INLINE_ Box<T, RefCounter>& operator=(const T& p_data){
        if (is_valid()) *(ptr()) = p_data;
        else inner_ptr = Ref<Box<T, RefCounter>::InnerPointer>::make_ref(p_data);
        return *this;
    }
    _FORCE_INLINE_ Box& operator=(const Box<T, RefCounter>& p_other){
        inner_ptr = p_other.inner_ptr;
        return *this;
    }
    _FORCE_INLINE_ void clear_reference() { inner_ptr = nullptr; }
    Box() : inner_ptr() {}
    Box(const T& p_data) {
        inner_ptr = Ref<Box<T, RefCounter>::InnerPointer>::make_ref(p_data);
    }
    Box(const Box& p_other) {
        inner_ptr = p_other.inner_ptr;
    }
    template<class... Args >
    static Box<T, RefCounter> make_box(Args&&... args){
        Box<T, RefCounter> re{};
        re.inner_ptr = Ref<Box<T, RefCounter>::InnerPointer>::make_ref(args...);
        return re;
    }
    ~Box() = default;
};
#endif //NEXUS_OBJECT_H
