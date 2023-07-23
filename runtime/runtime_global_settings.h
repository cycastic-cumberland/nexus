//
// Created by cycastic on 7/22/2023.
//

#ifndef NEXUS_RUNTIME_GLOBAL_SETTINGS_H
#define NEXUS_RUNTIME_GLOBAL_SETTINGS_H

#include "../core/typedefs.h"


struct NexusRuntimeGlobalSettings {
    uint32_t stack_size;
    bool bytecode_endian_mode;
private:
    static NexusRuntimeGlobalSettings settings;
public:
    _NO_DISCARD_ static _FORCE_INLINE_ const NexusRuntimeGlobalSettings& get_settings() { return settings; }
};


#endif //NEXUS_RUNTIME_GLOBAL_SETTINGS_H
