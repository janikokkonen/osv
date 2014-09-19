/* Copyright 2014 Huawei Technologies Duesseldorf GmbH
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

.global _longjmp
.global longjmp
.type _longjmp,@function
.type longjmp,@function
_longjmp:
longjmp:
        cbnz x1, 2f             //check if x1 != 0
        add x1, x1, #1          
2:
        ldp	x19, x20, [x0]
        ldp	x21, x22, [x0, #16]
	ldp	x23, x24, [x0, #32]
	ldp	x25, x26, [x0, #48]
	ldp	x27, x28, [x0, #64]
	ldp	x29, x30, [x0, #80]
        ldp	d8,  d9,  [x0, #96]   //restore floating point registers
	ldp	d10, d11, [x0, #112]
	ldp	d12, d13, [x0, #128]
	ldp	d14, d15, [x0, #144]
        ldr	x5, [x0, #160]     // restore stack pointer
        mov	sp, x5
        mov     x0, x1            // return value      
	br	x30
