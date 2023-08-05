//
// Literally stolen from Godot's StringName
//

#ifndef NEXUS_INTERNED_STRING_H
#define NEXUS_INTERNED_STRING_H

#include "../lock.h"
#include "../exception.h"
#include "safe_refcount.h"
#include "vstring.h"

class StringConstantException : public Exception {
public:
    explicit StringConstantException(const char *p_msg) : Exception(p_msg) {}
};

// A string class that have constant comparison time, "borrowed" from Godot
// Featuring: slow initialization/write, but constant time comparison
class InternedString {
    enum {
        STRING_TABLE_BITS = 12,
        STRING_TABLE_LEN = 1 << STRING_TABLE_BITS,
        STRING_TABLE_MASK = STRING_TABLE_LEN - 1
    };
    struct StringCell {
        StringCell* prev;
        StringCell* next;
        uint32_t hash;
        SafeRefCount refcount;

        VString data;

    };
    static BinaryMutex lock;
    static InternedString::StringCell* table[STRING_TABLE_LEN];
    static bool configured;

    StringCell* occupying{};
    void ref(StringCell* cell);
    void unref();
public:
    _NO_DISCARD_ _FORCE_INLINE_ const wchar_t* c_str() const { return occupying ? occupying->data.c_str() : L""; }
    _NO_DISCARD_ _FORCE_INLINE_ size_t length() const { return occupying ? occupying->data.length() : 0; }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return length() == 0; }
    _NO_DISCARD_ _FORCE_INLINE_ bool operator==(const InternedString& p_other) const { return occupying == p_other.occupying; }
    _NO_DISCARD_ _FORCE_INLINE_ bool operator!=(const InternedString& p_other) const { return !(*this == p_other); }
    _NO_DISCARD_ _FORCE_INLINE_ uint32_t hash() const { return occupying ? occupying->hash : 0; }

    InternedString& operator=(const InternedString& p_other);
    InternedString& operator=(const char* p_from);
    InternedString& operator=(const wchar_t* p_from);
    InternedString& operator=(const VString& p_from);

    InternedString();
    InternedString(const InternedString& p_other);
    InternedString(const char* p_from);
    InternedString(const wchar_t* p_from);
    InternedString(const VString& p_from);
    ~InternedString();

    void operator+=(const InternedString& p_other);
    void operator+=(const VString& p_other);
    _FORCE_INLINE_ InternedString operator+(const InternedString& p_other) const {
        InternedString re{*this};
        re += p_other;
        return re;
    }
    _FORCE_INLINE_ InternedString operator+(const VString& p_other) const {
        InternedString re{*this};
        re += p_other;
        return re;
    }
    operator VString() const;

    static void configure();
    static int cleanup();
};

#endif //NEXUS_INTERNED_STRING_H
