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

#include "cpu/power/perceptron_predictor_uta.hh"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "cpu/power/ml/utility.h"
#include "debug/PerceptronUTAPowerPred.hh"
#include "python/pybind11/vpi_shm.h"
#include "sim/stat_control.hh"

PerceptronPredictorUTA::PerceptronPredictorUTA(const Params *params)
    : PPredUnit(params)
{
    DPRINTF(PerceptronUTAPowerPred,
            "PerceptronPredictorUTA::PerceptronPredictorUTA()\n");
    state = NORMAL;
    next_state = NORMAL;
    t_count = 0;
    e_count = 0;
    num_ve = 0;
    total_misspred = 0;
    total_preds = 0;
    total_pred_action = 0;
    total_pred_inaction = 0;
    eta = params->eta;
    events = params->events;
    throttle_duration = params->duration;
    hysteresis = params->hysteresis;
    table.resize(params->table_size, \
        Array2D(1, params->events, 1.0));
    previous_idx = 0;
    pc = 0;
    e = Array2D(1, params->events, 0.0);
    previous_e = Array2D(1, params->events, 0.0);
}

void
PerceptronPredictorUTA::regStats()
{
    PPredUnit::regStats();

    s
        .name(name() + ".state")
        .desc("Current State of the Predictor")
        ;
    ns
        .name(name() + ".next_state")
        .desc("Next State of the Predictor")
        ;
    nve
        .name(name() + ".num_voltage_emergency")
        .desc("Num Voltage Emergencies")
        ;
    tmp
        .name(name() + ".total_mispred")
        .desc("Total Misprediction")
        ;
    tpred
        .name(name() + ".total_predictions")
        .desc("Total Predictions")
        ;
    taction
        .name(name() + ".total_action")
        .desc("Total Actions Taken")
        ;
    tiaction
        .name(name() + ".total_inaction")
        .desc("Total No Actions Taken")
        ;
    mp_rate
        .name(name() + ".mispred_rate")
        .desc("Misprediction Rate")
        .precision(6)
        ;
}

void
PerceptronPredictorUTA::tick(void)
{
  //DPRINTF(PerceptronUTAPowerPred, "PerceptronPredictorUTA::tick()\n");
  get_analog_stats();
  double prediction = 0.0;

  // Transition Logic
  switch(state) {
    case NORMAL : {
      next_state = NORMAL;
      if (supply_voltage < emergency) {
        DPRINTF(PerceptronUTAPowerPred, "Emergency Occured");
        emergency_occured = true;
        next_state = EMERGENCY;
      }
      // If hr updated:
      if (hr_updated) {
        // Signal we need to train
        train = true;
        // Update Old PC:
        previous_idx = pc;
        // Get PC from HR
        pc = history.get_pc();
        // Store old e
        previous_e = e;
        // Get array2d from HR with no pc
        e = history.get_array2d(events, true);
        // Compute "hash"
        pc = pc % table.size();

        // Eval Perceptron
        prediction = (table[pc]*e.transpose()).data[0][0];

        if (prediction > 0.0) {
          pred = true;
          total_pred_action++;
          next_state = THROTTLE;
          DPRINTF(PerceptronUTAPowerPred, "Predict Throttle\n");
        }
        else {
          pred = false;
          total_pred_inaction++;
          DPRINTF(PerceptronUTAPowerPred, "Predict No Throttle\n");
        }
        hr_updated = false;
        total_preds++;
      }
      break;
    }
    case EMERGENCY : {
      // DECOR Rollback
      next_state = EMERGENCY;
      if (e_count == 0) {
        num_ve++;
      }
      if (t_count == 0) {
        total_misspred++;
      }
      if (e_count > emergency_duration &&
         supply_voltage > emergency + hysteresis) {
        if (throttle_on_restore) {
          next_state = THROTTLE;
        }
        else {
          next_state = NORMAL;
        }
      }
      break;
    }
    case THROTTLE : {
      // Pre-emptive Throttle
      next_state = THROTTLE;
      if (supply_voltage < emergency) {
        DPRINTF(PerceptronUTAPowerPred, "Emergency Occured");
        emergency_occured = true;
        next_state = EMERGENCY;
      }
      else if (t_count > throttle_duration &&
               supply_voltage > emergency + hysteresis) {
        next_state = NORMAL;
      }
      break;
    }
    default : {
      break;
    }
  }

  // Output Logic
  switch(state) {
    case NORMAL : {
      t_count = 0;
      e_count = 0;
      // Restore Frequency
      clkRestore();
      unsetStall();
      break;
    }
    case EMERGENCY : {
      e_count += 1;
      t_count = 0;
      clkThrottle();
      setStall();
      break;
    }
    case THROTTLE : {
      t_count += 1;
      e_count = 0;
      clkThrottle();
      unsetStall();
      break;
    }
    default : {
      // Nothing
      break;
    }
  }
  // Update Stats:
  s = state;
  ns = next_state;
  // Update Next State Transition:
  state = next_state;

  // If emergency occured: Train the Perceptron
  if (train) {
    if (emergency_occured) {
      if (pred) {
        // We predicted the emergency and made
        // right prediction but emergency was
        // unavoidable
      }
      else {
        // We made the decition not to
        // throttle and emergency occured,
        // train in positive direction
        //std::cout << table[previous_idx];
        //std::cout << previous_e;
        table[previous_idx] = table[previous_idx] + (previous_e*eta);
        DPRINTF(PerceptronUTAPowerPred, "Train Re-enforce");
      }
    }
    else {
      if (pred) {
        // Cant tell if emergency would have
        // occured without prediction
      }
      else {
        // No emergency and no throttle, train
        // in negative direction
        //std::cout << table[previous_idx];
        //std::cout << previous_e;
        table[previous_idx] = table[previous_idx] - (previous_e*eta);
        DPRINTF(PerceptronUTAPowerPred, "Train Negative");
      }
    }
    train = false;
    emergency_occured = false;
  }

  // Set Permanant Stats:
  nve = num_ve;
  tmp = total_misspred;
  tpred = total_preds;
  taction = total_pred_action;
  tiaction = total_pred_inaction;
  if (total_preds != 0) {
    mp_rate = (double)total_misspred/(double)total_preds;
  }
  return;
}

PerceptronPredictorUTA*
PerceptronPredictorUTAParams::create()
{
  return new PerceptronPredictorUTA(this);
}
