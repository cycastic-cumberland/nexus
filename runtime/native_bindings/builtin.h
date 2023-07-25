//
// Created by cycastic on 7/19/2023.
//

#ifndef NEXUS_BUILTIN_H
#define NEXUS_BUILTIN_H

#include "../../core/types/vstring.h"
#include "../nexus_output.h"

static void builtin_println(VString* p_out){
    print_line(*p_out);
}

#endif //NEXUS_BUILTIN_H
