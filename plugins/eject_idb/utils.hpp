#pragma once

#include <stdio.h>

#ifdef __IDP__
#include <pro.h>
#include <loader.hpp>

inline qstring get_idb_path() { return get_path(PATH_TYPE_IDB); }

#endif

/*
 * Hash function (djb2) by Dan Bernstein.
 * Source: Originally described in comp.lang.c
 * This implementation is based on the description of the algorithm.
 */
inline unsigned long djb2(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

inline void make_semaphore_name(const char* string, char* out, size_t out_size) {
    _snprintf(out, out_size, "ejectidb_%08lx", djb2(string));
}
