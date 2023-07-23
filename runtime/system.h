//
// Created by cycastic on 7/22/2023.
//

#ifndef NEXUS_SYSTEM_H
#define NEXUS_SYSTEM_H

#include <thread>
#include "../core/typedefs.h"
#include "../core/types/vstring.h"

class SystemException : public Exception {
public:
    explicit SystemException(const char* p_msg) : Exception(p_msg) {}
    explicit SystemException(const CharString& p_msg) : Exception(p_msg) {}
};

class System {
protected:
    static System* singleton;
public:
    static _FORCE_INLINE_ System* get_singleton() { return singleton; }
    static _FORCE_INLINE_ void cleanup() { delete singleton; }
    virtual void load_dynamic_library(const VString& p_lib_path, void*& p_library_handle, const bool& p_also_set_library_path = false){
        throw SystemException("Unavailable");
    }
    virtual bool close_dynamic_library(void *p_library_handle){
        throw SystemException("Unavailable");
    }
    virtual void get_dynamic_library_symbol_handle(void *p_library_handle, const CharString& p_name, void *&p_symbol_handle, const bool& p_optional = false){
        throw SystemException("Unavailable");
    }
    virtual void yield() { std::this_thread::yield(); }
    virtual ~System() = default;
};

#endif //NEXUS_SYSTEM_H
