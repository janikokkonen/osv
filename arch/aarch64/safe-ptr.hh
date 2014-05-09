/*
 * Copyright (C) 2014 Huawei Technologies Duesseldorf GmbH
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#ifndef SAFE_PTR_HH_
#define SAFE_PTR_HH_

#include <osv/compiler.h>

/* warning: not "safe" at all for now. */
template <typename T>
static inline bool
safe_load(const T* potentially_bad_pointer, T& data)
{
    unsigned long long reg = 0;
    unsigned char ok = true;
    asm
        ("1: \n\t"
        "ldr %[reg], %[ptr] \n\t"
        "str %[reg], %[data] \n\t"
        "2: \n\t"
        ".pushsection .text.fixup, \"ax\" \n\t"
        "3: \n\t"
        "mov %[ok], #0 \n\t"
        "b 2b \n\t"
        ".popsection \n\t"
        ".pushsection .fixup, \"aw\" \n\t"
        ".quad 1b, 3b \n\t"
        ".popsection\n"
        : [reg]"+&r"(reg), [data]"=Q"(data), [ok]"=r"(ok)
        : [ptr]"Q"(*potentially_bad_pointer)
        : "memory" );
    return ok;
}

template <typename T>
static inline bool
safe_store(const T* potentially_bad_pointer, const T& data)
{
    unsigned long long reg = 0;
    asm goto
        ("1: \n\t"
         "ldr %[reg], %[data] \n\t"
         "str %[reg], %[ptr] \n\t"
         ".pushsection .fixup, \"aw\" \n\t"
         ".quad 1b, %l[fail] \n\t"
         ".popsection\n"
         :
         : [reg]"r"(reg), [ptr]"Q"(*potentially_bad_pointer), [data]"Q"(data)
         : "memory"
         : fail);
    return true;
    fail: ATTR_COLD_LABEL;
    return false;
}

#endif /* SAFE_PTR_HH_ */
