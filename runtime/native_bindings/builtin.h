//
// Created by cycastic on 7/25/2023.
//

#ifndef NEXUS_BUILTIN_H
#define NEXUS_BUILTIN_H

#include "../nexus_stack.h"
#include "../nexus_output.h"

void __builtin_println(NexusStack* p_stack){
    auto param_0 = (VString*)p_stack->get_last_frame().get_at(-1).data;

    print_line(*param_0);
}

#endif //NEXUS_BUILTIN_H
