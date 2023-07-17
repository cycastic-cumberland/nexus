//
// Created by cycastic on 6/17/2023.
//

#ifndef NEXUS_REFERENCE_H
#define NEXUS_REFERENCE_H

#include "safe_refcount.h"

class Object {
private:
    mutable SafeRefCount refcount;
public:
    _ALWAYS_INLINE_ bool ref() const { return refcount.ref(); }
    _ALWAYS_INLINE_ bool unref() const { return refcount.unref(); }
    _ALWAYS_INLINE_ unsigned int get_reference_count() const { return refcount.get(); }
    Object() = default;
    virtual ~Object() = default;
};

template <class T>
class InteropObject : public Object{
private:
    T* native;
public:
    InteropObject() : Object() {
        native = new T();
    }
    ~InteropObject() override {
        delete native;
    }
    _FORCE_INLINE_ const T* ptr() const { return native; }
    _FORCE_INLINE_ T* ptrw() { return native; }
    operator T *() { return ptrw(); }
    operator const T *() const { return ptr(); }
};

template <class T>
class Ref {
    T* reference{};

    void ref(const Ref& p_from) {
        if (p_from.reference == reference) return;
        unref();
        reference = p_from.reference;
        if (reference) reference->ref();
    }
public:
    void unref() {
        if (reference && reference->unref()) memdelete(reference);
    }
    _FORCE_INLINE_ bool operator==(const T *p_ptr) const {
        return reference == p_ptr;
    }
    _FORCE_INLINE_ bool operator!=(const T *p_ptr) const {
        return reference != p_ptr;
    }

    _FORCE_INLINE_ bool operator<(const Ref<T> &p_r) const {
        return reference < p_r.reference;
    }
    _FORCE_INLINE_ bool operator==(const Ref<T> &p_r) const {
        return reference == p_r.reference;
    }
    _FORCE_INLINE_ bool operator!=(const Ref<T> &p_r) const {
        return reference != p_r.reference;
    }

    _FORCE_INLINE_ T *operator->() {
        return reference;
    }

    _FORCE_INLINE_ T *operator*() {
        return reference;
    }

    _FORCE_INLINE_ const T *operator->() const {
        return reference;
    }

    _FORCE_INLINE_ const T *ptr() const {
        return reference;
    }
    _FORCE_INLINE_ T *ptr() {
        return reference;
    }

    _FORCE_INLINE_ const T *operator*() const {
        return reference;
    }

    Ref& operator=(const Ref &p_from) {
        ref(p_from);
        return *this;
    }
    Ref(const Ref &p_from) {
        ref(p_from);
    }
    explicit Ref(T *p_reference) {
        if (p_reference) {
            ref_pointer(p_reference);
        }
    }
    _NO_DISCARD_ _ALWAYS_INLINE_ bool is_valid() const { return reference != nullptr; }
    _NO_DISCARD_ _ALWAYS_INLINE_ bool is_null() const { return reference == nullptr; }

    void instantiate() {
        ref(new T);
    }

    Ref() {}

    ~Ref() {
        unref();
    }
};

#endif //NEXUS_REFERENCE_H
