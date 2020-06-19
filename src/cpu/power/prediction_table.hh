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

#ifndef __CPU_POWER_PREDICTION_TABLE_HH__
#define __CPU_POWER_PREDICTION_TABLE_HH__

#include <cassert>
#include <cstdlib>
#include <vector>

#include "base/statistics.hh"
#include "base/types.hh"
#include "cpu/inst_seq.hh"
#include "cpu/power/event_type.hh"
#include "cpu/static_inst.hh"
#include "sim/sim_object.hh"

namespace PPred {

class Entry {
private:
  // Event History
  uint64_t anchor_pc;
  std::vector<event_t> history;

  // Stat Member Variables
  uint64_t last_updated;
  uint64_t access_count;

public:
  /**
   * Create a new table
   * @param size The number of events in an entry
   * @return None
   */
  Entry(int size = 1);
  Entry(uint64_t anchor_pc, std::vector<event_t> history);

  /**
   * Checks if the entry matches an event/anchor_pc pair
   * @param pc The anchor PC of a history
   * @param event A vector of microarch events from the history register
   * @return boolean Match
   */
  bool match(uint64_t pc, std::vector<event_t> event);
  bool operator==(const Entry& obj);

  /**
   * Sets a new event/anchor_pc pair to this entry
   * @param pc The anchor PC of a history
   * @param event A vector of microarch events from the history register
   * @return None
   */
  void update(uint64_t pc, std::vector<event_t> event);

  /**
   * Updates the stat counters for accessing an entry
   */
  void access(void) {
    access_count++;
    last_updated = 0;
  }

  /**
   * Ticks the LRU last_updated counter
   */
  void tick(void) { last_updated++; }

  /**
   * Returns the last updated counter
   */
  uint64_t get_lru(void) { return last_updated; }

};

/**
 * Need to extend on the hash function to hash an arbitrary class
 */
//namespace std {
//  template <>
//  struct hash<Entry> {
//    size_t operator()(const Entry& k) const {
//      hash<uint64_t> h1;
//      hash<vector<event_t>> h2;
//      size_t seed = 0;
//      seed ^= h1(k.pc) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
//      seed ^= h2(k.history) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
//      return seed;
//    }
//  };
//}

class Table {
  // Prediction Table, Table of Entries
  std::vector<Entry> prediction_table;

public:
  // Stats:
  uint64_t insertions;
  uint64_t matches;
  uint64_t misses;

  /**
   * Create a new table
   * @param table_size The number of entries
   * @param history_length The number of events per history
   * @return None
   */
  Table(uint64_t table_size = 0, uint64_t history_length = 0);

  /**
   * Resize Table
   * @param table_size The number of entries
   * @param history_length The number of events per history
   * @return None
   */
  void resize(uint64_t table_size = 1, uint64_t history_length = 1);

  /**
   * Find an entry in the prediction table
   * @param pc The anchor program counter
   * @param history The event history register
   * @return boolean return true if the event is in the table
   */
  bool find(uint64_t pc, std::vector<event_t> history);
  bool find(const Entry& obj);

  /**
   * Insert an Entry based on LRU Replacement Policy
   * @param pc The anchor program counter
   * @param history The event history register
   * @return boolean insert success
   */
  bool insert(uint64_t pc, std::vector<event_t> history);
  bool insert(const Entry& obj);

  /**
   * Ticks the event history table for the LRU Policy
   */
  void tick(void);
};

} // namespace PPred

#endif
