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
#include "sim/global_event.hh"
#include "sim/sim_object.hh"

class PPredUnit : public SimObject
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
    void regStats() override;

    /**
     * Makes a prediction on what the power will be in the next epoch.
     * @param pc The pc to lookup in the predictor
     * @param tid The thread ID
     * @todo Must interface with power supply model to issue appropriate
     * commands.
     */
    void predict(void);

    /**
     * Performs a lookup on the power prediction module based on the current
     * PC.
     * @param tid The thread ID.
     * @param inst_PC The PC to look up.
     * @return Quantized value of the power prediction.
     */
    virtual int lookup(void) = 0;

    /**
     * Based on the feedback from the power supply unit, this function updates
     * the predictor.
     * @todo Must interface with power supply model to read the accuracy of the
     * prediction and size + duration of droop, Keep a log of previous lookups
     * used as a history and then update those too to avoid a signature that
     * leaves a large droop or overshoot.
     */
    virtual void update(void) = 0;

    void dump();

    void schedPowerPredEvent(Tick when, Tick repeat, PPredUnit* unit);

    void updateEvents();

  private:
    /** Stat for number of BP lookups. */
    Stats::Scalar lookups;

    GlobalEvent* powerEvent;
    int period;
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
        : GlobalEvent(_when, Power_Event_Pri, 0),
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
            unit->predict();
            unit->schedPowerPredEvent(curTick() + repeat, repeat, unit);
        }
    }

    const char *description() const
    {
        return "GlobalPowerPredictionEvent";
    }
};

#endif // __CPU_PRED_PPRED_UNIT_HH__
