/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * Copyright (C) 2014 Huawei Technologies Duesseldorf GmbH
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#include <osv/align.hh>
#include "exceptions.hh"
#include <signal.h>
#include <stdlib.h>
#include <arch-cpu.hh>
#include <osv/debug.hh>

extern unsigned long signal_stack_top;

namespace arch {

struct signal_frame {
    exception_frame state;
    siginfo_t si;
    struct sigaction sa;
    struct sched::arch_fpu fpu;
};

}

extern "C" {
    void call_signal_handler(arch::signal_frame* frame);
    void call_signal_handler_thunk(void);
}

namespace arch {

void build_signal_frame(exception_frame* ef,
                        const siginfo_t& si,
                        const struct sigaction& sa)
{
    void *sp_el0_stack_addr = nullptr;
    sp_el0_stack_addr = reinterpret_cast<void *>(&signal_stack_top);
    sp_el0_stack_addr -= sizeof(signal_frame);
    sp_el0_stack_addr = align_down(sp_el0_stack_addr, 16);
    signal_frame* frame = static_cast<signal_frame*>(sp_el0_stack_addr);
    frame->state = *ef;
    frame->si = si;
    frame->sa = sa;
    ef->elr = reinterpret_cast<ulong>(call_signal_handler_thunk);  //jump to call_signal_handler_thunk 
    ef->regs[30] += 4;        //jump over faulting address
    asm volatile ("mov x20, %0 \n" :: "r"(sp_el0_stack_addr): "x20");
    asm volatile ("isb; msr sp_el0, x20; isb \n");      //save signaling stack on sp_el0
}

}

//unsigned __thread signal_nesting;

void call_signal_handler(arch::signal_frame* frame)
{
    debug_early("JKo: entered call_signal_handler\n");
//    if (signal_nesting) {
//         Note: nested signals are legal, but rarely used, so they usually
//         indicate trouble
//         abort("nested signals");
//     }
//    ++signal_nesting;
    frame->fpu.save();
    if (frame->sa.sa_flags & SA_SIGINFO) {
        debug_early("JKo: entered call_signal_handler siginfo\n");
        ucontext_t uc = {};
        auto& regs = uc.uc_mcontext.regs;
        auto& f = frame->state;
        regs[0] = f.regs[0];
        regs[1] = f.regs[1];
        regs[2] = f.regs[2];
        regs[3] = f.regs[3];
        regs[4] = f.regs[4];
        regs[5] = f.regs[5];
        regs[6] = f.regs[6];
        regs[7] = f.regs[7];
        regs[8] = f.regs[8];
        regs[9] = f.regs[9];
        regs[10] = f.regs[10];
        regs[11] = f.regs[11];
        regs[12] = f.regs[12];
        regs[13] = f.regs[13];
        regs[14] = f.regs[14];
        regs[15] = f.regs[15];
        regs[16] = f.regs[16];
        regs[17] = f.regs[17];
        regs[18] = f.regs[18];
        regs[19] = f.regs[19];
        regs[20] = f.regs[20];
        regs[21] = f.regs[21];
        regs[22] = f.regs[22];
        regs[23] = f.regs[23];
        regs[24] = f.regs[24];
        regs[25] = f.regs[25];
        regs[26] = f.regs[26];
        regs[27] = f.regs[27];
        regs[28] = f.regs[28];
        regs[29] = f.regs[29];
        regs[30] = f.regs[30];
        frame->sa.sa_sigaction(frame->si.si_signo, &frame->si, &uc);
        f.regs[0] = regs[0];
        f.regs[1] = regs[1];
        f.regs[2] = regs[2];
        f.regs[3] = regs[3];
        f.regs[4] = regs[4];
        f.regs[5] = regs[5];
        f.regs[6] = regs[6];
        f.regs[7] = regs[7];
        f.regs[8] = regs[8];
        f.regs[9] = regs[9];
        f.regs[10] = regs[10];
        f.regs[11] = regs[11];
        f.regs[12] = regs[12];
        f.regs[13] = regs[13];
        f.regs[14] = regs[14];
        f.regs[15] = regs[15];
        f.regs[16] = regs[16];
        f.regs[17] = regs[17];
        f.regs[18] = regs[18];
        f.regs[19] = regs[19];
        f.regs[20] = regs[20];
        f.regs[21] = regs[21];
        f.regs[22] = regs[22];
        f.regs[23] = regs[23];
        f.regs[24] = regs[24];
        f.regs[25] = regs[25];
        f.regs[26] = regs[26];
        f.regs[27] = regs[27];
        f.regs[28] = regs[28];
        f.regs[29] = regs[29];
        f.regs[30] = regs[30];
    } else {
        debug_early("JKo: entered call_signal_handler sa_handler\n");
        frame->sa.sa_handler(frame->si.si_signo);
    }
    frame->fpu.restore();
    //  --signal_nesting;                 //add
    // FIXME: all the other glory details
}
