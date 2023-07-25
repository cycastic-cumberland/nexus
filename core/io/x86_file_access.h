//
// Created by cycastic on 7/20/2023.
//

#ifndef NEXUS_X86_FILE_ACCESS
#define NEXUS_X86_FILE_ACCESS

#include <fstream>
#include "virtual_file_access.h"

class x86FileAccess : public FileAccess {
private:
    std::fstream f;
    VString file_path{};
public:
    void close() override;
    void flush() override;
    bool is_open() const override;
    void open(const VString& p_file_path, AccessType p_access_type) override;

    VString get_absolute_path() const override { return file_path; }
    VString get_path() const override { return get_absolute_path(); }

    size_t get_pos() const override;
    size_t get_file_size() const override;
    void seek(const size_t& p_pos) override;
    bool eof_reached() const override;

    uint8_t peek_8() const override;
    uint8_t get_8() const override;
    void store_8(const uint8_t& p_data) override;

    FileAccess* duplicate() const override;
public:

    x86FileAccess(AccessType p_type, const bool& p_endian_swap) : f(), FileAccess(p_type, p_endian_swap) {}
    ~x86FileAccess() override { x86FileAccess::close(); }
};

#endif //NEXUS_X86_FILE_ACCESS
