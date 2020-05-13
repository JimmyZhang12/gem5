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


#include "cpu/power/simple_history.hh"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "debug/SimpleHistoryPowerPred.hh"
#include "python/pybind11/vpi_shm.h"
#include "sim/stat_control.hh"

SimpleHistory::SimpleHistory(const Params *params)
    : PPredUnit(params),
    num_entries((uint64_t)params->num_entries),
    nbits_pc((uint8_t)params->nbits_pc),
    pc_start(params->pc_start),
    history_size(params->history_size),
    quantization_levels(params->quantization_levels),
    confidence_level(params->confidence_level),
    limit(params->limit)
{
    DPRINTF(SimpleHistoryPowerPred, "SimpleHistory::SimpleHistory()\n");
    last_index = 0;
    assert((quantization_levels & (quantization_levels - 1)) == 0 &&
        quantization_levels != 0);
    assert((num_entries & (num_entries - 1)) == 0 &&
        num_entries != 0);
    assert(pow(2, nbits_pc*(history_size+1)) <= num_entries);
    lut.resize(num_entries, (quantization_levels--)/4);
    history.resize(history_size, 0);
    error_array.resize(10, params->quantization_levels);
    ea_idx = 0;
}

void
SimpleHistory::regStats()
{
    PPredUnit::regStats();

    action_taken
        .name(name() + ".action_taken")
        .desc("Number of times the action is taken")
        ;
    error
        .name(name() + ".error")
        .desc("Prediction Error")
        ;
    rolling_error
        .name(name() + ".rolling_error")
        .desc("Rolling average of the prediction error")
        ;
    look_up_index
        .name(name() + ".index")
        .desc("Index used to lookup in the table")
        ;
}

int
SimpleHistory::lookup(void)
{
    DPRINTF(SimpleHistoryPowerPred, "SimpleHistory::lookup()\n");
    int index = get_index();

    assert(index < lut.size());

    DPRINTF(SimpleHistoryPowerPred, "SimpleHistory::lookup(): PC = %x;"
        "index = %d; val = %d; \n", (uint64_t)PC, index, lut[index]);

    last_index = index;
    look_up_index = index;

    return lut[index];
}

void
SimpleHistory::update(void)
{
    DPRINTF(SimpleHistoryPowerPred, "SimpleHistory::update()\n");
    supply_voltage = Stats::pythonGetVoltage();
    supply_current = Stats::pythonGetCurrent();
    bool enable = Stats::pythonGetProfiling();

    uint8_t observed = (uint8_t)((supply_current/max_current)
                       *quantization_levels);
    DPRINTF(SimpleHistoryPowerPred, "SimpleHistory::update(): current = %d;"
        "voltage = %d; qp = %d;\n",
        supply_current, supply_voltage, observed);

    uint diff = abs(lut[last_index] - observed);
    if (enable)
    {
        if (lut[last_index] - observed > 0)
        {
            lut[last_index] -= (uint8_t)(diff*0.75);
        }
        else
        {
            lut[last_index] += (uint8_t)(diff*0.75);
        }
        add_error(diff);
        error = diff;
    }
    DPRINTF(SimpleHistoryPowerPred, "SimpleHistory::update(): val = %d;\n",
        lut[last_index]);
}

void
SimpleHistory::action(int lookup_val)
{
    double p = ((double)(max_current)* \
        (double)lookup_val/(double)quantization_levels) - supply_current;
    bool enable = Stats::pythonGetProfiling();
    double avg = average_error();
    rolling_error = avg;
    if (p > limit)
    {
        p = limit;
    }
    if (p < -limit)
    {
        p = -limit;
    }
    if (enable && (avg < confidence_level))
    {
        vpi_shm::set_prediction(p);
    }
    DPRINTF(SimpleHistoryPowerPred, "SimpleHistory::action(), "
        "p = %lf[A]; supply_current = %lf[A]; "
        "lookup_val = %d; enable = %d; avg_err = %d\n",
        p, supply_current, lookup_val, enable, avg);
}

unsigned int
SimpleHistory::get_index()
{
    unsigned int index = 0;
    unsigned int mask = get_mask();

    for (size_t i = 0; i < history.size(); i++)
    {
        index |= ((history[i] & mask) << i*nbits_pc);
    }
    index |= (PC & mask) << history.size()*nbits_pc;
    return index;
}

void
SimpleHistory::push_history()
{
    for (size_t i = history.size()-1; i>0; i--){
        history[i]=history[i-1];
    }
    history[0]=PC;
}

unsigned int
SimpleHistory::get_mask()
{
    uint64_t mask = ~0;
    mask = mask >> nbits_pc;
    mask = mask << nbits_pc;
    mask = ~mask;
    return mask;
}

double
SimpleHistory::average_error() {
    double avg = 0.0;
    for (auto it = error_array.begin(); it != error_array.end(); it++) {
        avg += (double)*it;
    }
    return (avg/((double)error_array.size()))/(double)quantization_levels;
}

void
SimpleHistory::add_error(unsigned int e) {
    error_array[ea_idx] = e;
    ea_idx = (ea_idx + 1)%error_array.size();
}

SimpleHistory*
SimpleHistoryPowerPredictorParams::create()
{
    return new SimpleHistory(this);
}
