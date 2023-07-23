//
// Created by cycastic on 7/20/2023.
//

#ifndef NEXUS_FILE_ACCESS_SERVER_H
#define NEXUS_FILE_ACCESS_SERVER_H
#if defined(__linux__) || defined(__unix__) || defined(_WIN32) || defined(_WIN64)
#include "x86_file_access.h"
#endif

typedef Ref<FileAccess> FilePointer;

class FileAccessServer {
public:
    static FilePointer open(const VString& p_file_path, FileAccess::AccessType p_access_type, const bool& p_endian_swap = false){
#if defined(__linux__) || defined(__unix__) || defined(_WIN32) || defined(_WIN64)
        auto access = new x86FileAccess(p_access_type, p_endian_swap);
#else
        return {};
#endif
        access->open(p_file_path, p_access_type);
        Ref<FileAccess> ref = access;
        return ref;
    }
    static FilePointer open_virtual(FileAccess::AccessType p_access_type = FileAccess::ACCESS_READ_WRITE, const bool& p_endian_swap = false){
        Ref<FileAccess> ref = new VirtualFileAccess(p_access_type, p_endian_swap);
        return ref;
    }
    static FilePointer copy_to_memory(const VString& p_file_path, const bool& p_endian_swap = false){
        auto file_ptr = open(p_file_path, FileAccess::ACCESS_READ, p_endian_swap);
        auto virtual_ptr = open_virtual();
        auto size = file_ptr->get_file_size();
        auto buffer = (uint8_t*) malloc(size);
        file_ptr->get_buffer(buffer, size);
        virtual_ptr->store_buffer(buffer, size);
        virtual_ptr->seek_start();
        free(buffer);
        return virtual_ptr;
    }
    static FilePointer duplicate_pointer(const FilePointer& p_pointer){
        FilePointer re = p_pointer->duplicate();
        return re;
    }
    static bool exists(const VString& p_file_path){
        auto file_ptr = open(p_file_path, FileAccess::ACCESS_READ, false);
        auto file_exists = file_ptr->is_open();
        file_ptr->close();
        return file_exists;
    }
};

#endif //NEXUS_FILE_ACCESS_SERVER_H
