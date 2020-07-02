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
    sysClkDomain(params->sys_clk_domain),
    min_current((double)params->min_current),
    max_current((double)params->max_current),
    cycle_period(params->cycle_period),
    emergency(params->emergency),
    emergency_duration(params->emergency_duration),
    period(params->period),
    delta(params->delta),
    emergency_throttle(params->emergency_throttle),
    voltage_set(params->voltage_set),
    clk(params->clk),
    clk_half(params->clk/2)
{
    DPRINTF(PowerPred, "PPredUnit::PPredUnit()\n");
    supply_voltage = 0.0;
    supply_current = 0.0;
    core_runtime_current = 0.0;
    total_core_runtime_current = 0.0;
    pct_total_runtime_current = 0.0;

    period_normal = this->clockPeriod();
    period_half = this->clockPeriod()*2;

    stall = false;
    this->id = params->cpu_id;
    vpi_shm::set_voltage_set(voltage_set, id);
    vpi_shm::set_core_freq(clk, id);
    history.resize(params->signature_length);
    hr_updated = false;
}

void
PPredUnit::regStats()
{
    SimObject::regStats();

    stat_freq
        .name(name() + ".frequency")
        .desc("Frequency set by Power Predictor")
        ;
    stat_ticks
        .name(name() + ".ticks")
        .desc("Num Ticks")
        ;
    stat_ttn
        .name(name() + ".ttn")
        .desc("Time to next tick in (ps)")
        ;
    stat_stall
        .name(name() + ".stall")
        .desc("PPred Issue a stall?")
        ;
    sv
        .name(name() + ".supply_voltage")
        .desc("Supply Voltage")
        .precision(6)
        ;
    sc
        .name(name() + ".supply_current")
        .desc("Supply Current")
        .precision(6)
        ;
    rtc
        .name(name() + ".core_runtime_current")
        .desc("Core Runtime Current")
        .precision(6)
        ;
    trtc
        .name(name() + ".total_runtime_current")
        .desc("Total Runtime Current")
        .precision(6)
        ;
    ptrtc
        .name(name() + ".pct_total_runtime_current")
        .desc("Percent Total Runtime Current")
        .precision(6)
        ;
}

void
PPredUnit::clkThrottle()
{
    // Set the CPU Clock Object to Half Freq
    sysClkDomain->clockPeriod(period_half);
    vpi_shm::set_core_freq(clk_half, id);
    stat_freq = clk_half;
}

void
PPredUnit::clkRestore()
{
    // Set the CPU Clock Object to Normal
    sysClkDomain->clockPeriod(period_normal);
    vpi_shm::set_core_freq(clk, id);
    stat_freq = clk;
}

void
PPredUnit::setStall()
{
    stall = true;
    stat_stall = false;
}

void
PPredUnit::unsetStall()
{
    stall = false;
    stat_stall = false;
}

void
PPredUnit::get_analog_stats() {
  supply_voltage = Stats::pythonGetVoltage();
  supply_current = Stats::pythonGetCurrent();
  core_runtime_current = Stats::pythonCoreCurrent(id);
  total_core_runtime_current = Stats::pythonTotalCurrent();
  pct_total_runtime_current = core_runtime_current\
      /total_core_runtime_current;
  sv = supply_voltage;
  sc = supply_current;
  rtc = core_runtime_current;
  trtc = total_core_runtime_current;
  ptrtc = pct_total_runtime_current;
}

void
PPredUnit::historyInsert(const PPred::event_t event) {
  hr_updated = history.add_event(event);
}

void
PPredUnit::historySetPC(const uint64_t pc) {
  history.set_pc(pc);
}
