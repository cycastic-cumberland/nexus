//
// Created by cycastic on 7/17/2023.
//

#ifndef NEXUS_MEMF_H
#define NEXUS_MEMF_H

#include <memory.h>

template <class T>
void memdelete(T *p_class) {
    if (!std::is_trivially_destructible<T>::value) {
        p_class->~T();
    }

    free(p_class);
}

template <class T>
T* array_alloc(size_t s){
    return (T*)malloc(sizeof(T) * s);
}

#endif //NEXUS_MEMF_H
