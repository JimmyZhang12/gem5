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
#include <string>


#include "base/statistics.hh"
#include "base/types.hh"
#include "cpu/inst_seq.hh"
#include "cpu/power/predictors/bloomfilter.h"
#include "cpu/power/event_type.hh"
#include "cpu/static_inst.hh"
#include "sim/sim_object.hh"

namespace PPred {

class Entry {
private:
  // Event History
  uint64_t anchor_pc;

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

  std::vector<event_t> history;
  uint64_t delay;


  /**
   * Checks if the entry matches an event/anchor_pc pair
   * @param pc The anchor PC of a history
   * @param event A vector of microarch events from the history register
   * @return boolean Match
   */
  bool match(uint64_t pc, std::vector<event_t> event);
  bool operator==(const Entry& obj);
  bool equals(const Entry& entry, int hamming_distance);

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
  uint64_t get_lru(void) const { return last_updated; }

  uint64_t get_access(void) const { return access_count; }

  uint64_t get_pc(void) const { return anchor_pc; }

  std::vector<event_t> get_history(void) const { return history; }

  int get_history_size(void) const { return history.size(); }
  event_t get_history_element(int index) const { return history[index]; }

  std::string to_str();
};


class Table {
  // Prediction Table, Table of Entries
  std::vector<Entry> prediction_table;

public:
  // Stats:
  uint64_t insertions;
  uint64_t matches;
  uint64_t misses;
  int last_find_index;
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
  bool find_variable_signature_len(const PPred::Entry& obj);
  bool find(uint64_t pc, std::vector<event_t> history);
  bool find(const Entry& obj, int hamming_distance);
  bool find(const Entry& obj);

  /**
   * Insert an Entry based on LRU Replacement Policy
   * @param pc The anchor program counter
   * @param history The event history register
   * @return boolean insert success
   */
  int insert(uint64_t pc, std::vector<event_t> history);
  int insert(const Entry& obj);

  Entry operator[](const int& index) {
      return prediction_table[index];
  }

  /**
   * Ticks the event history table for the LRU Policy
   */
  void tick(void);

  void print();
};

class TableBloom {
  // Prediction Table, Table of Entries
  std::vector<Entry> prediction_table;
  Bloomfilter<Entry> bf;

  uint64_t threshold;

public:
  // Stats:
  uint64_t insertions;
  uint64_t matches_cam;
  uint64_t matches_bloom;
  uint64_t misses;

  /**
   * Create a new table
   * @param table_size The number of entries
   * @param history_length The number of events per history
   * @param n The n hashes of the bloom filter
   * @param bf_size The size of the bloom filter table
   * @param seed The srand seed for the bloom filter n hashes
   * @param threshold The access count threshold required to insert a cam
   *    table entry into the bloom filter on replacement
   * @return None
   */
  TableBloom(uint64_t table_size = 0,
             uint64_t history_length = 0,
             uint64_t n = 3,
             uint64_t bf_size=2048,
             uint64_t seed = 0,
             uint64_t threshold = 1);

  /**
   * Resize Table
   * @param table_size The number of entries
   * @param history_length The number of events per history
   * @param n The n hashes of the bloom filter
   * @param bf_size The size of the bloom filter table
   * @param seed The srand seed for the bloom filter n hashes
   * @param threshold The access count threshold required to insert a cam
   *    table entry into the bloom filter on replacement
   * @return None
   */
  void resize(uint64_t table_size = 0,
              uint64_t history_length = 0,
              uint64_t n = 3,
              uint64_t bf_size=2048,
              uint64_t seed = 0);

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

  void print();
};

class Infinite_Table {
  // Prediction Table, Table of Entries
  std::vector<Entry> prediction_table;

public:
  // Stats:
  uint64_t insertions;
  uint64_t matches;
  uint64_t misses;
  int last_find_index;
  /**
   * Create a new table
   * @param table_size The number of entries
   * @param history_length The number of events per history
   * @return None
   */
  Infinite_Table();

  /**
   * Find an entry in the prediction table
   * @param pc The anchor program counter
   * @param history The event history register
   * @return boolean return true if the event is in the table
   */
  bool find(uint64_t pc, std::vector<event_t> history);
  bool find(const Entry& obj, int hamming_distance);
  bool find(const Entry& obj);

  /**
   * Insert an Entry based on LRU Replacement Policy
   * @param pc The anchor program counter
   * @param history The event history register
   * @return boolean insert success
   */
  int insert(uint64_t pc, std::vector<event_t> history);
  int insert(const Entry& obj);

  Entry operator[](const int& index) {
      return prediction_table[index];
  }

  /**
   * Ticks the event history table for the LRU Policy
   */
  void tick(void);

  void print();
};

} // namespace PPred


/**
 * Need to extend on the hash function to hash an arbitrary class
 */
namespace std {
  template <>
  struct hash<PPred::Entry> {
    size_t operator()(const PPred::Entry& k) const {
      hash<uint64_t> h1;
      hash<vector<PPred::event_t>> h2;
      size_t seed = 0;
      seed ^= h1(k.get_pc()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      seed ^= h2(k.get_history()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      return seed;
    }
  };
} // namespace std

#endif
