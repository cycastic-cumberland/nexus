//
// Created by cycastic on 8/6/2023.
//

#ifdef __linux__

#include <iostream>
#include <dlfcn.h>
#include "unix_system.h"
#include "../core/io/file_access_server.h"


void UnixSystem::load_dynamic_library(const VString &p_lib_path, void *&p_library_handle,
                                      const bool &p_also_set_library_path) {
    VString path = p_lib_path;
    if (FileAccessServer::exists(path) && path.is_relative_path()){
        path = VString("./") + path;
    }
    if (!FileAccessServer::exists(path)){
        path = get_executable_path().get_base_dir().plus_file(p_lib_path.get_file());
    }
    if (!FileAccessServer::exists(path)){
        path = get_executable_path().get_base_dir().plus_file("../lib").plus_file(p_lib_path.get_file());
    }
    p_library_handle = dlopen(path.utf8().c_str(), RTLD_NOW);
    if (!p_library_handle) throw SystemException((VString("Can't open dynamic library: ") + p_lib_path
        + ". Error: " + dlerror()).utf8());
}

bool UnixSystem::close_dynamic_library(void *p_library_handle) {
    if (dlclose(p_library_handle)) return false;
    return true;
}

void UnixSystem::get_dynamic_library_symbol_handle(void *p_library_handle, const CharString &p_name, void *&p_symbol_handle,
                                              const bool &p_optional) {
    const char *error;
    dlerror();
    p_symbol_handle = dlsym(p_library_handle, p_name.c_str());
    error = dlerror();
    if (error != nullptr) throw SystemException(CharString("Can't resolve symbol: ") + p_name
        + ". Error: " + error + ".");
}

void UnixSystem::print_line(const VString &p_message) {
    std::wcout << p_message << L"\n";
}

UnixSystem::UnixSystem() {
    singleton = this;
}

UnixSystem::~UnixSystem() {
    singleton = nullptr;
}

#endif