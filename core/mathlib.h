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
};

#endif //NEXUS_MATHLIB_H
