#include "arch-cpu.hh"
#include <osv/debug.hh>
#include <osv/sched.hh>
#include <osv/mmu.hh>
#include <osv/debug.h>
#include <osv/prio.hh>


void dump_frame(exception_frame *ef) {

    debug_early_u64("x0   ", (u64)ef->regs[0]);
    debug_early_u64("x1   ", (u64)ef->regs[1]);
    debug_early_u64("x2   ", (u64)ef->regs[2]);
    debug_early_u64("x30  ", (u64)ef->regs[29]);
    debug_early_u64("x31  ", (u64)ef->regs[30]);
    debug_early_u64("sp   ", (u64)ef->sp);
    debug_early_u64("pc   ", (u64)ef->pc);
    debug_early_u64("pstate  ", (u64)ef->pstate);
}

void page_fault(exception_frame *ef, u64 addr)
{
    static int i = 0;
    debug_early("entering page_fault handler\n");
    //dump_frame(ef);
    debug_early_u64("faulting address ", (u64)addr);

    //sched::exception_guard g;
    //auto addr = processor::read_cr2();
    //if (fixup_fault(ef)) {
    //    return;
    //} 
    auto pc = reinterpret_cast<void*>(ef->pc);
    if (!pc) {
        abort("trying to execute null pointer");
    }
    // The following code may sleep. So let's verify the fault did not happen
    // when preemption was disabled, or interrupts were disabled.
    assert(sched::preemptable());
    assert(ef->pstate & processor::daif_i);

    // And since we may sleep, make sure interrupts are enabled.
    //DROP_LOCK(irq_lock) { // irq_lock is acquired by HW
        //sched::inplace_arch_fpu fpu;   //check this
        //fpu.save();
         mmu::vm_fault(addr, ef);
        //fpu.restore();
    //}
    i++;
    debug_early("exiting page_fault handler\n");
    if(i == 2)
        asm("wfi");


} 

namespace mmu {

void flush_tlb_all() {
    asm volatile("dsb sy; tlbi vmalle1is; dsb sy; isb;");
}

void flush_tlb_local() {
    asm volatile("dsb sy; tlbi vmalle1; dsb sy; isb;");
}

static pt_element page_table_root[2] __attribute__((init_priority((int)init_prio::pt_root)));

void switch_to_runtime_page_tables()
{
    auto pt_ttbr0 = mmu::page_table_root[0].next_pt_addr();
    auto pt_ttbr1 = mmu::page_table_root[1].next_pt_addr();
    asm volatile("msr ttbr0_el1, %0; isb;" ::"r" (pt_ttbr0));
    asm volatile("msr ttbr1_el1, %0; isb;" ::"r" (pt_ttbr1));
    mmu::flush_tlb_all();
}

pt_element *get_root_pt(uintptr_t virt)
{
    return &page_table_root[virt >> 63];
}

pt_element make_empty_pte() {  return pt_element(); }

pt_element make_pte(phys addr, bool large,
                    unsigned perm = perm_read | perm_write | perm_exec)
{
    if(anon_flag)
        debug_early_u64("make pte :", (u64)addr); 
    pt_element pte;
    pte.set_valid(perm != 0);
    pte.set_writable(perm & perm_write);
    pte.set_executable(perm & perm_exec);
    pte.set_dirty(true);
    pte.set_large(large);
    pte.set_addr(addr, large);
    arch_pt_element::set_user(&pte, false);
    arch_pt_element::set_accessed(&pte, true);
    arch_pt_element::set_share(&pte, true);
    if (addr >= mmu::device_range_start && addr < mmu::device_range_stop) {
        /* we need to mark device memory as such, because the
           semantics of the load/store instructions change */
        debug_early_u64("make_pte: device memory at ", (u64)addr);
        arch_pt_element::set_attridx(&pte, 0);
    } else {
        arch_pt_element::set_attridx(&pte, 4);
    }

    return pte;
}

pt_element make_normal_pte(phys addr, unsigned perm)
{
    return make_pte(addr, false, perm);
}

pt_element make_large_pte(phys addr, unsigned perm)
{
    return make_pte(addr, true, perm);
}

bool is_page_fault_insn(unsigned int esr) {
    unsigned int ec = esr >> 26;
    return ec == 0x20 || ec == 0x21;
}

bool is_page_fault_write(unsigned int esr) {
    unsigned int ec = esr >> 26;
    return (ec == 0x24 || ec == 0x25) && (esr & 0x40);
    //return (ec == 0x24 || ec == 0x25);

}

bool is_page_fault_write_exclusive(unsigned int esr) {
    return is_page_fault_write(esr);
}

}
