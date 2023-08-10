//
// Created by cycastic on 7/18/2023.
//

#ifndef NEXUS_MATHLIB_H
#define NEXUS_MATHLIB_H

#include <cmath>
#include "typedefs.h"

//#ifndef _HUGE_NUMBER
//#define _HUGE_NUMBER  1e+300
//#endif
//
//#define INFINITY   ((float)(_HUGE_NUMBER * _HUGE_NUMBER))
//#define NAN        (-(float)(INFINITY * 0.0F))

class Math {
public:
    static constexpr uint32_t max_u32 = 0xFFFFFFFF;
    static constexpr uint32_t min_u32 = 0x0;
    static constexpr int32_t  max_i32 = 0x7FFFFFFF;
    static constexpr  int32_t min_i32 = 0x80000000;
    static constexpr uint64_t max_u64 = 0xFFFFFFFFFFFFFFFF;
    static constexpr uint64_t min_u64 = 0x0;
    static constexpr  int64_t max_i64 = 0x7FFFFFFFFFFFFFFF;
    static constexpr  int64_t min_i64 = 0x8000000000000000;
    static constexpr  size_t max_ptr =
#if defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
            max_u32
#elif defined(__x86_64__) || defined(_M_X64)
            max_u64
#else
            0
#endif
        ;
    static constexpr  size_t min_ptr = 0x0;

    static _ALWAYS_INLINE_ bool is_infinite(const double& p_val) {
        return std::isinf(p_val);
    }
    static _ALWAYS_INLINE_ bool is_infinite(const float& p_val) {
        return std::isinf(p_val);
    }
    static _ALWAYS_INLINE_ bool is_nan(double p_val) {
        return std::isnan(p_val);
    }
    template<typename T>
    static _ALWAYS_INLINE_ T max(const T& p_left, const T& p_right){
        return p_left > p_right ? p_left : p_right;
    }
    template<typename T>
    static _ALWAYS_INLINE_ T abs(const T& p_val){
        return p_val > 0 ? p_val : -p_val;
    }
    template<typename T>
    static _ALWAYS_INLINE_ T clamp(const T& p_val, const T& p_lower, const T& p_upper){
        return p_val < p_lower ? p_lower : (p_val > p_upper ? p_upper : p_val);
    }
    static _ALWAYS_INLINE_ double clamp01(const double& p_val){
        return clamp(p_val, 0.0, 1.0);
    }
    static _ALWAYS_INLINE_ float clamp01(const float& p_val){
        return clamp(p_val, 0.0f, 1.0f);
    }
};

#define MAX(m_a, m_b) Math::max(m_a, m_b)
#define ABS(m_val) Math::abs(m_val)
#define CLAMP(m_val, m_low, m_high) Math::clamp(m_val, m_low, m_high)
#define CLAMP01(m_val) Math::clamp01(m_val)

#endif //NEXUS_MATHLIB_H
