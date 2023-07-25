//
// Created by cycastic on 7/24/2023.
//

#include "exception.h"
#include "types/char_string.h"

Exception::Exception(const CharString &p_message) {
    message = new CharString(p_message);
}

const char *Exception::what() const noexcept {
    return message->c_str();
}

Exception::Exception(const char *p_message) {
    message = new CharString(p_message ? p_message : "");
}

Exception::~Exception() {
    delete message;
}
