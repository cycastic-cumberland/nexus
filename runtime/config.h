//
// Created by cycastic on 7/24/2023.
//

#ifndef NEXUS_CONFIG_H
#define NEXUS_CONFIG_H

#if defined(_WIN32) || defined(_WIN64)
#include "windows_system.h"
#elif defined(__linux__)
#include "unix_system.h"
#else
#include "system.h"
#endif

#include "runtime_global_settings.h"
#include "task_scheduler.h"
#include "../core/output.h"

static System* nexus_system = nullptr;
static TaskScheduler* task_scheduler = nullptr;
static NexusRuntimeGlobalSettings* nexus_settings = nullptr;

static void initialize_nexus_runtime(bool initialize_wcout){
    nexus_settings = new NexusRuntimeGlobalSettings{
        .stack_size = 1024 * 1024 * 16, // 16 MiB
        .stack_metadata_initial_capacity = 8, // 1024 stack objects before relocation
        .bytecode_endian_mode = false,
        .task_scheduler_max_request_per_cycle = 3,
        .task_scheduler_starting_thread_count = 3,
    };
    NexusRuntimeGlobalSettings::set_singleton(nexus_settings);
#if defined(_WIN32) || defined(_WIN64)
    nexus_system = new WindowsSystem();
#elif defined(__linux__)
    nexus_system = new UnixSystem();
#else
    nexus_system = nullptr;
#endif
    task_scheduler = new TaskScheduler();

    if (initialize_wcout) init_locale();
}

static void destroy_nexus_runtime(){
    delete task_scheduler;
    delete nexus_system;
    NexusRuntimeGlobalSettings::set_singleton(nullptr);
    delete nexus_settings;
}

#endif //NEXUS_CONFIG_H
