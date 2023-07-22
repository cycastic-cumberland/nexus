//
// Created by cycastic on 7/17/2023.
//
#include "vstring.h"
#include "../hashfuncs.h"

VString itos(int64_t p_val) {
    return VString::num_int64(p_val);
}

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
                    throw VStringException("Invalid skip");
                }

                if (skip == 1 && (c & 0x1E) == 0) {
                    throw VStringException("Overlong rejected");
                }

                str_size++;

            } else {
                --skip;
            }

            cstr_size++;
            ptrtmp++;
        }

        if (skip) {
            throw VStringException("No space left");
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
            throw VStringException("Invalid len");

            return true; //invalid UTF8
        }

        if (len > cstr_size) {
            throw VStringException("No space left");
        }

        if (len == 2 && (*p_utf8 & 0x1E) == 0) {
            throw VStringException("No space left");
        }

        /* Convert the first_ptr character */

        uint32_t unichar = 0;

        if (len == 1) {
            unichar = *p_utf8;
        } else {
            unichar = (0xFF >> (len + 1)) & *p_utf8;

            for (int i = 1; i < len; i++) {
                if ((p_utf8[i] & 0xC0) != 0x80) {
                    throw VStringException("Invalid utf8");
                }
                if (unichar == 0 && i == 2 && ((p_utf8[i] & 0x7F) >> (7 - len)) == 0) {
                    throw VStringException("Invalid utf8 overlong");
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

    const size_t len = string_length(p_source);

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
    if (p_vstring.ptr() == nullptr) return p_ostream;
    p_ostream << p_vstring.ptr();
    return p_ostream;
}

CharString VString::utf8() const {
    auto l = length();
    if (!l) {
        return {};
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

    CharString utf8s;
    if (fl == 0) {
        return utf8s;
    }

    utf8s.resize(fl + 1);
    uint8_t *cdst = (uint8_t *)utf8s.c_str();

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

    return utf8s;
}

std::ostream &operator<<(std::ostream &p_ostream, const VString &p_vstring) {
    p_ostream << p_vstring.utf8();
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

bool VString::is_network_share_path() const {
    return begins_with("//") || begins_with("\\\\");
}

VString VString::get_base_dir() const {
    int end = 0;

    // URL scheme style base.
    auto basepos = find("://");
    if (basepos != -1) {
        end = basepos + 3;
    }

    // Windows top level directory base.
    if (end == 0) {
        basepos = find(":/");
        if (basepos == -1) {
            basepos = find(":\\");
        }
        if (basepos != -1) {
            end = basepos + 2;
        }
    }

    // Windows UNC network share path.
    if (end == 0) {
        if (is_network_share_path()) {
            basepos = find("/", 2);
            if (basepos == -1) {
                basepos = find("\\", 2);
            }
            auto servpos = find("/", basepos + 1);
            if (servpos == -1) {
                servpos = find("\\", basepos + 1);
            }
            if (servpos != -1) {
                end = servpos + 1;
            }
        }
    }

    // Unix root directory base.
    if (end == 0) {
        if (begins_with("/")) {
            end = 1;
        }
    }

    VString rs;
    VString base;
    if (end != 0) {
        rs = substr(end, length());
        base = substr(0, end);
    } else {
        rs = *this;
    }

    int sep = MAX(rs.rfind("/"), rs.rfind("\\"));
    if (sep == -1) {
        return base;
    }

    return base + rs.substr(0, sep);
}

int64_t VString::find(const char *p_str, int64_t p_from) const {
    if (p_from < 0) {
        return -1;
    }

    const size_t len = length();

    if (len == 0) {
        return -1; // won't find anything!
    }

    const wchar_t *src = c_str();

    int src_len = 0;
    while (p_str[src_len] != '\0') {
        src_len++;
    }

    if (src_len == 1) {
        const char needle = p_str[0];

        for (int64_t i = p_from; i < len; i++) {
            if (src[i] == needle) {
                return i;
            }
        }

    } else {
        for (int64_t i = p_from; i <= (len - src_len); i++) {
            bool found = true;
            for (int j = 0; j < src_len; j++) {
                int read_pos = i + j;

                if (read_pos >= len) {
                    throw VStringException("read_pos >= len");
                    return -1;
                };

                if (src[read_pos] != p_str[j]) {
                    found = false;
                    break;
                }
            }

            if (found) {
                return i;
            }
        }
    }

    return -1;
}
bool VString::begins_with(const VString &p_string) const {
    auto l = p_string.length();
    if (l > length()) {
        return false;
    }

    if (l == 0) {
        return true;
    }

    const auto *p = &p_string[0];
    const auto *s = &operator[](0);

    for (int i = 0; i < l; i++) {
        if (p[i] != s[i]) {
            return false;
        }
    }

    return true;
}

VString VString::substr(int64_t p_from, int64_t p_chars) const {
    if (p_chars == -1) {
        p_chars = (int64_t)length() - p_from;
    }

    if (empty() || p_from < 0 || p_from >= length() || p_chars <= 0) {
        return "";
    }

    if ((p_from + p_chars) > length()) {
        p_chars = (int64_t)length() - p_from;
    }

    if (p_from == 0 && p_chars >= length()) {
        return {*this};
    }

    VString s{};
    s.copy_from_unchecked(&c_str()[p_from], p_chars);
    return s;
}

int64_t VString::rfind(const VString& p_str, int64_t p_from) const {
    // establish a limit
    auto limit = int64_t(length() - p_str.length());
    if (limit < 0) {
        return -1;
    }

    // establish a starting point
    if (p_from < 0) {
        p_from = limit;
    } else if (p_from > limit) {
        p_from = limit;
    }

    auto src_len = p_str.length();
    auto len = length();

    if (src_len == 0 || len == 0) {
        return -1; // won't find anything!
    }

    const auto *src = c_str();

    for (int64_t i = p_from; i >= 0; i--) {
        bool found = true;
        for (int j = 0; j < src_len; j++) {
            int read_pos = i + j;

            if (read_pos >= len) {
                throw VStringException("read_pos >= len");
            };

            if (src[read_pos] != p_str[j]) {
                found = false;
                break;
            }
        }

        if (found) {
            return i;
        }
    }

    return -1;
}

VString VString::replace(const char *p_key, const char *p_with) const{
    VString new_string;
    int64_t search_from = 0;
    int64_t result = 0;

    while ((result = find(p_key, search_from)) >= 0) {
        new_string += substr(search_from, result - search_from);
        new_string += p_with;
        int k = 0;
        while (p_key[k] != '\0') {
            k++;
        }
        search_from = result + k;
    }

    if (search_from == 0) {
        return *this;
    }

    new_string += substr(search_from, int64_t(length()) - search_from);

    return new_string;
}

VString VString::num_int64(int64_t p_num, int base, bool capitalize_hex) {
    bool sign = p_num < 0;

    int64_t n = p_num;

    int chars = 0;
    do {
        n /= base;
        chars++;
    } while (n);

    if (sign) {
        chars++;
    }
    VString s;
    s.resize(chars + 1);
    auto *c = s.ptrw();
    c[chars] = 0;
    n = p_num;
    do {
        int mod = ABS(n % base);
        if (mod >= 10) {
            char a = (capitalize_hex ? 'A' : 'a');
            c[--chars] = a + (mod - 10);
        } else {
            c[--chars] = '0' + mod;
        }

        n /= base;
    } while (n);

    if (sign) {
        c[0] = '-';
    }

    return s;
}
