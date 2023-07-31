//
// Literally stolen from Godot's StringName
//

#ifndef NEXUS_STRING_LITERAL_H
#define NEXUS_STRING_LITERAL_H

#include "../lock.h"
#include "../exception.h"
#include "safe_refcount.h"
#include "vstring.h"

class StringLiteral;
class StringLiteralServer;

class StringLiteralException : public Exception {
public:
    explicit StringLiteralException(const char *p_msg) : Exception(p_msg) {}
};

class StringLiteral {
    enum {
        STRING_TABLE_BITS = 12,
        STRING_TABLE_LEN = 1 << STRING_TABLE_BITS,
        STRING_TABLE_MASK = STRING_TABLE_LEN - 1
    };
    struct StringCell  {
        StringCell* prev;
        StringCell* next;
        uint32_t hash;
        SafeRefCount refcount;

        VString data;

    };
    static BinaryMutex lock;
    static StringLiteral::StringCell* table[STRING_TABLE_LEN];
    static bool configured;

    StringCell* occupying{};
    void ref(StringCell* cell);
    void unref();
public:
    _NO_DISCARD_ _FORCE_INLINE_ const wchar_t* c_str() const { return occupying ? occupying->data.c_str() : L""; }
    _NO_DISCARD_ _FORCE_INLINE_ size_t length() const { return occupying ? occupying->data.length() : 0; }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return length() == 0; }
    _NO_DISCARD_ _FORCE_INLINE_ bool operator==(const StringLiteral& p_other) const { return occupying == p_other.occupying; }
    _NO_DISCARD_ _FORCE_INLINE_ bool operator!=(const StringLiteral& p_other) const { return !(*this == p_other); }

    StringLiteral& operator=(const StringLiteral& p_other);
    StringLiteral& operator=(const char* p_from);
    StringLiteral& operator=(const wchar_t* p_from);
    StringLiteral& operator=(const VString& p_from);

    StringLiteral();
    StringLiteral(const StringLiteral& p_other);
    StringLiteral(const char* p_from);
    StringLiteral(const wchar_t* p_from);
    StringLiteral(const VString& p_from);
    ~StringLiteral();

    static void configure();
    static int cleanup();
};

#endif //NEXUS_STRING_LITERAL_H
