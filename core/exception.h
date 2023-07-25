//
// Created by cycastic on 7/20/2023.
//

#ifndef NEXUS_EXCEPTION_H
#define NEXUS_EXCEPTION_H

#include <exception>
#include "typedefs.h"

class CharString;

class Exception : public std::exception {
private:
    CharString *message;
public:
    explicit Exception(const CharString& p_message);
    explicit Exception(const char* p_message = nullptr);
    ~Exception() override;

    _NO_DISCARD_ const char* what() const noexcept override;
};

#endif //NEXUS_EXCEPTION_H
