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


#include "cpu/power/simple.hh"

#include <algorithm>
#include <cassert>
#include <cstdlib>

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "debug/SimplePowerPred.hh"
#include "python/pybind11/vpi_shm.h"
#include "sim/stat_control.hh"

Simple::Simple(const Params *params)
    : PPredUnit(params),
    num_entries((uint64_t)params->num_entries),
    num_correlation_bits((uint8_t)params->num_correlation_bits),
    pc_start(params->pc_start),
    quantization_levels(params->quantization_levels),
    confidence_level(params->confidence_level),
    limit(params->limit)
{
    DPRINTF(SimplePowerPred, "Simple::Simple()\n");
    lut.resize(num_entries, 63);
    error_array.resize(params->error_array_size,
        params->quantization_levels);
    ea_idx = 0;
    last_index = 0;
}

void
Simple::regStats()
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

void
Simple::tick(void) {
    DPRINTF(SimplePowerPred, "Simple::tick()\n");
    update();
    action(lookup());
}

int
Simple::lookup(void)
{
    DPRINTF(SimplePowerPred, "Simple::lookup()\n");
    uint64_t mask = ~0;
    mask = mask >> num_correlation_bits;
    mask = mask << num_correlation_bits;
    mask = ~mask;
    int index = ((uint64_t)PC >> pc_start) & mask;

    DPRINTF(SimplePowerPred, "Simple::lookup(): PC = %x;"
        "index = %d; mask = %x; val = %d; \n",
        (uint64_t)PC, index, mask, lut[index]);

    last_index = index;
    look_up_index = index;

    return lut[index];
}

void
Simple::update(void)
{
    DPRINTF(SimplePowerPred, "Simple::update()\n");
    supply_voltage = Stats::pythonGetVoltage();
    supply_current = Stats::pythonGetCurrent();
    bool enable = Stats::pythonGetProfiling();
    double avg = average_error();

    uint8_t observed = (uint8_t)((supply_current/max_current)*255.0);
    DPRINTF(SimplePowerPred, "Simple::update(): current = %d;"
        "voltage = %d; qp = %d;\n",
        supply_current, supply_voltage, observed);

    uint diff = abs(lut[last_index] - observed);
    if (enable && avg >= confidence_level)
    {
        if (lut[last_index] - observed > 0)
        {
            lut[last_index] -= (uint8_t)(diff*0.75);
        }
        else
        {
            lut[last_index] += (uint8_t)(diff*0.75);
        }
    }
    if (enable)
    {
        add_error(diff);
        error = diff;
    }
    DPRINTF(SimplePowerPred, "Simple::update(): val = %d;\n", lut[last_index]);
}

void
Simple::action(int lookup_val)
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
    DPRINTF(SimplePowerPred, "Simple::action(), "
        "Prediction: %lf[A]; lookup_val = %d; enable = %d; average_error\n",
        p, lookup_val, enable, avg);
}

double
Simple::average_error() {
    double avg = 0.0;
    for (auto it = error_array.begin(); it != error_array.end(); it++) {
        avg += (double)*it;
    }
    return (avg/((double)error_array.size()))/(double)quantization_levels;
}

void
Simple::add_error(unsigned int e) {
    error_array[ea_idx] = e;
    ea_idx = (ea_idx + 1)%error_array.size();
}

Simple*
SimplePowerPredictorParams::create()
{
    return new Simple(this);
}

