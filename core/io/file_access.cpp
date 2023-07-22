//
// Created by cycastic on 7/20/2023.
//

#include "file_access.h"

uint16_t FileAccess::get_16() const {
    uint16_t res;
    uint8_t a, b;

    a = get_8();
    b = get_8();

    if (endian_swap) {
        SWAP(a, b);
    }

    res = b;
    res <<= 8;
    res |= a;

    return res;
}

uint32_t FileAccess::get_32() const {
    uint32_t res;
    uint16_t a, b;

    a = get_16();
    b = get_16();

    if (endian_swap) {
        SWAP(a, b);
    }

    res = b;
    res <<= 16;
    res |= a;

    return res;
}

uint64_t FileAccess::get_64() const {
    uint64_t res;
    uint32_t a, b;

    a = get_32();
    b = get_32();

    if (endian_swap) {
        SWAP(a, b);
    }

    res = b;
    res <<= 32;
    res |= a;

    return res;
}

float FileAccess::get_float() const {
    auto raw = get_32();
    return *((float*)&raw);
}

double FileAccess::get_double() const {
    auto raw = get_64();
    return *((double*)&raw);
}

CharString FileAccess::get_char_string() const {
    Vector<uint8_t> buffer{};

    while (!eof_reached()){
        auto c = get_8();
        if (c == 0) break;
        buffer.push_back(c);
    }
    buffer.push_back(0);

    return {(const char*)buffer.ptr()};
}

VString FileAccess::get_string_text(bool p_skip_cr) const {
    CowArray<uint8_t> buffer{};
    auto size = get_file_size() - get_pos();
    buffer.resize(size + 1);
    buffer.ptrw()[size] = 0;
    get_buffer(buffer.ptrw(), size);

    return VString::from_utf8((const char*)buffer.ptrw(), -1, p_skip_cr);
}

VString FileAccess::get_string() const {
    static constexpr auto term = L'\0';

    auto size = get_32();
    CowArray<uint8_t> buffer{};
    buffer.resize(size + sizeof(term));
    get_buffer(buffer.ptrw(), size);
    (wchar_t &)(buffer.ptrw()[size]) = term;

    return VString::from_utf8((const char*)buffer.ptr());
}

VString FileAccess::get_line() const {
    CharString buffer{};
    auto c = get_8();
    while (!eof_reached()){
        if (c == '\n' || c == '\0'){
            c += 0;
            return VString::from_utf8(buffer.c_str());
        } else if (c != '\r'){
            buffer += (char)c;
        }
        c = get_8();
    }
    c += 0;
    return VString::from_utf8(buffer.c_str());
}

uint64_t FileAccess::get_buffer(uint8_t *p_buffer, const size_t &p_bytes_count) const {
    if (!p_buffer && p_bytes_count > 0) return -1;
    uint64_t i = 0;
    for (i = 0; i < p_bytes_count && !eof_reached(); i++) {
        p_buffer[i] = get_8();
    }

    return i;
}

void FileAccess::store_16(const uint16_t &p_data) {
    uint8_t a, b;

    a = p_data & 0xFF;
    b = p_data >> 8;

    if (endian_swap) {
        SWAP(a, b);
    }

    store_8(a);
    store_8(b);
}

void FileAccess::store_32(const uint32_t &p_data) {
    uint16_t a, b;

    a = p_data & 0xFFFF;
    b = p_data >> 16;

    if (endian_swap) {
        SWAP(a, b);
    }

    store_16(a);
    store_16(b);
}

void FileAccess::store_64(const uint64_t &p_data) {
    uint32_t a, b;

    a = p_data & 0xFFFFFFFF;
    b = p_data >> 32;

    if (endian_swap) {
        SWAP(a, b);
    }

    store_32(a);
    store_32(b);
}

void FileAccess::store_float(const float &p_data) {
    auto casted = *(const uint32_t*)(&p_data);
    store_32(casted);
}

void FileAccess::store_double(const double &p_data) {
    auto casted = *(const uint64_t*)(&p_data);
    store_64(casted);
}

void FileAccess::store_char_string(const CharString& p_from){
    for (auto i = 0; i < p_from.length(); i++){
        store_8(p_from[i]);
    }
    store_8(0);
}

void FileAccess::store_string_text(const VString& p_from){
    store_char_string(p_from.utf8());
}

void FileAccess::store_string(const VString& p_from){
    auto utf8 = p_from.utf8();
    store_32(utf8.length());
    // Does not store null terminator
    for (auto i = 0; i < utf8.length(); i++){
        store_8(utf8[i]);
    }
}

void FileAccess::store_buffer(const uint8_t *p_buffer, const size_t &p_bytes_count) {
    if (!p_buffer && p_bytes_count > 0) throw InvalidBufferException("p_buffer is null");
    for (size_t i = 0; i < p_bytes_count; i++) {
        store_8(p_buffer[i]);
    }
}


