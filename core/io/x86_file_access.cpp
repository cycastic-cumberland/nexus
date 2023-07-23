//
// Created by cycastic on 7/20/2023.
//

#include "x86_file_access.h"

void x86FileAccess::close() {
    flush();
    f.close();
}

void x86FileAccess::flush() {
    f.flush();
}


bool x86FileAccess::is_open() const {
    return f.is_open();
}

void x86FileAccess::open(const VString &p_file_path, FileAccess::AccessType p_access_type) {
    if (is_open()) close();
    if (p_access_type == ACCESS_READ)
        f.open(p_file_path.utf8().c_str(), std::ios::in  | std::ios::binary);
    else if (p_access_type == ACCESS_WRITE)
        f.open(p_file_path.utf8().c_str(), std::ios::out | std::ios::binary);
    else if (p_access_type == ACCESS_APPEND)
        f.open(p_file_path.utf8().c_str(), std::ios::app | std::ios::binary);
    else throw FileException("Access type not supported");
    if (!is_open()) throw FileCannotBeOpenedException(VString("Cannot open file: ") + p_file_path);
    file_path = p_file_path;
    access_type = p_access_type;
}

size_t x86FileAccess::get_pos() const {
    return const_cast<x86FileAccess*>(this)->f.tellg();
}

size_t x86FileAccess::get_file_size() const {
    auto mutable_self = const_cast<x86FileAccess*>(this);
    // Get the current_callback data_size
    auto pos = mutable_self->get_pos();
    // Move toward the end
    mutable_self->f.seekg(0, std::ios_base::end);
    // Extract the file data_size
    auto size = mutable_self->get_pos();
    // Return to the old pos
    mutable_self->seek((int64_t)pos);
    return size;
}

void x86FileAccess::seek(const size_t &p_pos) {
    f.seekg((int64_t)p_pos);
}

bool x86FileAccess::eof_reached() const {
    return const_cast<x86FileAccess*>(this)->f.peek() == std::char_traits<char>::eof();
}

uint8_t x86FileAccess::peek_8() const {
    if (!is_open()) throw FileNotOpenedException();
    if (!(access_type & ACCESS_READ)) throw ReadViolationException("Read operation not permitted");
    if (eof_reached()) throw EOFReachedException("End of file reached");
    auto b = const_cast<x86FileAccess*>(this)->f.peek();
    return *(uint8_t*)(&b);
}

uint8_t x86FileAccess::get_8() const {
    if (!is_open()) throw FileNotOpenedException();
    if (!(access_type & ACCESS_READ)) throw ReadViolationException("Read operation not permitted");
    if (eof_reached()) throw EOFReachedException("End of file reached");
    auto b = const_cast<x86FileAccess*>(this)->f.get();
//    if (b == std::char_traits<char>::eof()) throw EOFReachedException();
    return *(uint8_t*)(&b);
}

void x86FileAccess::store_8(const uint8_t &p_data) {
    if (!is_open()) throw FileNotOpenedException();
    if (!(access_type & ACCESS_WRITE)) throw ReadViolationException("Write operation not permitted");
    f.put(*(char*)&p_data);
}

FileAccess *x86FileAccess::duplicate() const {
    auto ptr = new x86FileAccess(access_type, get_endian_mode());
    if (is_open()) {
        ptr->open(get_absolute_path(), access_type);
        ptr->seek(get_pos());
    }
    return ptr;
}
