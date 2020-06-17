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

#ifndef __CPU_POWER_HISTORY_REGISTER_HH__
#define __CPU_POWER_HISTORY_REGISTER_HH__

#include <cassert>
#include <cstdlib>
#include <vector>

#include "base/statistics.hh"
#include "base/types.hh"
#include "cpu/inst_seq.hh"
#include "cpu/power/event_type.hh"
#include "cpu/power/ppred_unit.hh"
#include "cpu/power/prediction_table.hh"
#include "cpu/static_inst.hh"
#include "sim/sim_object.hh"

namespace PPred {

class HistoryRegister {
  /**
   * Event History of Execution
   */
  std::vector<event_t> signature;

  /**
   * PC of last event
   */
  uint64_t anchor_pc;

public:
  /**
   * Default Constructor
   */
  HistoryRegister(size_t len = 4);

  /**
   * Convert the History Register to an Event type
   * @return Entry type that can be hashed or looked up in a CAM
   */
  Entry get_entry();

  /**
   * Add Event
   * Adds Event to the HistoryRegister; requires an event_t enum and ad the PC
   * of the uArch event
   * @param PC Current Program Counter
   * @param event uArch Event Type from the event_t enum
   * @return None
   */
  void add_event(uint64_t PC, event_t event);
};

} // namespace PPred

#endif // __CPU_POWER_HISTORY_REGISTER_HH__
