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

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "debug/TestPowerPred.hh"
#include "python/pybind11/vpi_shm.h"


Test::Test(const Params *params)
    : PPredUnit(params),
    num_entries((uint64_t)params->num_entries),
    num_correlation_bits((uint8_t)params->num_correlation_bits),
    pc_start(params->pc_start)
{
    DPRINTF(TestPowerPred, "Test::Test()\n");

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
    uint64_t last_PC_address = (uint64_t)PC;
    uint8_t ret_val;
    uint64_t mask = ~(~0 >> num_correlation_bits) << num_correlation_bits;
     last_index = (last_PC_address >> pc_start) & mask;

    auto lookup_iter = pred_table.find(last_index);

    if (lookup_iter==pred_table.end()){
        // If index is not found then insert index with default value 63 for
        // 25% current because it is quantized to 8 bits
        pred_table[last_index]=63;
        ret_val = 63;
    }
    else
        ret_val= lookup_iter->second;


    DPRINTF(TestPowerPred, "Test::lookup()\n");

    return ret_val;
}

void
Test::update(void)
{
    //double supply_voltage = vpi_shm::get_voltage();
    double supply_current = vpi_shm::get_current();
    vpi_shm::ack_supply();


    uint8_t quantized_prediction = (uint8_t)(min_current + \
        ((max_current-min_current)*supply_current)/256);

    auto lookup_iter = pred_table.find(last_index);

    if (lookup_iter->second!=quantized_prediction){

        error = quantized_prediction - lookup_iter->second;

        // Update value
        pred_table[last_index]=quantized_prediction;
        //Add stats

    }

    DPRINTF(TestPowerPred, "Test::update()\n");
}

void
Test::action(int lookup_val)
{
    double prediction = (double)(min_current + \
        (max_current-min_current)*lookup_val/255);
    vpi_shm::set_prediction(prediction);
    DPRINTF(TestPowerPred, "Test::action(), "
        "Prediction: %lf[A]\n", prediction);
}

Test*
TestPowerPredictorParams::create()
{
    return new Test(this);
}
