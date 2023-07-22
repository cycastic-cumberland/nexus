//
// Created by cycastic on 7/19/2023.
//

#include "hashfuncs.h"
#include "types/vstring.h"

uint32_t StandardHasher::hash(const VString &p_data)  { return djb2_string_hash(p_data.ptr()); }
