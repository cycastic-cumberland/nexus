//
// Created by cycastic on 7/19/2023.
//

#ifndef NEXUS_I32_H
#define NEXUS_I32_H

#include <cstdint>

static void i32_init(int32_t* p_ret){
    *p_ret = 0;
}

static void i32_destroy(int32_t* p_obj){
    // Nothing in particular...
    // Stack item get cleaned up by the runtime
}

static void i32_static__get_max(int32_t* p_ret){
    *p_ret = 2147483647;
}

static void i32_static__get_min(int32_t* p_ret){
    *p_ret = -2147483648;
}

static void i32__register(){

}

#endif //NEXUS_I32_H
