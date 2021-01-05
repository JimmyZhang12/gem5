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

#ifndef __CPU_POWER_HARVARD_HH__
#define __CPU_POWER_HARVARD_HH__

#include <deque>
#include <string>
#include <vector>

#include "base/statistics.hh"
#include "base/types.hh"
#include "cpu/inst_seq.hh"
#include "cpu/power/history_register.hh"
#include "cpu/power/ppred_unit.hh"
#include "cpu/power/prediction_table.hh"
#include "cpu/power/event_type.hh"
#include "cpu/static_inst.hh"
#include "params/HarvardPowerPredictor.hh"
#include "sim/probe/pmu.hh"
#include "sim/sim_object.hh"

class Harvard : public PPredUnit
{
  public:
    typedef HarvardPowerPredictorParams Params;

    /**
     * @param params The params object, that has the size of the BP and BTB.
     */
    Harvard(const Params *p);

    /**
     * Registers statistics.
     */
    void regStats() override;

    /**
     * Update the Harvard State Machine.
     */
    void tick(void);

  protected:
    /** Hysteresis level */
    double hysteresis;

    /** # Cycles to throttle */
    unsigned int throttle_duration;

    /** Throttle after DeCoR rollback */
    bool throttle_on_restore;

  private:
    enum state_t {
      NORMAL=1,
      THROTTLE,
      EMERGENCY
    };

    PPred::Table table;

    state_t state;
    state_t next_state;

    // Counter for # Cycles to delay
    unsigned int e_count;
    unsigned int t_count;
    Stats::Scalar s;
    Stats::Scalar ns;

    // Permanant Stats:
    // Num Voltage Emergencies
    uint64_t num_ve;
    uint64_t total_misspred;
    uint64_t total_preds;
    uint64_t total_pred_action;
    uint64_t total_pred_inaction;
    Stats::Scalar nve;
    Stats::Scalar tmp;
    Stats::Scalar tpred;
    Stats::Scalar taction;
    Stats::Scalar tiaction;
    Stats::Scalar mp_rate;

    //jimmy stats
    uint64_t test_counter;
    Stats::Scalar counter;

    uint64_t entry_length;
    uint64_t table_length;

    Stats::Scalar hr_anchorPC;

    //std::vector<PPred::event_t> entry_vector;
    //std::vector<PPred::event_t> table_vector;
    // Stats::Vector table_entry_insert_Stats;
    // Stats::Vector table_entry_insert_prev_Stats;

    // std::vector<PPred::Entry> table_dump;
    // Stats::Vector table_dump_Stats;

    bool new_insert;
    int64_t last_insert_index;
    Stats::Scalar last_insert_index_Stats;
    Stats::Scalar last_insert_index_prev_Stats;

    Stats::Scalar last_find_index_Stats;

    //new events
    std::vector<PPred::event_t> new_events;
    Stats::Vector new_events_Stats;
    Stats::Vector new_events_prev_Stats;


};

#endif // __CPU_PRED_HARVARD_HH__