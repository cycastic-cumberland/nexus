//
// Created by cycastic on 7/17/2023.
//

#ifndef NEXUS_VSTRING_H
#define NEXUS_VSTRING_H

#include <iostream>
#include "../exception.h"

class VStringException : public Exception {
public:
    explicit VStringException(const char* p_err = nullptr) : Exception(p_err) {}
};

class VString {
    CowArray<wchar_t> data{};

    void copy_from_unchecked(const wchar_t *p_char, const int& p_length);
    _FORCE_INLINE_ void resize(const size_t& new_size) {
        data.resize(new_size);
    }
public:
    _NO_DISCARD_ _FORCE_INLINE_ const wchar_t* ptr() const { return data.ptr(); }
    _FORCE_INLINE_ wchar_t* ptrw() { return data.ptrw(); }
    _NO_DISCARD_ _FORCE_INLINE_ const wchar_t* c_str() const { return ptr(); }
    _FORCE_INLINE_ const wchar_t& operator[](const size_t& idx) const { return ptr()[idx]; }
    _FORCE_INLINE_ wchar_t& operator[](const size_t& idx) { return ptrw()[idx]; }
    _NO_DISCARD_ _FORCE_INLINE_ size_t capacity() const { return data.capacity(); }
    _NO_DISCARD_ _FORCE_INLINE_ size_t length() const { return capacity() == 0 ? 0 : capacity() - 1; }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return length() == 0; }
//    _FORCE_INLINE_ void clear() { data.clear(); }

    _NO_DISCARD_ uint32_t hash_u32() const;
    _NO_DISCARD_ bool operator==(const VString& other) const;
    _NO_DISCARD_ _FORCE_INLINE_ bool operator==(const char* other) const { return *this == VString(other); }
    _NO_DISCARD_ _FORCE_INLINE_ bool operator!=(const VString& other) const { return !(*this == other); }
    _NO_DISCARD_ _FORCE_INLINE_ bool operator!=(const char* other) const { return !(*this == VString(other)); }
    void copy_from(const wchar_t *p_source, const int& p_clip_to);
    void copy_from(const char *p_source);
    bool parse_utf8(const char *p_utf8, int p_len = -1, const bool& p_skip_cr = false);
    _NO_DISCARD_ bool begins_with(const VString &p_string) const;
    _NO_DISCARD_ int64_t find(const char *p_str, int64_t p_from = 0) const;
    _NO_DISCARD_ int64_t rfind(const VString& p_str, int64_t p_from = -1) const;
    _NO_DISCARD_ VString substr(int64_t p_from, int64_t p_chars) const;
    _NO_DISCARD_ bool is_network_share_path() const;
    _NO_DISCARD_ VString replace(const char *p_key, const char *p_with) const;
    _NO_DISCARD_ VString get_base_dir() const;
    CharString utf8() const;
    static _FORCE_INLINE_ VString from_utf8(const char *p_utf8, int p_len = -1, bool p_skip_cr = false){
        VString re{};
        re.parse_utf8(p_utf8, p_len, p_skip_cr);
        return re;
    }
    _NO_DISCARD_ static VString num_int64(int64_t p_num, int base = 10, bool capitalize_hex = false);

    _FORCE_INLINE_ VString() = default;
    _FORCE_INLINE_ VString(const VString& p_other) { data = p_other.data; }
    _FORCE_INLINE_ VString(const char* p_str) : VString() { copy_from(p_str); }
    _FORCE_INLINE_ VString(const wchar_t* p_str, const int& p_clip_to = -1) : VString() { copy_from(p_str, p_clip_to); }
    _FORCE_INLINE_ VString(const wchar_t& p_str) : VString() {
        data.resize(2);
        new (&data.ptrw()[0]) wchar_t(p_str);
        data.ptrw()[1] = L'\0';
    }
    _FORCE_INLINE_ VString(const char& p_str) : VString((wchar_t )p_str) {}
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
    void operator+=(const wchar_t& p_other);
    void operator+=(const char& p_other);
    _FORCE_INLINE_ void operator+=(const char* p_str){
        *this += VString(p_str);
    }
    _FORCE_INLINE_ void operator+=(const wchar_t* p_str){
        *this += VString(p_str);
    }

    friend std::wostream& operator<<(std::wostream& p_ostream, const VString& p_vstring);
    friend std::ostream& operator<<(std::ostream& p_ostream, const VString& p_vstring);
};

VString itos(int64_t p_val);

#endif //NEXUS_VSTRING_H
