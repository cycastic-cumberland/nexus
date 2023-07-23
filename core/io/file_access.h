//
// Created by cycastic on 7/20/2023.
// Stolen from Godot
//

#ifndef NEXUS_FILE_ACCESS_H
#define NEXUS_FILE_ACCESS_H

#include "../types/object.h"
#include "../types/vstring.h"
#include "../exception.h"

class FileException : public Exception {
public:
    explicit FileException(const char* p_err = nullptr) : Exception(p_err) {}
};

class InvalidBufferException : public FileException {
public:
    explicit InvalidBufferException(const char* p_err) : FileException(p_err) {}
};

class EOFReachedException : public FileException {
public:
    explicit EOFReachedException(const char* p_err = nullptr) : FileException(p_err) {}
};

class ReadViolationException : public FileException {
public:
    explicit ReadViolationException(const char* p_err = nullptr) : FileException(p_err) {}
};

class WriteViolationException : public FileException {
public:
    explicit WriteViolationException(const char* p_err = nullptr) : FileException(p_err) {}
};

class FileNotOpenedException : public FileException {
public:
    explicit FileNotOpenedException(const char* p_err = nullptr) : FileException(p_err) {}
};

class FileNotFoundException : public FileException {
public:
    explicit FileNotFoundException(const char* p_err = nullptr) : FileException(p_err) {}
    explicit FileNotFoundException(const VString& p_err) : FileException(p_err.utf8().c_str()) {}
};

class FileCannotBeOpenedException : public FileException {
public:
    explicit FileCannotBeOpenedException(const char* p_err = nullptr) : FileException(p_err) {}
    explicit FileCannotBeOpenedException(const VString& p_err) : FileException(p_err.utf8().c_str()) {}
};

class FileAccess : public Object {
public:
    enum AccessType : unsigned int{
        ACCESS_READ = 1,
        ACCESS_WRITE = 2,
        ACCESS_RESERVED_APPEND = 4,
        ACCESS_READ_WRITE = ACCESS_READ | ACCESS_WRITE,
        ACCESS_APPEND = ACCESS_RESERVED_APPEND | ACCESS_WRITE,
        ACCESS_MAX
    };
private:
    bool endian_swap{};
protected:
    AccessType access_type;
public:
    virtual void close() = 0;
    virtual void flush() {}
    virtual bool is_open() const = 0;
    virtual void open(const VString& p_file_path, AccessType p_access_type) = 0;

    virtual VString get_path() const { return ""; }
    virtual VString get_absolute_path() const { return ""; }

    virtual bool get_endian_mode() const { return endian_swap; }
    virtual void set_endian_mode(const bool& p_endian) { endian_swap = p_endian; }
    virtual FileAccess* duplicate() const = 0;

    virtual size_t get_pos() const = 0;
    virtual size_t get_file_size() const = 0;
    virtual void seek(const size_t& p_pos) = 0;
    virtual void seek_start() { seek(0); }
    virtual void seek_end() { seek(get_file_size()); }
    virtual bool eof_reached() const { return get_pos() >= get_file_size(); }
    AccessType get_access_type() const { return access_type; }

    virtual uint8_t peek_8() const = 0;
    virtual uint8_t get_8() const = 0;
    virtual uint16_t get_16() const;
    virtual uint32_t get_32() const;
    virtual uint64_t get_64() const;
    virtual float get_float() const;
    virtual double get_double() const;
    virtual CharString get_char_string() const;
    VString get_string_text(bool p_skip_cr = true) const;
    VString get_string() const;
    virtual VString get_line() const;
    virtual uint64_t get_buffer(uint8_t* p_buffer, const size_t& p_bytes_count) const;

    virtual void store_8(const uint8_t& p_data) = 0;
    virtual void store_16(const uint16_t& p_data);
    virtual void store_32(const uint32_t& p_data);
    virtual void store_64(const uint64_t& p_data);
    virtual void store_float(const float& p_data);
    virtual void store_double(const double& p_data);
    virtual void store_char_string(const CharString& p_from);
    virtual void store_string_text(const VString& p_from);
    virtual void store_string(const VString& p_from);
    virtual void store_buffer(const uint8_t* p_buffer, const size_t& p_bytes_count);

    explicit FileAccess(AccessType p_type, const bool& p_endian_swap = false) : Object(), access_type(p_type), endian_swap(p_endian_swap) {}
};

#endif //NEXUS_FILE_ACCESS_H
