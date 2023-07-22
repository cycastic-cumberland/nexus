//
// Created by cycastic on 7/18/2023.
//

#ifndef NEXUS_HASHFUNCS_H
#define NEXUS_HASHFUNCS_H

#include "mathlib.h"
#include "typedefs.h"

class VString;

static _FORCE_INLINE_ uint32_t djb2_string_hash(const wchar_t* str){
    unsigned long hash = 5381;
    wchar_t c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + static_cast<unsigned long>(c); // hash * 33 + c

    return hash;
}

static _FORCE_INLINE_ uint32_t djb2_char_hash(const char* str){
    unsigned long hash = 5381;
    char c;
    while ((c = *str++))
    {
        hash = (hash << 5) + hash + c; /* hash * 33 + c */
    }
    return hash;
}

static _FORCE_INLINE_ uint32_t hash_one_uint64(const uint64_t p_int) {
    uint64_t v = p_int;
    v = (~v) + (v << 18); // v = (v << 18) - v - 1;
    v = v ^ (v >> 31);
    v = v * 21; // v = (v + (v << 2)) + (v << 4);
    v = v ^ (v >> 11);
    v = v + (v << 6);
    v = v ^ (v >> 22);
    return (int)v;
}

static _FORCE_INLINE_ uint32_t hash_djb2_one_float(double p_in, uint32_t p_prev = 5381) {
    union {
        double d;
        uint64_t i;
    } u{};

    // Normalize +/- 0.0 and NaN values, so they hash the same.
    if (p_in == 0.0f) {
        u.d = 0.0;
    } else if (Math::is_nan(p_in)) {
        u.d = NAN;
    } else {
        u.d = p_in;
    }

    return ((p_prev << 5) + p_prev) + hash_one_uint64(u.i);
}

struct StandardHasher {
    static _FORCE_INLINE_ uint32_t hash(const char* p_data) { return djb2_char_hash(p_data); }
    static uint32_t hash(const VString& p_data);
    static _FORCE_INLINE_ uint32_t hash(const double & p_data) { return hash_djb2_one_float(p_data); }
    static _FORCE_INLINE_ uint32_t hash(const float & p_data) { return hash_djb2_one_float(p_data); }
    static _FORCE_INLINE_ uint32_t hash(const uint64_t & p_data) { return hash_one_uint64(p_data); }
    static _FORCE_INLINE_ uint32_t hash(const int64_t & p_data) { return hash((uint64_t)p_data); }
    static _FORCE_INLINE_ uint32_t hash(const uint32_t & p_data) { return hash((uint64_t)p_data); }
    static _FORCE_INLINE_ uint32_t hash(const int32_t & p_data) { return hash((int64_t)p_data); }
    static _FORCE_INLINE_ uint32_t hash(const uint16_t & p_data) { return hash((uint64_t)p_data); }
    static _FORCE_INLINE_ uint32_t hash(const int16_t & p_data) { return hash((int64_t)p_data); }
    static _FORCE_INLINE_ uint32_t hash(const uint8_t & p_data) { return hash((uint64_t)p_data); }
    static _FORCE_INLINE_ uint32_t hash(const int8_t & p_data) { return hash((int64_t)p_data); }
};

#endif //NEXUS_HASHFUNCS_H
