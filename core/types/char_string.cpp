//
// Created by cycastic on 7/20/2023.
//

#include "char_string.h"

#include "../hashfuncs.h"

uint32_t CharString::hash_u32() const {
    return StandardHasher::hash(c_str());
}

bool CharString::operator==(const CharString &p_other) const {
    if (length() != p_other.length()) return false;
    if (hash_u32() != p_other.hash_u32()) return false;
    for (auto i = 0; i < length(); i++){
        if (ptr()[i] != p_other[i]) return false;
    }
    return true;
}

void CharString::copy_from(const char *p_source) {
    auto len = string_length(p_source);
    if (len == 0) {
        resize(1);
        ptrw()[0] = 0;
        return;
    }
    resize(len + 1);
    memcpy(data.ptrw(), p_source, len * sizeof(char));
    ptrw()[len] = 0;
}

void CharString::append(const char *p_data) {
    auto old_length = length();
    auto data_length = string_length(p_data);
    auto new_length = data_length + old_length;
    if (old_length == new_length) return;
    resize(new_length + 1);
    memcpy(data.ptrw() + (old_length * sizeof(char)), p_data, data_length * sizeof(char));
    ptrw()[new_length] = 0;
}

std::ostream &operator<<(std::ostream &p_ostream, const CharString &p_char_string) {
    for (auto i = 0; i < p_char_string.length(); i++){
        p_ostream << p_char_string.ptr()[i];
    }
    return p_ostream;
}
