//
// Created by cycastic on 7/17/2023.
//

#ifndef NEXUS_VSTRING_H
#define NEXUS_VSTRING_H

#include <iostream>
#include "cow.h"

class VString {
    CowArray<wchar_t> data;

    void copy_from_unchecked(const wchar_t *p_char, const int& p_length);
    _FORCE_INLINE_ void resize(const size_t& new_size) {
        data.resize(new_size);
    }
public:
    _NO_DISCARD_ _FORCE_INLINE_ const wchar_t* ptr() const { return data.ptr(); }
    _FORCE_INLINE_ wchar_t* ptrw() { return data.ptrw(); }
    _FORCE_INLINE_ const wchar_t& operator[](const size_t& idx) const { return ptr()[idx]; }
    _FORCE_INLINE_ wchar_t& operator[](const size_t& idx) { return ptrw()[idx]; }
    _NO_DISCARD_ _FORCE_INLINE_ size_t capacity() const { return data.capacity(); }
    _NO_DISCARD_ _FORCE_INLINE_ size_t length() const { return capacity() == 0 ? 0 : capacity() - 1; }
    _FORCE_INLINE_ void clear() { data.clear(); }

    _NO_DISCARD_ uint32_t hash_u32() const;
    _NO_DISCARD_ bool operator==(const VString& other) const;
    void copy_from(const wchar_t *p_source, const int& p_clip_to);
    void copy_from(const char *p_source);
    bool parse_utf8(const char *p_utf8, int p_len = -1, const bool& p_skip_cr = false);
    void utf8(char** p_out_str) const;
    static _FORCE_INLINE_ VString from_utf8(const char *p_utf8, int p_len = -1, bool p_skip_cr = false){
        VString re{};
        re.parse_utf8(p_utf8, p_len, p_skip_cr);
        return re;
    }

    _FORCE_INLINE_ VString() { data = CowArray<wchar_t>(); }
    _FORCE_INLINE_ VString(const VString& p_other) { data = p_other.data; }
    _FORCE_INLINE_ VString(const char* p_str) : VString() { copy_from(p_str); }
    _FORCE_INLINE_ VString(const wchar_t* p_str, const int& p_clip_to = -1) : VString() { copy_from(p_str, p_clip_to); }
    _NO_DISCARD_ _FORCE_INLINE_ VString duplicate() const { return *this; }
    _FORCE_INLINE_ VString& operator=(const VString& p_other) {
        data = p_other.data;
        return *this;
    }
    _FORCE_INLINE_ VString operator+(const VString& p_other) const {
        VString re = duplicate();
        re += p_other;
        return re;
    }
    _FORCE_INLINE_ VString operator+(const char* p_str) const {
        return *this + VString(p_str);
    }
    _FORCE_INLINE_ VString operator+(const wchar_t* p_str) const {
        return *this + VString(p_str);
    }
    void operator+=(const VString& p_other);
    _FORCE_INLINE_ void operator+=(const char* p_str){
        *this += VString(p_str);
    }
    _FORCE_INLINE_ void operator+=(const wchar_t* p_str){
        *this += VString(p_str);
    }

    friend std::wostream& operator<<(std::wostream& p_ostream, const VString& p_vstring);
    friend std::ostream& operator<<(std::ostream& p_ostream, const VString& p_vstring);
};

#endif //NEXUS_VSTRING_H
