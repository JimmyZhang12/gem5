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

#ifndef __CPU_POWER_SIMPLE_HISTORY_HH__
#define __CPU_POWER_SIMPLE_HISTORY_HH__

#include <deque>
#include <string>
#include <unordered_map>

#include "base/statistics.hh"
#include "base/types.hh"
#include "cpu/inst_seq.hh"
#include "cpu/power/ppred_unit.hh"
#include "cpu/static_inst.hh"
#include "params/SimpleHistoryPowerPredictor.hh"
#include "sim/probe/pmu.hh"
#include "sim/sim_object.hh"

class SimpleHistory : public PPredUnit
{
  public:
    typedef SimpleHistoryPowerPredictorParams Params;

    /**
     * @param params The params object, that has the size of the BP and BTB.
     */
    SimpleHistory(const Params *p);

    /**
     * Registers statistics.
     */
    void regStats() override;

    /**
     * Performs a lookup on the power prediction module based on the current
     * PC.
     * @param tid The thread ID.
     * @param inst_PC The PC to look up.
     * @return Quantized value of the power prediction.
     */
    int lookup(void);

    /**
     * Based on the feedback from the power supply unit, this function updates
     * the predictor.
     * @todo Must interface with power supply model to read the accuracy of the
     * prediction and size + duration of droop, Keep a log of previous lookups
     * used as a history and then update those too to avoid a signature that
     * leaves a large droop or overshoot.
     */
    void update(void);

    /**
     * The action taken by the SimpleHistory predictor is none.
     * @param lookup_val The value returned by the lookup method
     */
    void action(int lookup_val);

  protected:
    unsigned int num_entries;
    unsigned int nbits_pc;
    unsigned int pc_start;
    unsigned int history_size;
    unsigned int quantization_levels;

    // Update:
    uint64_t last_index;

    std::vector<uint8_t> lut;
    std::vector<uint64_t> history;

    //std::unordered_map<uint64_t, uint8_t> pred_table;

  private:
    Stats::Scalar action_taken;
    Stats::Scalar error;
    Stats::Scalar look_up_index;

    unsigned int get_index();
    void push_history();
    unsigned int get_mask();
};

#endif // __CPU_PRED_SIMPLE_HISTORY_HH__
