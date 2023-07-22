//
// Created by cycastic on 7/20/2023.
//

#ifndef NEXUS_EXCEPTION_H
#define NEXUS_EXCEPTION_H

#include <exception>
#include "typedefs.h"
#include "types/char_string.h"

class Exception : public std::exception {
public:
    explicit Exception(const CharString& p_message) : message(p_message) {}
    explicit Exception(const char* p_message = nullptr) : message(p_message ? p_message : "") {}

    _NO_DISCARD_ _FORCE_INLINE_ const char* what() const noexcept override {
        return message.c_str();
    }

private:
    CharString message;
};

#endif //NEXUS_EXCEPTION_H
