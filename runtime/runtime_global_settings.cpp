//
// Created by cycastic on 7/22/2023.
//

#include "runtime_global_settings.h"

NexusRuntimeGlobalSettings NexusRuntimeGlobalSettings::settings = {
        .stack_size = 1024 * 1024 * 16, // 16 MiB
        .bytecode_endian_mode = false,
};
