//
// Created by cycastic on 7/18/2023.
//

#ifndef NEXUS_MATHLIB_H
#define NEXUS_MATHLIB_H

#include <cmath>
#include "typedefs.h"

class Math {
public:
    static _ALWAYS_INLINE_ bool is_nan(double p_val) {
#ifdef _MSC_VER
        return _isnan(p_val);
#elif defined(__GNUC__) && __GNUC__ < 6
        union {
			uint64_t u;
			double f;
		} ieee754;
		ieee754.f = p_val;
		// (unsigned)(0x7ff0000000000001 >> 32) : 0x7ff00000
		return ((((unsigned)(ieee754.u >> 32) & 0x7fffffff) + ((unsigned)ieee754.u != 0)) > 0x7ff00000);
#else
        return std::isnan(p_val);
#endif
    }

    template<typename T>
    static _ALWAYS_INLINE_ T max(const T& p_left, const T& p_right){
        return p_left > p_right ? p_left : p_right;
    }
    template<typename T>
    static _ALWAYS_INLINE_ T abs(const T& p_val){
        return p_val > 0 ? p_val : -p_val;
    }
};

#define MAX(m_a, m_b) Math::max(m_a, m_b)
#define ABS(m_val) Math::abs(m_val)

#endif //NEXUS_MATHLIB_H
