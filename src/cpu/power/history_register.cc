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

#include "cpu/power/history_register.hh"

#include <iostream>

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "debug/HistoryRegister.hh"
#include "python/pybind11/vpi_shm.h"
#include "sim/stat_control.hh"

/**
 * Default Constructor
 */
PPred::HistoryRegister::HistoryRegister(size_t len) {
  this->signature.resize(len, BRANCH_T);
  this->pc.resize(len, 0);
}

/**
 * Convert the History Register to an Event type
 * @return Entry type that can be hashed or looked up in a CAM
 */
PPred::Entry PPred::HistoryRegister::get_entry() {
  return PPred::Entry(this->pc[0], this->signature);
}

/**
 * Convert the History Register to an Array2D type that can be used in the
 * perceptron and DNN. Returns a 1x(2*len(EHR)) array.
 * @return Array2D
 */
Array2D PPred::HistoryRegister::get_array2d() {
  Array2D ret = Array2D(1,pc.size()+signature.size(), 0.0);
  for (size_t i = 0; i < pc.size(); i++) {
    ret.data[0][i] = (double)pc[i];
    ret.data[0][i+pc.size()] = (double)(int)signature[i];
  }
  return ret;
}

/**
 * Add Event
 * Adds Event to the HistoryRegister; takes the internal updating PC
 * register, and external event and adds to the PC/Event registers. This is
 * so events that dont have reference to a PC can also insert events, such as
 * the memory hierarchy.
 * @param event uArch Event Type from the event_t enum
 * @return if the event is added correctly
 */
bool PPred::HistoryRegister::add_event(PPred::event_t event) {
  // First check if the event isnt already in the HR for the same PC
  for (size_t i = 0; i < signature.size(); i++) {
    if (pc[i] != inst_pc) {
      // Not in the most recent set of PCs, any other match would
      // be from a previous time frame
      break;
    }
    if (signature[i] == event && pc[i] == inst_pc) {
      // Event & PC value are a duplicate
      return false;
    }
  }

  // shift all the vector elements:
  for (int i = (int)signature.size() - 2; i >= 0; i--) {
    this->signature[i+1] = this->signature[i];
  }
  for (int i = (int)pc.size() - 2; i >= 0; i--) {
    this->pc[i+1] = this->pc[i];
  }
  this->signature[0] = event;
  this->pc[0] = this->inst_pc;
  // Print Vector & PC
  DPRINTF(HistoryRegister, "[ HistoryRegister ] add_event(): PC %d; " \
  "Signature", this->pc[0]);
  //for (auto i : this->signature) {
  //  DPRINTF(HistoryRegister, "%s,", PPred::event_t_name[i]);
  //}
  DPRINTF(HistoryRegister, "\n");
  return true;
}

/**
 * Set the internal PC value, called from the cpu.tick() function.
 * @param pc The current pc value at the time of the cpu.tick() function.
 */
void PPred::HistoryRegister::set_pc(uint64_t pc) {
  this->inst_pc = pc;
}
