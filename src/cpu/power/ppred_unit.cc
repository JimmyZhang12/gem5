/*
 * Copyright (c) 2020, University of Illinois
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2004-2005 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Andrew Smith
 */

#include "cpu/power/ppred_unit.hh"

#include <algorithm>

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "debug/PowerPred.hh"

PPredUnit::PPredUnit(const Params *params)
    : ClockedObject(params),
    min_current((double)params->min_current),
    max_current((double)params->max_current),
    period(params->period),
    cycle_period(params->cycle_period),
    delta(params->delta),
    emergency(params->emergency),
    clk(params->clk),
    emergency_throttle(params->emergency_throttle),
    voltage_set(params->voltage_set)
{
    DPRINTF(PowerPred, "PPredUnit::PPredUnit()\n");
    supply_voltage = 0.0;
    supply_current = 0.0;

    DPRINTF(PowerPred, "ppred_events.size(): %d\n",
              PPred::ppred_events.size());
    this->id = (int)PPred::ppred_events.size();
    PPred::ppred_events.push_back(NULL);

    vpi_shm::set_voltage_set(voltage_set);
    vpi_shm::set_freq(clk, cycle_period);

    PPred::interface.sim_period = this->period;
    PPred::interface.cycle_period = this->cycle_period;

    /*
     * If the period is set to 0, then we do not want to dump
     * periodically, thus we deschedule the event. Else, if the
     * period is not 0, but the event has already been scheduled,
     * we need to get rid of the old event before we create a new
     * one, as the old event will no longer be moved forward in the
     * event that we resume from a checkpoint.
     */
    if (PPred::ppred_events[this->id] != NULL && \
          (period == 0 || PPred::ppred_events[this->id]->scheduled())) {
        // Event should AutoDelete, so we do not need to free it.
        PPred::ppred_events[this->id]->deschedule();
    }

    /*
     * If the period is not 0, we schedule the event. If this is
     * called with a period that is less than the current tick,
     * then we shift the first dump by curTick. This ensures that
     * we do not schedule the event is the past.
     */
    if (period != 0) {
        // Schedule the event
        if (period >= curTick()) {
            schedPowerPredEvent((Tick)period, (Tick)period, this);
        } else {
            schedPowerPredEvent((Tick)period + curTick(), (Tick)period, this);
        }
    }
}

void
PPredUnit::regStats()
{
    SimObject::regStats();

    lookups
        .name(name() + ".lookups")
        .desc("Number of PP lookups")
        ;
}

void
PPredUnit::predict(void)
{
    DPRINTF(PowerPred, "PPredUnit::predict()\n");
    update();
    action(lookup());
    ++lookups;
}

void
PPredUnit::sendPC(const StaticInstPtr &inst, const InstSeqNum &seqNum,
                 TheISA::PCState pc, ThreadID tid)
{
  this->PC = pc.instAddr();
}

void
PPredUnit::dump()
{
    DPRINTF(PowerPred, "PPredUnit::dump()\n");
}


void
PPredUnit::schedPowerPredEvent(Tick when, Tick repeat, PPredUnit* unit)
{
    // simQuantum is being added to the time when the stats would be
    // dumped so as to ensure that this event happens only after the next
    // sync amongst the event queues.  Asingle event queue simulation
    // should remain unaffected.
    PPred::ppred_events[this->id] = new PowerPredEvent(when + simQuantum, \
                                                        repeat, unit);
}

namespace PPred {

std::vector<GlobalEvent*> ppred_events;

PPredIF interface;

void
updateEvents()
{
    /*
     * If the event has been scheduled, but is scheduled in the past, then
     * we need to shift the event to be at a valid point in time. Therefore, we
     * shift the event by curTick.
     */
    for (size_t i = 0; i < ppred_events.size(); i++) {
        if (ppred_events[i] != NULL &&
            (ppred_events[i]->scheduled() \
              && ppred_events[i]->when() < curTick())) {
            // shift by curTick() and reschedule
            Tick _when = ppred_events[i]->when();
            ppred_events[i]->reschedule(_when + curTick());
        }
    }
}

}; // namespace PPred
