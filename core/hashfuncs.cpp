//
// Created by cycastic on 7/19/2023.
//

#include "hashfuncs.h"
#include "types/vstring.h"
#include "types/interned_string.h"


uint32_t StandardHasher::hash(const wchar_t *p_data) {
    return djb2_string_hash(p_data);
}

uint32_t StandardHasher::hash(const VString &p_data)  {
    return djb2_string_hash(p_data.ptr());
}

uint32_t StandardHasher::hash(const InternedString &p_data) {
    return hash(p_data.hash());
}
