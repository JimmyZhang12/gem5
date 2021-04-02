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

#include "cpu/power/predictors/prediction_table.hh"

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "debug/PredictionTable.hh"
#include "python/pybind11/vpi_shm.h"
#include "sim/stat_control.hh"

PPred::Entry::Entry(int size) {
  anchor_pc = 0;
  history.resize(size, DUMMY_EVENT);
  last_updated = 0;
  access_count = 0;
  delay = 0;
}

PPred::Entry::Entry(uint64_t anchor_pc, std::vector<PPred::event_t> history) {
  this->anchor_pc = anchor_pc;
  this->history = history;
  last_updated = 0;
  access_count = 0;
  delay = 0;
}

bool
PPred::Entry::operator==(const PPred::Entry& obj) {
  return this->anchor_pc == obj.anchor_pc && this->history == obj.history;
}

bool
PPred::Entry::match(uint64_t pc, std::vector<PPred::event_t> history) {
  return this->anchor_pc == pc && this->history == history;
}

bool
PPred::Entry::equals(const PPred::Entry& entry, int hamming_distance){
  if (this->history.size() != entry.get_history_size())
    return false;
  
  int diffs = 0;
  for (int i=0; i<this->history.size(); i++){
    if (this->history[i] !=  entry.get_history_element(i))
      diffs+=1;
  }

  if (diffs > hamming_distance)
    return false;
  else
    return true;
}



void
PPred::Entry::update(uint64_t pc, std::vector<PPred::event_t> history) {
  this->anchor_pc = pc;
  this->history = history;
  last_updated = 0;
  access_count = 0;
}

std::string
PPred::Entry::to_str() {
  std::string ret;

  // std::cout << anchor_pc << "," << last_updated << "," << access_count;
  ret += (std::to_string(anchor_pc) + ": ");
  for (auto i : history) 
    ret += (event_t_name[i] + ", ");
  return ret;
}