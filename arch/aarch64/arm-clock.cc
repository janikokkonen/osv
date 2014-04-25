/*
 * Copyright (C) 2014 Huawei Technologies Duesseldorf GmbH
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#include "drivers/clockevent.hh"
#include "drivers/clock.hh"
#include "arm-clock.hh"
#include "exceptions.hh"

#include <osv/debug.hh>
#include <osv/prio.hh>

using namespace processor;

#define NANO_PER_SEC 1000000000

class arm_clock : public clock {
public:
    arm_clock();
    /* Get the current value of the nanoseconds since boot */
    virtual s64 uptime();
    /* Get the nanoseconds since the epoc */
    virtual s64 time();
    /* Return current estimate of wall-clock time at OSV's boot. */
    virtual s64 boot_time();
protected:
    u32 freq_hz;  /* frequency in Hz (updates per second) */
    friend class arm_clock_events;
};

arm_clock::arm_clock() {
    asm volatile ("mrs %0, cntfrq_el0; isb; " : "=r"(freq_hz) :: "memory");
    if (freq_hz == 0) {
        abort("arm_clock(): read invalid frequency (0 Hz)\n");
    }
}

static __attribute__((constructor(init_prio::clock))) void setup_arm_clock()
{
    debug_early_entry("setup_arm_clock()");
    clock::register_clock(new arm_clock);
}

s64 arm_clock::uptime()
{
    u64 cntvct;
    asm volatile ("isb; mrs %0, cntvct_el0; isb; " : "=r"(cntvct) :: "memory");
    cntvct *= NANO_PER_SEC;
    cntvct /= this->freq_hz;
    return cntvct;
}

s64 arm_clock::time()
{
    return uptime(); /* XXX back to the 70s */
}

s64 arm_clock::boot_time()
{
    return uptime();
}

class arm_clock_events : public clock_event_driver {
public:
    arm_clock_events();
    ~arm_clock_events();
    virtual void setup_on_cpu();
    virtual void set(std::chrono::nanoseconds nanos);

protected:
    unsigned int read_ctl();
    void write_ctl(unsigned int cntv_ctl);
    unsigned int read_tval();
    void write_tval(unsigned int cntv_tval);

    static void irq_handler(struct interrupt_desc *);

    unsigned int irqid; /* global InterruptID */
};

arm_clock_events::arm_clock_events()
{
    this->irqid = 27; /* XXX */
    interrupt_table.register_handler(this->irqid, &arm_clock_events::irq_handler);
}

void arm_clock_events::irq_handler(struct interrupt_desc *desc)
{
    clock_event->callback()->fired();
}

arm_clock_events::~arm_clock_events()
{
}

void arm_clock_events::setup_on_cpu()
{
}

unsigned int arm_clock_events::read_ctl()
{
    unsigned int cntv_ctl;
    asm volatile ("isb; mrs %0, cntv_ctl_el0; isb;" : "=r"(cntv_ctl)
                  :: "memory");
    return cntv_ctl;
}

void arm_clock_events::write_ctl(unsigned int cntv_ctl)
{
    asm volatile ("isb; msr cntv_ctl_el0, %0; isb;" :: "r"(cntv_ctl)
                  : "memory");
}

unsigned int arm_clock_events::read_tval()
{
    unsigned int cntv_tval;
    asm volatile ("isb; mrs %0, cntv_tval_el0; isb;" : "=r"(cntv_tval)
                  :: "memory");
    return cntv_tval;
}

void arm_clock_events::write_tval(unsigned int cntv_tval)
{
    asm volatile ("isb; msr cntv_tval_el0, %0; isb;" :: "r"(cntv_tval)
                  : "memory");
}

void arm_clock_events::set(std::chrono::nanoseconds nanos)
{
    if (nanos.count() <= 0) {
        _callback->fired();
    } else {
        u64 tval = nanos.count();
        class arm_clock *c = static_cast<arm_clock *>(clock::get());
        tval *= c->freq_hz;
        tval /= NANO_PER_SEC;

        u32 ctl = this->read_ctl();
        ctl |= 1;  /* set enable */
        ctl &= ~2; /* unmask timer interrupt */
        this->write_ctl(ctl);
        this->write_tval(tval);
    }
}

void __attribute__((constructor)) setup_arm_clock_events()
{
    debug_early_entry("setup_arm_clock_events()");
    clock_event = new arm_clock_events;
}
