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


#include "cpu/power/decor_only.hh"

#include <algorithm>
#include <cassert>
#include <cstdlib>

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "cpu/power/bloomfilter.h"
#include "debug/DecorOnlyPowerPred.hh"
#include "python/pybind11/vpi_shm.h"
#include "sim/stat_control.hh"

DecorOnly::DecorOnly(const Params *params)
    : PPredUnit(params),
    throttle_on_restore(params->throttle_on_restore),
    throttle_duration(params->duration)
{
    DPRINTF(DecorOnlyPowerPred, "DecorOnly::DecorOnly()\n");
    e_count = 0;
    t_count = 0;
    state = NORMAL;
    next_state = NORMAL;
    num_ve = 0;
}

void
DecorOnly::regStats()
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
}

void
DecorOnly::tick(void)
{
  DPRINTF(DecorOnlyPowerPred, "DecorOnly::tick()\n");
  get_analog_stats();

  // Transition Logic
  switch(state) {
    case NORMAL : {
      next_state = NORMAL;
      if (supply_voltage < emergency) {
        num_ve++;
        next_state = EMERGENCY;
      }
      break;
    }
    case EMERGENCY : {
      // DECOR Rollback
      next_state = EMERGENCY;
      if (e_count > emergency_duration &&
         supply_voltage > emergency) {
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
      // Throttle On Restore
      next_state = THROTTLE;
      if (supply_voltage < emergency) {
        num_ve++;
        next_state = EMERGENCY;
      }
      if (t_count > throttle_duration) {
        next_state = NORMAL;
      }
      break;
    }
    default : {
      // Nothing
      //next_state = NORMAL;
      break;
    }
  }

  // Output Logic
  switch(state) {
    case NORMAL : {
      // Restore Frequency
      t_count = 0;
      e_count = 0;
      clkRestore();
      unsetStall();
      break;
    }
    case EMERGENCY : {
      t_count = 0;
      e_count+=1;
      clkThrottle();
      setStall();
      break;
    }
    case THROTTLE : {
      t_count+=1;
      e_count = 0;
      clkThrottle();
      unsetStall();
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
  nve = num_ve;
  return;
}

DecorOnly*
DecorOnlyParams::create()
{
  return new DecorOnly(this);
}

