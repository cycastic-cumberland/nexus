//
// Created by cycastic on 7/20/2023.
//

#ifndef NEXUS_VIRTUAL_FILE_ACCESS_H
#define NEXUS_VIRTUAL_FILE_ACCESS_H

#include "file_access.h"

class VirtualFileAccess : public FileAccess {
private:
    mutable size_t position = 0;
    CowArray<uint8_t> data;
public:
    void close() override { data = CowArray<uint8_t>(); }
    bool is_open() const override { return true; }
    void open(const VString& p_file_path, AccessType p_access_type) override {}

    const uint8_t* ptr() const { return data.ptr(); }
    uint8_t* ptrw() { return data.ptrw(); }

    size_t get_pos() const override { return position; }
    size_t get_file_size() const override { return data.capacity(); }
    void seek(const size_t& p_pos) override { position = p_pos; }

    uint8_t peek_8() const override {
        if (!(access_type & ACCESS_READ)) throw ReadViolationException("Read operation not permitted");
        if (eof_reached()) throw EOFReachedException("End of file reached");
        return data.ptr()[position];
    }
    uint8_t get_8() const override {
        if (!(access_type & ACCESS_READ)) throw ReadViolationException("Read operation not permitted");
        if (eof_reached()) throw EOFReachedException("End of file reached");
        return data.ptr()[position++];
    }

    void store_8(const uint8_t& p_data) override {
        if (!(access_type & ACCESS_WRITE)) throw WriteViolationException("Write operation not permitted");
        if (eof_reached()){
            position++;
            data.push_back(p_data);
        } else data.ptrw()[position++] = p_data;
    }
    void store_buffer(const uint8_t* p_buffer, const size_t& p_bytes_count) override {
        auto current_capacity = data.capacity();
        auto end_region = position + p_bytes_count;
        if (end_region > current_capacity) data.resize(end_region);
        memcpy(data.ptrw(), p_buffer, p_bytes_count);
        position += p_bytes_count;
    }
public:
    void allocate(const size_t& p_bytes){ data.resize(get_file_size() + p_bytes); }

    VirtualFileAccess(AccessType p_type, const bool& p_endian_swap) : data(), FileAccess(p_type, p_endian_swap) {}
    VirtualFileAccess(const VirtualFileAccess& p_other) : VirtualFileAccess(p_other.access_type,
                                                                            p_other.get_endian_mode()){
        data = p_other.data;
        VirtualFileAccess::seek(p_other.get_pos());
    }
    FileAccess* duplicate() const override {
        return new VirtualFileAccess(*this);
    }
};

#endif //NEXUS_VIRTUAL_FILE_ACCESS_H
