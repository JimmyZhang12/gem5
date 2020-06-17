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
#include <string>

#include "base/statistics.hh"
#include "base/types.hh"
#include "cpu/inst_seq.hh"
#include "cpu/static_inst.hh"
#include "debug/PowerPred.hh"
#include "params/PowerPredictor.hh"
#include "python/pybind11/vpi_shm.h"
#include "sim/clocked_object.hh"
#include "sim/global_event.hh"
#include "sim/sim_object.hh"
#include "sim/stat_control.hh"

namespace PPred {

class PPredIF {
  public:
    uint32_t sim_period;
    uint32_t cycle_period;
    bool stat_event_fired;
    PPredIF() {
      sim_period = 0;
      cycle_period = 0;
      stat_event_fired = false;
    }
};

extern PPredIF interface;

extern std::vector<GlobalEvent*> ppred_events;

void updateEvents();

}; // namespace PPred

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
     * Called by the simulation event queue after a stat event and ticks
     * the predictor
     * @param tid The thread ID
     * @todo Must interface with power supply model to issue appropriate
     * commands.
     */
    void process(void);

    /**
     * Tick: Invoke the Derived Predictor
     * @param tid The thread ID.
     * @param inst_PC The PC to look up.
     * @return Quantized value of the power prediction.
     */
    virtual void tick(void) = 0;

    /**
     * Send the PC to the branch pred unit.
     * @param inst The branch instruction.
     * @param PC The predicted PC is passed back through this parameter.
     * @param tid The thread id.
     */
    void sendPC(const StaticInstPtr &inst, const InstSeqNum &seqNum,
                 TheISA::PCState pc, ThreadID tid);

    void dump();

    void clkThrottle();

    void clkRestore();

    void schedPowerPredEvent(Tick when, Tick repeat, PPredUnit* unit);

  protected:
    Stats::Scalar freq;
    Stats::Scalar ticks;
    Stats::Scalar ttn;

    SrcClockDomain* sysClkDomain;

    double min_current;
    double max_current;
    double supply_voltage;
    double supply_current;
    Addr PC;
    int cycle_period;
    double emergency;
  private:
    int period;
    double delta;
    bool emergency_throttle;
    double voltage_set;
    double clk;
    double clk_half;
    Tick period_normal;
    Tick period_half;
    int id;
};

/**
 * Event to Perform the power prediction
 */
class PowerPredEvent : public GlobalEvent
{
  private:
    Tick repeat;
    PPredUnit* unit;

  public:
    PowerPredEvent(Tick _when, Tick _repeat, PPredUnit* _unit = NULL)
        : GlobalEvent(_when, Power_Event_Pri, AutoDelete),
          repeat(_repeat),
          unit(_unit)
    {
    }

    virtual void
    process()
    {
        DPRINTF(PowerPred, "PowerPredEvent::process() curTick %i\n",
            curTick());
        if (unit != NULL)
        {
            if (Stats::pythonGetProfiling()) {
              repeat = PPred::interface.sim_period;
              if (PPred::interface.stat_event_fired) {
                PPred::interface.stat_event_fired = false;
                unit->process();
              }
            }
            unit->schedPowerPredEvent(curTick() + repeat, repeat, unit);
        }
    }

    const char *description() const
    {
        return "GlobalPowerPredictionEvent";
    }
};

#endif // __CPU_PRED_PPRED_UNIT_HH__
