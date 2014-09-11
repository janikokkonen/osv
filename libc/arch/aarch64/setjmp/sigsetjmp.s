/* Copyright 2014 Huawei Technologies Duesseldorf GmbH
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

.global _sigsetjmp
.global sigsetjmp
.type _sigsetjmp,@function
.type sigsetjmp,@function
_sigsetjmp:
sigsetjmp:
    str x1, [x0, #-176]         // store savemask in __jmp_buf
    cbz x1, 2f                   // if x1 zero, do not save sigmask
    stp x0, x30, [sp, #-16]!    // save jmp_buf and lr to stack
    add x2, x0, #-184           // store current sigmask in x2
    mov x1, #0                  // new sigmask
    mov x0, #2                  // SIG_SETMASK
    bl sigprocmask
    ldp x0, x30, [sp], #16      // load jmp_buf and lr to stack
2:
    b setjmp
