//
// Created by cycastic on 6/17/2023.
//

#ifndef NEXUS_TYPEDEFS_H
#define NEXUS_TYPEDEFS_H

#include <cstdlib>
#include <type_traits>
#include "memf.h"

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
#define _FORCE_INLINE_
#else
#define _FORCE_INLINE_ _ALWAYS_INLINE_
#endif
#endif

// No discard allows the compiler to flag warnings if we don't use the return value of functions / classes
#ifndef _NO_DISCARD_
#define _NO_DISCARD_ [[nodiscard]]
#endif

// In some cases _NO_DISCARD_ will get false positives,
// we can prevent the warning in specific cases by preceding the call with a cast.
#ifndef _ALLOW_DISCARD_
#define _ALLOW_DISCARD_ (void)
#endif

static _FORCE_INLINE_ size_t strlen(const char* p_str){
    size_t re = 0;
    for (;;re++){
        if (p_str[re] == 0) break;
    }
    return re;
}

#endif //NEXUS_TYPEDEFS_H
