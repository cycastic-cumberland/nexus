//
// Created by cycastic on 7/19/2023.
//

#ifndef NEXUS_BUILTIN_H
#define NEXUS_BUILTIN_H

#include "../../core/types/vstring.h"
#include <iostream>

static void builtin_print(VString* p_out){
    std::wcout << *p_out << std::endl;
}

#endif //NEXUS_BUILTIN_H
