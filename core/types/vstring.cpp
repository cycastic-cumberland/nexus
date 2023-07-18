//
// Created by cycastic on 7/17/2023.
//
#include "vstring.h"
#include "../hashfuncs.h"

uint32_t VString::hash_u32() const {
    return StandardHasher::hash(*this);
}

bool VString::operator==(const VString& other) const {
    // Compare capacity
    if (capacity() != other.capacity()) return false;
    // Compare hashes
    if (hash_u32() != other.hash_u32()) return false;
    // If all else fail
    for (size_t i = 0, s = capacity(); i < s; i++){
        if (ptr()[i] != other[i]) return false;
    }
    return true;
}

// Shamelessly stolen from Godot's String
bool VString::parse_utf8(const char *p_utf8, int p_len, const bool& p_skip_cr) {
//#define _UNICERROR(m_err) print_line("Unicode error: " + String(m_err));
    if (!p_utf8) {
        return true;
    }

    VString aux;

    int cstr_size = 0;
    int str_size = 0;

    /* HANDLE BOM (Byte Order Mark) */
    if (p_len < 0 || p_len >= 3) {
        bool has_bom = uint8_t(p_utf8[0]) == 0xEF && uint8_t(p_utf8[1]) == 0xBB && uint8_t(p_utf8[2]) == 0xBF;
        if (has_bom) {
            //just skip it
            if (p_len >= 0) {
                p_len -= 3;
            }
            p_utf8 += 3;
        }
    }
    {
        const char *ptrtmp = p_utf8;
        const char *ptrtmp_limit = &p_utf8[p_len];
        int skip = 0;
        while (ptrtmp != ptrtmp_limit && *ptrtmp) {
            if (skip == 0) {
                uint8_t c = *ptrtmp >= 0 ? *ptrtmp : uint8_t(256 + *ptrtmp);

                if (p_skip_cr && c == '\r') {
                    ptrtmp++;
                    continue;
                }

                /* Determine the number of characters in sequence */
                if ((c & 0x80) == 0) {
                    skip = 0;
                } else if ((c & 0xE0) == 0xC0) {
                    skip = 1;
                } else if ((c & 0xF0) == 0xE0) {
                    skip = 2;
                } else if ((c & 0xF8) == 0xF0) {
                    skip = 3;
                    if (sizeof(wchar_t) == 2) {
                        str_size++; // encode as surrogate pair.
                    }
                } else if ((c & 0xFC) == 0xF8) {
                    skip = 4;
                    // invalid character, too long to encode as surrogates.
                } else if ((c & 0xFE) == 0xFC) {
                    skip = 5;
                    // invalid character, too long to encode as surrogates.
                } else {
                    // TODO: Implement error
//                    _UNICERROR("invalid skip");
                    return true; //invalid utf8
                }

                if (skip == 1 && (c & 0x1E) == 0) {
                    //printf("overlong rejected\n");
                    // TODO: Implement error
//                    _UNICERROR("overlong rejected");
                    return true; //reject overlong
                }

                str_size++;

            } else {
                --skip;
            }

            cstr_size++;
            ptrtmp++;
        }

        if (skip) {
            // TODO: Implement error
//            _UNICERROR("no space left");
            return true; //not enough spac
        }
    }

    if (str_size == 0) {
        *this = "";
        return false;
    }

    resize(str_size + 1);
    wchar_t *dst = ptrw();
    dst[str_size] = 0;

    while (cstr_size) {
        int len = 0;

        if (p_skip_cr && *p_utf8 == '\r') {
            p_utf8++;
            continue;
        }

        /* Determine the number of characters in sequence */
        if ((*p_utf8 & 0x80) == 0) {
            len = 1;
        } else if ((*p_utf8 & 0xE0) == 0xC0) {
            len = 2;
        } else if ((*p_utf8 & 0xF0) == 0xE0) {
            len = 3;
        } else if ((*p_utf8 & 0xF8) == 0xF0) {
            len = 4;
        } else if ((*p_utf8 & 0xFC) == 0xF8) {
            len = 5;
        } else if ((*p_utf8 & 0xFE) == 0xFC) {
            len = 6;
        } else {
            // TODO: Implement error
//            _UNICERROR("invalid len");

            return true; //invalid UTF8
        }

        if (len > cstr_size) {
            // TODO: Implement error
//            _UNICERROR("no space left");
            return true; //not enough space
        }

        if (len == 2 && (*p_utf8 & 0x1E) == 0) {
            //printf("overlong rejected\n");
            // TODO: Implement error
//            _UNICERROR("no space left");
            return true; //reject overlong
        }

        /* Convert the first character */

        uint32_t unichar = 0;

        if (len == 1) {
            unichar = *p_utf8;
        } else {
            unichar = (0xFF >> (len + 1)) & *p_utf8;

            for (int i = 1; i < len; i++) {
                if ((p_utf8[i] & 0xC0) != 0x80) {
                    // TODO: Implement error
//                    _UNICERROR("invalid utf8");
                    return true; //invalid utf8
                }
                if (unichar == 0 && i == 2 && ((p_utf8[i] & 0x7F) >> (7 - len)) == 0) {
                    // TODO: Implement error
//                    _UNICERROR("invalid utf8 overlong");
                    return true; //no overlong
                }
                unichar = (unichar << 6) | (p_utf8[i] & 0x3F);
            }
        }

        if (sizeof(wchar_t) == 2 && unichar > 0x10FFFF) {
            unichar = ' '; // invalid character, too long to encode as surrogates.
        } else if (sizeof(wchar_t) == 2 && unichar > 0xFFFF) {
            *(dst++) = uint32_t((unichar >> 10) + 0xD7C0); // lead surrogate.
            *(dst++) = uint32_t((unichar & 0x3FF) | 0xDC00); // trail surrogate.
        } else {
            *(dst++) = unichar;
        }
        cstr_size -= len;
        p_utf8 += len;
    }

    return false;
}

