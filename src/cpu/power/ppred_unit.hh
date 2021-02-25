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
 * * Copyright (c) 2004-2005 The Regents of The University of Michigan
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

#ifndef __CPU_POWER_PPRED_UNIT_HH__
#define __CPU_POWER_PPRED_UNIT_HH__

#include <deque>
#include <list>
#include <string>

#include "base/statistics.hh"
#include "base/types.hh"

#include "cpu/inst_seq.hh"
#include "cpu/power/history_register.hh"
#include "cpu/static_inst.hh"

#include "debug/PowerPred.hh"
#include "params/PowerPredictor.hh"
#include "python/pybind11/vpi_shm.h"

#include "sim/clocked_object.hh"
#include "sim/global_event.hh"
#include "sim/sim_object.hh"
#include "sim/stat_control.hh"

//forward declare
class PPredStat;

class PPredUnit : public ClockedObject
{
  public:
    typedef PowerPredictorParams Params;
    /**
     * @param params The params object, that has the size of the BP and BTB.
     */
    PPredUnit(const Params *p);

    /**
     * Registers statistics.
     */
    virtual void regStats() override;

    /**
     * Tick: Invoke the Derived Predictor
     */
    virtual void tick(void) = 0;

    /**
     * Old Throttleing interface
     */
    void clkThrottle();

    void clkRestore();

    /**
     * Take an action from the LUT
     * Not nescescarily a Throttle Action
     */
    void takeAction(size_t idx = 0);

    void setStall();

    void unsetStall();

    void historyInsert(const PPred::event_t event);

    void historySetPC(const uint64_t pc);

    bool get_stall() const {
      return stall;
    }

    int get_cycle_period() const {
      return cycle_period;
    }

    /**
     * setNumInstrsPending
     * Used by the IEW or IQ to set the number of instructions that can be
     * executed.
     */
    void setNumInstrsPending(const uint64_t inst);

    /**
     * setCPUStalled
     * Set by the CPU if it is stalled.
     */
    void setCPUStalled(const bool stalled);

    /**
    * Update the various stats
    */
    void update_stats(bool pred, bool ve);

  protected:
    Stats::Scalar stat_freq;
    Stats::Scalar stat_ticks;
    Stats::Scalar stat_ttn;
    Stats::Scalar stat_stall;
    Stats::Scalar stat_insts_available;
    Stats::Scalar stat_decode_idle;
    //analog stats
    Stats::Scalar sv;
    Stats::Scalar sv_p;
    Stats::Scalar sc;
    //prediction stats
    Stats::Vector event_count;
    Stats::Vector hits;
    Stats::Vector false_pos;

    Stats::Scalar ves_outside_leadtime;
    Stats::Scalar preds_outside_leadtime;
    Stats::Scalar _total_ve;
    Stats::Scalar _total_action;
    Stats::Formula overall_hit_rate;
    Stats::Formula overall_fp_rate;


    SrcClockDomain* sysClkDomain;
    double min_current;
    double max_current;

    double supply_voltage;
    double supply_current;
    double supply_voltage_prev;


    // Number of Pending Instructions
    uint64_t pendingInstructions;

    // Is Stalled
    bool cpuStalled;

    uint64_t PC;
    const int cycle_period;
    double emergency;
    double emergency_duration;
    int id;
    PPred::HistoryRegister history;
    bool hr_updated;

    uint64_t get_num_actions() {
      return clk_vals.size();
    }

  private:
    int period;
    double delta;
    bool emergency_throttle;
    double voltage_set;
    double clk;
    double clk_half;
    std::vector<double> clk_vals;
    std::vector<Tick> period_vals;
    Tick period_normal;
    Tick period_half;
    bool stall;

    /*stats variables*/

    const int LEAD_TIME_CAP;

    int total_ve;
    int total_action;
    int cycles_since_pred;
    int cycles_since_ve;
    std::list<bool> action;



    /**
     * Rescale from range [a0, a1] -> [b0, b1]
     */
    double rescale(double i,
                   double a0,
                   double a1,
                   double b0,
                   double b1);
};

#endif // __CPU_PRED_PPRED_UNIT_HH__
