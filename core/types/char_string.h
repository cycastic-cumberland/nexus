//
// Created by cycastic on 7/20/2023.
//

#ifndef NEXUS_CHAR_STRING_H
#define NEXUS_CHAR_STRING_H

#include <iostream>
#include "cow.h"

class VString;

class CharString {
    CowArray<char> data;
    _FORCE_INLINE_ void resize(const size_t& new_size){
        data.resize(new_size);
    }
    void append(const char* p_data);

    friend class VString;
public:
    _NO_DISCARD_ _FORCE_INLINE_ const char* ptr() const { return data.ptr(); }
    _FORCE_INLINE_ char* ptrw() { return data.ptrw(); }
    _FORCE_INLINE_ const char& operator[](const size_t& idx) const { return ptr()[idx]; }
    _FORCE_INLINE_ char& operator[](const size_t& idx) { return ptrw()[idx]; }
    _NO_DISCARD_ _FORCE_INLINE_ size_t capacity() const { return data.capacity(); }
    _NO_DISCARD_ _FORCE_INLINE_ size_t length() const { return capacity() == 0 ? 0 : capacity() - 1; }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return length() == 0; }
    _NO_DISCARD_ _FORCE_INLINE_ const char* c_str() const { return ptr(); }

    _NO_DISCARD_ uint32_t hash_u32() const;
    _NO_DISCARD_ bool operator==(const CharString& p_other) const;
    void copy_from(const char *p_source);

    _FORCE_INLINE_ CharString() : data() { }
    _FORCE_INLINE_ CharString(const char* p_from) : CharString() { copy_from(p_from); }
    _FORCE_INLINE_ CharString(const CharString& p_from) : CharString() { copy_from(p_from.c_str()); }
    _FORCE_INLINE_ CharString& operator=(const char* p_from){
        copy_from(p_from);
        return *this;
    }
    _FORCE_INLINE_ CharString& operator=(const CharString& p_from){
        copy_from(p_from.c_str());
        return *this;
    }
    _FORCE_INLINE_ void operator+=(const char* p_from){
        append(p_from);
    }
    _FORCE_INLINE_ void operator+=(const CharString& p_from){
        append(p_from.c_str());
    }
    _FORCE_INLINE_ void operator+=(const char& p_from){
        resize(capacity() + 1);
        ptrw()[length()] = p_from;
    }
    _FORCE_INLINE_ _NO_DISCARD_ CharString operator+(const char* p_from) const {
        CharString re = *this;
        re += p_from;
        return re;
    }
    _FORCE_INLINE_ _NO_DISCARD_ CharString operator+(const CharString& p_from) const {
        CharString re = *this;
        re += p_from;
        return re;
    }
    _FORCE_INLINE_ _NO_DISCARD_ CharString operator+(const char& p_from) const {
        CharString re = *this;
        re += p_from;
        return re;
    }
    friend std::ostream& operator<<(std::ostream& p_ostream, const CharString& p_char_string);
};

#endif //NEXUS_CHAR_STRING_H
