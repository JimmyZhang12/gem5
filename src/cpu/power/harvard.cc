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
//no decor no throttle


#include "cpu/power/harvard.hh"

#include <algorithm>
#include <cassert>
#include <cstdlib>

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "debug/HarvardPowerPred.hh"
#include "python/pybind11/vpi_shm.h"
#include "sim/stat_control.hh"

Harvard::Harvard(const Params *params): PPredUnit(params)
{
    DPRINTF(HarvardPowerPred,
            "Harvard::Harvard()\n");
    state = NORMAL;
    next_state = NORMAL;
    //cam + bloom
    //table.resize(params->table_size, params->signature_length, 3, params->bloom_filter_size);
    
    //cam only
    table.resize(params->table_size, params->signature_length);

    history.resize(params->signature_length);
    throttle_duration = params->duration;
    throttle_on_restore = params->throttle_on_restore;
    hysteresis = params->hysteresis;
    t_count = 0;
    e_count = 0;
    num_ve = 0;
    total_misspred = 0;
    total_preds = 0;
    total_pred_action = 0;
    total_pred_inaction = 0;

    test_counter = 0;
    entry_length = params->signature_length;
    table_length = params->table_size;
    //table_dump = table.getTable();
    last_insert_index_prev_Stats = -1;
    
}

void
Harvard::regStats()
{
    PPredUnit::regStats();

    s
        .name(name() + ".state")
        .desc("Current State of the Predictor (STALLING OFF, THROTTLING ON)")
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
    counter
        .name(name() + ".counter")
        .desc("Simple tick counter")
        ;
    table_entry_insert_Stats
        .init(entry_length)
        .name(name() + ".inserted_New_Entry")
        .desc("the current history")
        ;
    table_entry_insert_prev_Stats
        .init(entry_length)
        .name(name() + ".inserted_New_Entry_prev")
        .desc("the previous cycle history")
        ;
    // table_dump_Stats
    //     .init(entry_length * table_length)
    //     .name(name() + ".table_dump")
    //     .desc("HARVARD cam table")
    //     ;
    last_find_index_Stats
        .name(name() + ".last_find_index")
        .desc("Last found index in pred table")
        ;
    last_insert_index_Stats
        .name(name() + ".last_insert_index")
        .desc("Last insert index in pred table")
        ;
    last_insert_index_prev_Stats
        .name(name() + ".last_insert_index_prev")
        .desc("prev cycle insert index in pred table")
        ;
    hr_anchorPC
        .name(name() + ".anchorPC")
        .desc("Anchor pc of Hist Reg")
        ;
        
  }

void
Harvard::tick(void)
{
  DPRINTF(HarvardPowerPred, "Harvard::tick()\n");
  get_analog_stats();
  table.tick();
  
  test_counter++;
  new_insert = false;
  // Transition Logic
  switch(state) {
    case NORMAL : {
      next_state = NORMAL;
      if (supply_voltage < emergency) {
        next_state = EMERGENCY;
        num_ve++;
        new_insert = true;
        table.insert(this->history.get_entry());
      }
      // If hr updated:
      if (hr_updated) {
        if (table.find(this->history.get_entry())) {
          total_pred_action++;
        }
        else {
          total_pred_inaction++;
        }
        hr_updated = false;
        total_preds++;
      }  
      break;
    }
    case EMERGENCY : {
      // DECOR Rollback
      next_state = EMERGENCY;
      //if (e_count == 0) {
      //  num_ve++;
      //}
      if (t_count == 0) {
        total_misspred++;
      }
      if (hr_updated) {
        if (table.find(this->history.get_entry()))
          total_pred_action++;
        else 
          total_pred_inaction++;
        
        hr_updated = false;
        total_preds++;
      } 
      // if (e_count > emergency_duration &&
      //    supply_voltage > emergency + hysteresis) {
      //   if (throttle_on_restore) {
      //     next_state = THROTTLE;
      //   }
      //   else {
      //     next_state = NORMAL;
      //   }
      // }
      if (supply_voltage > emergency + hysteresis){
        next_state = NORMAL;
      }
      break;
    }
    case THROTTLE : {
      // Pre-emptive Throttle
      next_state = THROTTLE;
      if (supply_voltage < emergency) {
        next_state = EMERGENCY;
        table.insert(this->history.get_entry());
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
      // clkThrottle();
      // setStall();
      clkRestore();
      unsetStall();
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

  // Set Permanant Stats:
  nve = num_ve;
  tmp = total_misspred;
  tpred = total_preds;
  taction = total_pred_action;
  tiaction = total_pred_inaction;

  if (total_preds != 0) {
    mp_rate = (double)total_misspred/(double)total_preds;
  }

  //cycle counter
  counter = test_counter;

  //insertion index stat
  last_insert_index_prev_Stats = last_insert_index;
  if (new_insert)
    last_insert_index = table.last_insert_index;
  else
    last_insert_index = -1;
  last_insert_index_Stats = last_insert_index;

  //predictino hit index stat
  last_find_index_Stats = table.last_find_index;

  //history register stat
  for(int e=0; e < entry_vector.size(); e++){
    table_entry_insert_prev_Stats[e] = entry_vector[e];
  }
  entry_vector = this->history.get_entry().get_history();
  for(int e=0; e < entry_vector.size(); e++){
    table_entry_insert_Stats[e] = entry_vector[e];
  }

  // //table dump stat
  // table_dump = table.getTable();
  // for(int ind=0; ind < table_dump.size(); ind++){
  //   table_vector = table_dump[ind].get_history();
  //   for(int e=0; e < table_vector.size(); e++){
  //     table_dump_Stats[ind*table_vector.size() + e] = table_vector[e];
  //   }
  // }

  //anchor pc of history register
  hr_anchorPC = this->history.get_pc();

  return;
}

Harvard*
HarvardPowerPredictorParams::create(){
  return new Harvard(this);
}


