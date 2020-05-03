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


#include "cpu/power/test.hh"

#include <algorithm>
#include <cassert>
#include <cstdlib>

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "debug/TestPowerPred.hh"
#include "python/pybind11/vpi_shm.h"
#include "sim/stat_control.hh"

Test::Test(const Params *params)
    : PPredUnit(params),
    num_entries((uint64_t)params->num_entries),
    num_correlation_bits((uint8_t)params->num_correlation_bits),
    pc_start(params->pc_start)
{
    DPRINTF(TestPowerPred, "Test::Test()\n");
    lut.resize(num_entries, 63);
    last_index = 0;
}

void
Test::regStats()
{
    PPredUnit::regStats();

    action_taken
        .name(name() + ".action_taken")
        .desc("Number of times the action is taken")
        ;
    error
        .name(name() + ".error")
        .desc("Error between the table and the"
              " supply current")
        ;

}

int
Test::lookup(void)
{
    DPRINTF(TestPowerPred, "Test::lookup()\n");
    uint64_t mask = ~0;
    mask = mask >> num_correlation_bits;
    mask = mask << num_correlation_bits;
    mask = ~mask;
    int index = ((uint64_t)PC >> pc_start) & mask;

    DPRINTF(TestPowerPred, "Test::lookup(): PC = %x;"
        "index = %d; mask = %x; val = %d; \n",
        (uint64_t)PC, index, mask, lut[index]);

    last_index = index;

    return lut[index];
}

void
Test::update(void)
{
    DPRINTF(TestPowerPred, "Test::update()\n");
    double supply_voltage = Stats::pythonGetVoltage();
    double supply_current = Stats::pythonGetCurrent();

    uint8_t observed = (uint8_t)((supply_current/max_current)*255.0);
    DPRINTF(TestPowerPred, "Test::update(): current = %d;"
        "voltage = %d; qp = %d;\n",
        supply_current, supply_voltage, observed);

    if (lut[last_index] != observed)
    {
        error = abs(lut[last_index] - observed);
        lut[last_index] = observed;
    }
    DPRINTF(TestPowerPred, "Test::update(): val = %d;\n", lut[last_index]);
}

void
Test::action(int lookup_val)
{
    double prediction = (double)(max_current)*(double)lookup_val/255.0;
    vpi_shm::set_prediction(prediction);
    DPRINTF(TestPowerPred, "Test::action(), "
        "Prediction: %lf[A]; lookup_val = %d;\n", prediction, lookup_val);
}

Test*
TestPowerPredictorParams::create()
{
    return new Test(this);
}