// Shamelessly stolen from Godot's String
void VString::copy_from(const wchar_t *p_source, const int& p_clip_to) {
    if (!p_source) {
        resize(0);
        return;
    }

    int len = 0;
    const auto *ptr = p_source;
    while ((p_clip_to < 0 || len < p_clip_to) && *(ptr++) != 0) {
        len++;
    }

    if (len == 0) {
        resize(0);
        return;
    }

    copy_from_unchecked(p_source, len);
}

// Shamelessly stolen from Godot's String
void VString::copy_from(const char *p_source) {
    if (!p_source) {
        resize(0);
        return;
    }

    const size_t len = strlen(p_source);

    if (len == 0) {
        resize(0);
        return;
    }

    resize(len + 1); // include 0

    wchar_t *dst = this->ptrw();

    for (size_t i = 0; i <= len; i++) {
        dst[i] = p_source[i];
    }
}

void VString::copy_from_unchecked(const wchar_t *p_char, const int &p_length) {
    resize(p_length + 1);

    wchar_t *dst = ptrw();
    memcpy(dst, p_char, p_length * sizeof(wchar_t));
    dst[p_length] = 0;
}

std::wostream &operator<<(std::wostream &p_ostream, const VString &p_vstring) {
    p_ostream << p_vstring.ptr();
    return p_ostream;
}

