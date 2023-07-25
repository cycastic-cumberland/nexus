//
// Created by cycastic on 6/17/2023.
//

#ifndef NEXUS_TYPEDEFS_H
#define NEXUS_TYPEDEFS_H

#include <cstdlib>
#include <type_traits>
#include "memf.h"

// Stolen from Godot
#ifndef SWAP

#define SWAP(m_x, m_y) __swap_tmpl((m_x), (m_y))
template <class T>
inline void __swap_tmpl(T &x, T &y) {
    T aux = x;
    x = y;
    y = aux;
}

#endif

// Should always inline no matter what.
#ifndef _ALWAYS_INLINE_
#if defined(__GNUC__)
#define _ALWAYS_INLINE_ __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define _ALWAYS_INLINE_ __forceinline
#else
#define _ALWAYS_INLINE_ inline
#endif
#endif

#define DEV_ENABLED

// Should always inline, except in dev builds because it makes debugging harder.
#ifndef _FORCE_INLINE_
#ifdef DEV_ENABLED
#define _FORCE_INLINE_ inline
#else
#define _FORCE_INLINE_ _ALWAYS_INLINE_
#endif
#endif

// No discard allows the compiler to flag warnings if we don't use the return value of functions / classes
#ifndef _NO_DISCARD_
#define _NO_DISCARD_ [[nodiscard]]
#endif

// In some cases _NO_DISCARD_ will get_instance false positives,
// we can prevent the warning in specific cases by preceding the call with a cast.
#ifndef _ALLOW_DISCARD_
#define _ALLOW_DISCARD_ (void)
#endif

#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) x
#define unlikely(x) x
#endif

#define GUARD(lock_by_reference) LockGuard __guard(lock_by_reference)
#define R_GUARD(lock_by_reference) ReadLockGuard __r_guard(lock_by_reference)
#define W_GUARD(lock_by_reference) WriteLockGuard __w_guard(lock_by_reference)

static _FORCE_INLINE_ size_t string_length(const char* p_str){
    size_t re = 0;
    for (;;re++){
        if (p_str[re] == 0) break;
    }
    return re;
}

#endif //NEXUS_TYPEDEFS_H
