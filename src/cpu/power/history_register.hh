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
#include <iostream>
#include <vector>

#include "base/statistics.hh"
#include "base/types.hh"
#include "cpu/inst_seq.hh"
#include "cpu/power/event_type.hh"
#include "cpu/power/ml/array.h"
#include "cpu/power/prediction_table.hh"
#include "cpu/static_inst.hh"
#include "sim/sim_object.hh"

namespace PPred {

class HistoryRegister {
  /**
   * Event History of Execution
   */
  std::vector<event_t> signature;

  std::vector<event_t> new_events;

  /**
   * PC of last event
   */
  std::vector<uint64_t> pc;

  /**
   * PC of tick
   */
  uint64_t inst_pc;


public:
  /**
   * Default Constructor
   */
  HistoryRegister(size_t len = 4);

  /**
   * resize
   * Resize the history register
   * @param len The length of the new history register
   */
  void resize(size_t len = 4) {
    signature.resize(len);
    pc.resize(len);
    new_events.resize(len);
  }

  /**
   * clear newly added events from new events vector
   */
  void clear_new_events();


  /**
   * Convert the History Register to an Event type
   * @return Entry type that can be hashed or looked up in a CAM
   */
  Entry get_entry();

  /**
   * Convert the History Register to an Array2D type that can be used in the
   * perceptron and DNN
   * @param events Number of events/pcs to return
   * @param no_pc Return an array with no PC values
   * @param anchor_pc Return an array with [anchor_pc, e0, e1,...]
   * @return Array2D
   */
  Array2D get_array2d(size_t events, bool no_pc=false, bool anchor_pc=false);

  /**
   * Add Event
   * Adds Event to the HistoryRegister; takes the internal updating PC
   * register, and external event and adds to the PC/Event registers. This is
   * so events that dont have reference to a PC can also insert events, such as
   * the memory hierarchy.
   * @param event uArch Event Type from the event_t enum
   * @return if the event is added correctly
   */
  bool add_event(event_t event);

  /**
   * Set the internal PC value, called from the cpu.tick() function.
   * @param pc The current pc value at the time of the cpu.tick() function.
   */
  void set_pc(uint64_t pc);

  /**
   * Friend ostream& operator<<
   * Write the contents of the History Register out to a stream
   * @param os The output stream
   * @param t This
   * @return output stream reference
   */
  friend std::ostream& operator<<(std::ostream& os, const HistoryRegister& t) {
    // Print PC Followed by all the uArchEvent IDs
    for (size_t i = 0; i < t.pc.size(); i++) {
      if (i != 0) {
        os << ",";
      }
      os << std::hex << t.pc[i];
    }
    for (size_t i = 0; i < t.signature.size(); i++) {
      os << ",";
      os << std::dec << t.signature[i];
    }
    return os;
  }

  /**
   * == operator
   * Compare the other HR to self
   * @param other
   * @return bool
   */
  bool operator==(const HistoryRegister& other) const {
    return (this->get_signature() == other.get_signature() && \
        this->get_pc() == other.get_pc());
  }

  /**
   * != operator
   * Compare the other HR to self
   * @param other
   * @return bool
   */
  bool operator!=(const HistoryRegister& other) const {
    return (this->get_signature() != other.get_signature() || \
        this->get_pc() != other.get_pc());
  }

  /**
   * get_signature
   * Return the signature vector
   * @return Anchor Signature
   */
  std::vector<event_t> get_signature() const {
    return signature;
  }
 std::vector<event_t> get_new_events() const {
    return new_events;
  }
  
  /**
   * get_pc
   * Return the PC value
   * @return Anchor PC
   */
  uint64_t get_pc() const {
    return pc[0];
  }


};

} // namespace PPred

#endif // __CPU_POWER_HISTORY_REGISTER_HH__
