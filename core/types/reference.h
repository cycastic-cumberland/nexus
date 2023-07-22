//
// Created by cycastic on 6/17/2023.
//

#ifndef NEXUS_REFERENCE_H
#define NEXUS_REFERENCE_H

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
        if (reference && reference->unref()) {
            memdelete(reference);
            reference = nullptr;
        }
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

    // Downward cast only
    template <class To>
    _FORCE_INLINE_ Ref<To> cast() const {
        auto casted = (Ref<To>*)(this);
        return *casted;
    }

    Ref& operator=(const Ref &p_from) {
        ref(p_from);
        return *this;
    }
    Ref(const Ref &p_from) {
        ref(p_from);
    }
    Ref(T *p_reference) {
        if (p_reference) {
            p_reference->init_ref();
            reference = p_reference;
        }
    }
    _NO_DISCARD_ _ALWAYS_INLINE_ bool is_valid() const { return reference != nullptr; }
    _NO_DISCARD_ _ALWAYS_INLINE_ bool is_null() const { return reference == nullptr; }

    void instantiate() {
        ref(new T);
    }

    Ref() = default;

    ~Ref() {
        unref();
    }

    static _ALWAYS_INLINE_ Ref<T> init(T* p_from){
        Ref<T> re = p_from;
        return re;
    }
    static _ALWAYS_INLINE_ Ref<T> init(){
        Ref<T> re = new T();
        return re;
    }
    template<class... Args >
    static _ALWAYS_INLINE_ Ref<T> make_ref(Args&&... args){
        return init(new T(args...));
    }
};

#endif //NEXUS_REFERENCE_H
