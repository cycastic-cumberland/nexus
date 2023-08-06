//
// Created by cycastic on 06/08/2023.
//

#ifndef NEXUS_BOX_H
#define NEXUS_BOX_H

#include "object.h"

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
    _FORCE_INLINE_ void clear_reference() { inner_ptr = Ref<Box<T, RefCounter>::InnerPointer>::null(); }
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

#endif //NEXUS_BOX_H
