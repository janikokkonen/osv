/* Copyright 2014 Huawei Technologies Duesseldorf GmbH
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

.global __setjmp
.global _setjmp
.global setjmp
.type __setjmp,@function
.type _setjmp,@function
.type setjmp,@function
__setjmp:
_setjmp:
setjmp:
    stp x19, x20, [x0]
    stp	x21, x22, [x0, #16]
    stp	x23, x24, [x0, #32]
    stp	x25, x26, [x0, #48]
    stp	x27, x28, [x0, #64]
    stp	x29, x30, [x0, #80]
    stp d8,  d9,  [x0, #96]    // save floating point registers
    stp d10, d11, [x0, #112]
    stp d12, d13, [x0, #128]
    stp d14, d15, [x0, #144]
    mov x2, sp                  // save stack pointer
    str x2, [x0, #160]
    mov x0, #0                  //always return 0
    ret
