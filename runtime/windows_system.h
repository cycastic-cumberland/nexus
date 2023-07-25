//
// Created by cycastic on 7/22/2023.
//

#ifndef NEXUS_WINDOWS_SYSTEM_H
#define NEXUS_WINDOWS_SYSTEM_H

#include "system.h"

class WindowsSystem : public System {
public:
    void load_dynamic_library(const VString& p_lib_path, void*& p_library_handle, const bool& p_also_set_library_path = false) override;
    bool close_dynamic_library(void *p_library_handle) override;
    void get_dynamic_library_symbol_handle(void *p_library_handle, const CharString& p_name, void *&p_symbol_handle, const bool& p_optional = false) override;
    void print_line(const VString& p_message) override;
    void print_error(const VString& p_message) override { print_line(p_message); }
    void print_warning(const VString& p_message) override { print_line(p_message); }
    WindowsSystem();
    ~WindowsSystem() override;
};

#endif //NEXUS_WINDOWS_SYSTEM_H
