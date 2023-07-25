//
// Created by cycastic on 7/24/2023.
//

#ifndef NEXUS_NEXUS_OUTPUT_H
#define NEXUS_NEXUS_OUTPUT_H

#include "system.h"

static void print_line(const VString& p_msg) {
    System::get_singleton()->print_line(p_msg);
}

static void print_line(const CharString& p_msg) {
    print_line(VString(p_msg.c_str()));
}

static void print_line(const char* p_msg) {
    print_line(VString(p_msg));
}

static void print_error(const VString& p_msg) {
    System::get_singleton()->print_error(p_msg);
}

static void print_error(const CharString& p_msg) {
    print_error(VString(p_msg.c_str()));
}

static void print_error(const char* p_msg) {
    print_error(VString(p_msg));
}

static void print_warning(const VString& p_msg) {
    System::get_singleton()->print_warning(p_msg);
}

static void print_warning(const CharString& p_msg) {
    print_warning(VString(p_msg.c_str()));
}

static void print_warning(const char* p_msg) {
    print_warning(VString(p_msg));
}

#endif //NEXUS_NEXUS_OUTPUT_H
