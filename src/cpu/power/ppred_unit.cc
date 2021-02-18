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
#include <list>

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
    clk_half(params->clk/2),
    LEAD_TIME_CAP(params->lead_time)
{
    DPRINTF(PowerPred, "PPredUnit::PPredUnit()\n");
    supply_voltage = 0.0;
    supply_current = 0.0;

    period_normal = this->clockPeriod();
    period_half = this->clockPeriod()*2;

    stall = false;
    this->id = params->cpu_id;

    history.resize(params->signature_length);
    hr_updated = false;
    clk_vals.resize(params->action_length);
    period_vals.resize(params->action_length);
    for (size_t i = 0; i < clk_vals.size(); i++) {
        clk_vals[i] = rescale(((double)i/(double)params->action_length), \
            0.0, 1.0, clk, clk_half);
        period_vals[i] = (Tick)(int)rescale(((double)i/ \
            (double)params->action_length),
            0.0, 1.0, period_normal, period_half);
    }

    /*****Stats stuff*******/
    total_action = 0;
    total_ve = 0;
    cycles_since_pred = 0;
    cycles_since_ve = 0;

    action.resize(LEAD_TIME_CAP);
    for (auto &x:action){
         x= false;
    }

  
}

void
PPredUnit::regStats()
{
    using namespace Stats;

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
    stat_decode_idle
        .name(name() + ".decode_idle")
        .desc("Decode Idle")
        ;
    stat_insts_available
        .name(name() + ".insts_available")
        .desc("Instructions Available")
        ;
    sv
        .name(name() + ".supply_voltage")
        .desc("Supply Voltage")
        .precision(6)
        ;
    sv_p
        .name(name() + ".supply_voltage_prev")
        .desc("Previous cycle supply voltage")
        .precision(6)
        ;
    sc
        .name(name() + ".supply_current")
        .desc("Supply Current")
        .precision(6)
        ;
    hits
        .init(LEAD_TIME_CAP)
        .name(name() + "hit rate")
        .desc("Noncumulative hit rate over lead time")
        .precision(3)
        .flags(total)
        ;
    false_pos
        .init(LEAD_TIME_CAP)
        .name(name() + "false_pos rate")
        .desc("Noncumulative false positive rate over lead time")
        .precision(3)
        .flags(total)
        ;
    ves_outside_leadtime
        .name(name() + ".hits_outside_leadtime")
        .desc("uncaught hits at lead time cap")
        .precision(6)
        ;
    preds_outside_leadtime
        .name(name() + ".hits_outside_leadtime")
        .desc("uncaught preds at lead time cap")
        .precision(6)
        ;

    _total_action
        .name(name() + ".total_actions")
        .desc("total number for prediction highs")
        .precision(6)
        ;
    _total_ve
        .name(name() + ".total_ve")
        .desc("total number of VEs")
        .precision(6)
        ;
    overall_hit_rate
        .name(name() + ".overall_hit_rate")
        .desc("hit rate at lead time cap")
        .precision(6)
        ;
    overall_fp_rate
        .name(name() + ".overall_fp_rate")
        .desc("false positive rate at lead time cap")
        .precision(6)
        ;

}

void
PPredUnit::clkThrottle()
{
    // Set the CPU Clock Object to Half Freq
    sysClkDomain->clockPeriod(period_half);
    stat_freq = clk_half;
}

void
PPredUnit::clkRestore()
{
    // Set the CPU Clock Object to Normal
    sysClkDomain->clockPeriod(period_normal);
    stat_freq = clk;
}

void
PPredUnit::takeAction(size_t idx) {
    // Set the CPU Clock Object to Normal
    sysClkDomain->clockPeriod(period_vals[idx]);
    stat_freq = clk_vals[idx];
}

double
PPredUnit::rescale(double i, double a0, double a1, double b0, double b1) {
  double d0 = a1 - a0;
  double d1 = b1 - b0;
  return (d1*(i-a0)/d0)+b0;
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
PPredUnit::historyInsert(const PPred::event_t event) {
    hr_updated = history.add_event(event);
}

void
PPredUnit::historySetPC(const uint64_t pc) {
  history.set_pc(pc);
}

void
PPredUnit::setNumInstrsPending(const uint64_t inst) {
  pendingInstructions = inst;
  stat_insts_available = pendingInstructions;
}

void
PPredUnit::setCPUStalled(const bool stalled) {
  cpuStalled = stalled;
  stat_decode_idle = cpuStalled;
}

void
PPredUnit::update_stats(bool pred, bool ve){
    action.push_front(pred);
    action.pop_back();

    if (pred){
        total_action++;
        _total_action = total_action;

        cycles_since_pred = 0;
    }
    else{
        cycles_since_pred++;
    }

    if (ve){
        total_ve++;
        _total_ve = total_ve;


        cycles_since_ve = 0;
        if (cycles_since_pred >= LEAD_TIME_CAP)
            ves_outside_leadtime++;
        else
            hits[cycles_since_pred] += 1;
    }
    else{
        cycles_since_ve++;
    }

    if (action.back()){
        if (cycles_since_ve >= LEAD_TIME_CAP)
            preds_outside_leadtime++;
        else
            false_pos[LEAD_TIME_CAP-cycles_since_ve-1] += 1;
    }

    //stats
    supply_voltage_prev = supply_voltage;
    supply_voltage = PPredStat::voltage;
    supply_current = PPredStat::current;
    overall_hit_rate = ves_outside_leadtime.value() / total_ve;
    overall_fp_rate = preds_outside_leadtime.value() / total_action;
    sv = supply_voltage;
    sv_p = supply_voltage_prev;
    sc = supply_current;


}
