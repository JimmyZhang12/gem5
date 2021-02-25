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

#include <cmath>
#include <iostream>

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "cpu/power/ml/func.h"
#include "debug/HistoryRegister.hh"
#include "python/pybind11/vpi_shm.h"
#include "sim/stat_control.hh"

/**
 * Default Constructor
 */
PPred::HistoryRegister::HistoryRegister(size_t len) {
  this->signature.resize(len, BRANCH_T);
}

/**
 * Convert the History Register to an Event type
 * @return Entry type that can be hashed or looked up in a CAM
 */
PPred::Entry PPred::HistoryRegister::get_entry() {
  return PPred::Entry(this->pc, this->signature);
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
void PPred::HistoryRegister::add_event(PPred::event_t event) {
  // shift all the vector elements:
  for (int i = (int)signature.size() - 2; i >= 0; i--) {
    this->signature[i+1] = this->signature[i];
  }
  this->signature[0] = event;

  // Print Vector & PC
  DPRINTF(HistoryRegister, "[ HistoryRegister ] add_event(): PC %d; " \
  "Signature", this->pc);
  //for (auto i : this->signature) {
  //  DPRINTF(HistoryRegister, "%s,", PPred::event_t_name[i]);
  //}
  DPRINTF(HistoryRegister, "\n");
}

/**
 * Set the internal PC value, called from the cpu.tick() function.
 *
 */

void PPred::HistoryRegister::set_pc(uint64_t pc) {
  this->pc = pc;
}
