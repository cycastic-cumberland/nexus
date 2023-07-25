//
// Created by cycastic on 7/22/2023.
//

#ifndef NEXUS_RUNTIME_GLOBAL_SETTINGS_H
#define NEXUS_RUNTIME_GLOBAL_SETTINGS_H

#include "task.h"

struct NexusRuntimeGlobalSettings {
    uint32_t stack_size;
    bool bytecode_endian_mode;

    uint8_t task_scheduler_max_request_per_cycle;

private:
    static NexusRuntimeGlobalSettings* singleton;
public:
    _NO_DISCARD_ static _FORCE_INLINE_ const NexusRuntimeGlobalSettings* get_settings() { return singleton; }
    static void set_singleton(NexusRuntimeGlobalSettings* p_instance){ singleton = p_instance; }

};


#endif //NEXUS_RUNTIME_GLOBAL_SETTINGS_H
