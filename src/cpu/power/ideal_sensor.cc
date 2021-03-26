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


#include "cpu/power/ideal_sensor.hh"

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "python/pybind11/vpi_shm.h"
#include "sim/stat_control.hh"
#include "debug/IdealSensor.hh"


IdealSensor::IdealSensor(const Params *params): 
  PPredUnit(params),
  threshold(params->threshold),
  voltage_min(params->voltage_min),
  voltage_max(params->voltage_max),
  num_buckets(params->num_buckets)
{
  bucket_len = (voltage_max - voltage_min) / num_buckets;
  voltage_history.resize(params->history_len);
}

void
IdealSensor::regStats(){
    PPredUnit::regStats();
}

int
IdealSensor::voltage_to_bucket(float voltage){
  return (int) ((voltage - voltage_min) / bucket_len);
}

void
IdealSensor::tick(void){
  bool ve = false;
  bool prediction = false;
  voltage_history.push_front(voltage_to_bucket(supply_voltage));
  voltage_history.pop_back();

  set_vhistory::const_iterator got = voltage_history_table.find(voltage_history);

  if (got != voltage_history_table.end()){
    prediction = true; 
  }

  // if (supply_voltage < threshold && supply_voltage_prev > threshold){
  //   prediction = true; 
  // } 

  if (supply_voltage < emergency && supply_voltage_prev > emergency){
    ve = true;
  } 
  update_stats(prediction, ve);

  if (is_ve_missed()){
    voltage_history_table.insert(voltage_history);
  }

}

IdealSensor*
IdealSensorParams::create(){
  return new IdealSensor(this);
}