void VString::utf8(char **p_out_str) const {
    auto l = length();
    if (!l) {
        return;
    }

    const wchar_t *d = &operator[](0);
    int fl = 0;
    for (int i = 0; i < l; i++) {
        uint32_t c = d[i];
        if ((c & 0xfffffc00) == 0xd800) { // decode surrogate pair.
            if ((i < l - 1) && (d[i + 1] & 0xfffffc00) == 0xdc00) {
                c = (c << 10UL) + d[i + 1] - ((0xd800 << 10UL) + 0xdc00 - 0x10000);
                i++; // skip trail surrogate.
            } else {
                fl += 1;
                continue;
            }
        } else if ((c & 0xfffffc00) == 0xdc00) {
            fl += 1;
            continue;
        }
        if (c <= 0x7f) { // 7 bits.
            fl += 1;
        } else if (c <= 0x7ff) { // 11 bits
            fl += 2;
        } else if (c <= 0xffff) { // 16 bits
            fl += 3;
        } else if (c <= 0x001fffff) { // 21 bits
            fl += 4;
        } else if (c <= 0x03ffffff) { // 26 bits
            fl += 5;
        } else if (c <= 0x7fffffff) { // 31 bits
            fl += 6;
        }
    }
    if (fl == 0) {
        return;
    }
    *p_out_str = (char*)malloc(sizeof(char) * (fl + 1));
    uint8_t * cdst = (uint8_t*)(*p_out_str);
#define APPEND_CHAR(m_c) *(cdst++) = m_c
    for (int i = 0; i < l; i++) {
        uint32_t c = d[i];
        if ((c & 0xfffffc00) == 0xd800) { // decode surrogate pair.
            if ((i < l - 1) && (d[i + 1] & 0xfffffc00) == 0xdc00) {
                c = (c << 10UL) + d[i + 1] - ((0xd800 << 10UL) + 0xdc00 - 0x10000);
                i++; // skip trail surrogate.
            } else {
                APPEND_CHAR(' ');
                continue;
            }
        } else if ((c & 0xfffffc00) == 0xdc00) {
            APPEND_CHAR(' ');
            continue;
        }

        if (c <= 0x7f) { // 7 bits.
            APPEND_CHAR(c);
        } else if (c <= 0x7ff) { // 11 bits

            APPEND_CHAR(uint32_t(0xc0 | ((c >> 6) & 0x1f))); // Top 5 bits.
            APPEND_CHAR(uint32_t(0x80 | (c & 0x3f))); // Bottom 6 bits.
        } else if (c <= 0xffff) { // 16 bits

            APPEND_CHAR(uint32_t(0xe0 | ((c >> 12) & 0x0f))); // Top 4 bits.
            APPEND_CHAR(uint32_t(0x80 | ((c >> 6) & 0x3f))); // Middle 6 bits.
            APPEND_CHAR(uint32_t(0x80 | (c & 0x3f))); // Bottom 6 bits.
        } else if (c <= 0x001fffff) { // 21 bits

            APPEND_CHAR(uint32_t(0xf0 | ((c >> 18) & 0x07))); // Top 3 bits.
            APPEND_CHAR(uint32_t(0x80 | ((c >> 12) & 0x3f))); // Upper middle 6 bits.
            APPEND_CHAR(uint32_t(0x80 | ((c >> 6) & 0x3f))); // Lower middle 6 bits.
            APPEND_CHAR(uint32_t(0x80 | (c & 0x3f))); // Bottom 6 bits.
        } else if (c <= 0x03ffffff) { // 26 bits

            APPEND_CHAR(uint32_t(0xf8 | ((c >> 24) & 0x03))); // Top 2 bits.
            APPEND_CHAR(uint32_t(0x80 | ((c >> 18) & 0x3f))); // Upper middle 6 bits.
            APPEND_CHAR(uint32_t(0x80 | ((c >> 12) & 0x3f))); // middle 6 bits.
            APPEND_CHAR(uint32_t(0x80 | ((c >> 6) & 0x3f))); // Lower middle 6 bits.
            APPEND_CHAR(uint32_t(0x80 | (c & 0x3f))); // Bottom 6 bits.
        } else if (c <= 0x7fffffff) { // 31 bits

            APPEND_CHAR(uint32_t(0xfc | ((c >> 30) & 0x01))); // Top 1 bit.
            APPEND_CHAR(uint32_t(0x80 | ((c >> 24) & 0x3f))); // Upper upper middle 6 bits.
            APPEND_CHAR(uint32_t(0x80 | ((c >> 18) & 0x3f))); // Lower upper middle 6 bits.
            APPEND_CHAR(uint32_t(0x80 | ((c >> 12) & 0x3f))); // Upper lower middle 6 bits.
            APPEND_CHAR(uint32_t(0x80 | ((c >> 6) & 0x3f))); // Lower lower middle 6 bits.
            APPEND_CHAR(uint32_t(0x80 | (c & 0x3f))); // Bottom 6 bits.
        }
    }
#undef APPEND_CHAR
    *cdst = 0; //trailing zero
}

std::ostream &operator<<(std::ostream &p_ostream, const VString &p_vstring) {
    char* as_char_array;
    p_vstring.utf8(&as_char_array);
    p_ostream << as_char_array;
    free(as_char_array);
    return p_ostream;
}

void VString::operator+=(const VString &p_other) {
    auto old_size = capacity();
    data.arr_copy(p_other.ptr(), p_other.capacity(), old_size == 0 ? 0 : capacity() - 1);
}

void VString::operator+=(const char &p_other) {
    *this += VString(p_other);
}

void VString::operator+=(const wchar_t &p_other) {
    *this += VString(p_other);
}
