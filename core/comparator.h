//
// Created by cycastic on 7/18/2023.
//

#ifndef NEXUS_COMPARATOR_H
#define NEXUS_COMPARATOR_H

#include "mathlib.h"

template <typename T>
struct StandardComparator {
    static bool compare(const T &p_lhs, const T &p_rhs) {
        return p_lhs == p_rhs;
    }

    bool compare(const float &p_lhs, const float &p_rhs) {
        return (p_lhs == p_rhs) || (Math::is_nan(p_lhs) && Math::is_nan(p_rhs));
    }

    bool compare(const double &p_lhs, const double &p_rhs) {
        return (p_lhs == p_rhs) || (Math::is_nan(p_lhs) && Math::is_nan(p_rhs));
    }
};

#endif //NEXUS_COMPARATOR_H
