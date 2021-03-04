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

#include "cpu/power/prediction_table.hh"

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
  history.resize(size, BRANCH_T);
  last_updated = 0;
  access_count = 0;
}

PPred::Entry::Entry(uint64_t anchor_pc, std::vector<PPred::event_t> history) {
  this->anchor_pc = anchor_pc;
  this->history = history;
  last_updated = 0;
  access_count = 0;
}

bool
PPred::Entry::operator==(const PPred::Entry& obj) {
  return this->anchor_pc == obj.anchor_pc && this->history == obj.history;
}



bool
PPred::Entry::match(uint64_t pc, std::vector<PPred::event_t> history) {
  return this->anchor_pc == pc && this->history == history;
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

PPred::Table::Table(uint64_t table_size, uint64_t history_length) {
  this->resize(table_size, history_length);
}

void
PPred::Table::resize(uint64_t table_size, uint64_t history_length) {
  // Prediction Table
  prediction_table.resize(table_size, Entry(history_length));

  // Stats
  insertions = 0;
  matches = 0;
  misses = 0;
}

bool
PPred::Table::find(uint64_t pc, std::vector<PPred::event_t> history) {
  for (auto it = this->prediction_table.begin();
      it != this->prediction_table.end(); it++) {
    if (it->match(pc, history)) {
      it->access();
      matches++;
      return true;
    }
  }
  misses++;
  return false;
}

bool
PPred::Table::find(const PPred::Entry& obj) {
  for (auto it = this->prediction_table.begin(); it != this->prediction_table.end(); it++) {
    if (*it == obj) {
      it->access();
      matches++;
      return true;
    }
  }
  misses++;
  return false;
}

int
PPred::Table::insert(uint64_t pc, std::vector<PPred::event_t> history) {
  PPred::Entry obj = Entry(pc, history);
  return (this->insert(obj));
}

int
PPred::Table::insert(const Entry& obj) {
  insertions++;
  uint64_t max_time = 0;
  size_t idx = 0;

  for (size_t i = 0; i < prediction_table.size(); i++) {
    if (prediction_table[i] == obj){
      prediction_table[i] = obj;
      return idx;
    }
    if (max_time < prediction_table[i].get_lru()) {
      max_time = prediction_table[i].get_lru();
      idx = i;
    }
  }
  prediction_table[idx] = obj;
  return idx;
}

void
PPred::Table::tick(void) {
  for (auto it = this->prediction_table.begin(); it != this->prediction_table.end(); it++) {
    it->tick();
  }
}







void
PPred::Table::print() {
  //TODO JIMMY
  // std::cout << "\n\n";
  // for (auto it = this->prediction_table.begin();
  //   it != this->prediction_table.end(); it++) {
  //   it->print();
  // }
}

PPred::TableBloom::TableBloom(uint64_t table_size,
                              uint64_t history_length,
                              uint64_t n,
                              uint64_t bf_size,
                              uint64_t seed,
                              uint64_t threshold) {
  this->threshold = threshold;
  this->resize(table_size, history_length, n, bf_size, seed);
}

void
PPred::TableBloom::resize(uint64_t table_size,
                          uint64_t history_length,
                          uint64_t n,
                          uint64_t bf_size,
                          uint64_t seed) {
  // Prediction TableBloom
  prediction_table.resize(table_size, Entry(history_length));
  bf.resize(n, bf_size, seed);

  // Stats
  insertions = 0;
  matches_cam = 0;
  matches_bloom = 0;
  misses = 0;
}

bool
PPred::TableBloom::find(uint64_t pc, std::vector<PPred::event_t> history) {
  for (auto it = this->prediction_table.begin();
      it != this->prediction_table.end(); it++) {
    if (it->match(pc, history)) {
      it->access();
      matches_cam++;
      return true;
    }
  }
  PPred::Entry obj(pc, history);
  if (bf.find(obj)) {
    matches_bloom++;
    return true;
  }
  misses++;
  return false;
}

bool
PPred::TableBloom::find(const PPred::Entry& obj) {
  for (auto it = this->prediction_table.begin();
      it != this->prediction_table.end(); it++) {
    if (*it == obj) {
      it->access();
      matches_cam++;
      return true;
    }
  }
  if (bf.find(obj)) {
    matches_bloom++;
    return true;
  }
  misses++;
  return false;
}

bool
PPred::TableBloom::insert(uint64_t pc, std::vector<PPred::event_t> history) {
  insertions++;
  uint64_t max_time = 0;
  size_t idx = 0;
  if (TableBloom::find(pc, history)) {
    return true;
  }
  for (size_t i = 0; i < prediction_table.size(); i++) {
    if (max_time < prediction_table[i].get_lru()) {
      max_time = prediction_table[i].get_lru();
      idx = i;
    }
  }
  if (prediction_table[idx].get_access() >= threshold) {
    bf.insert(prediction_table[idx]);
  }
  prediction_table[idx].update(pc, history);
  prediction_table[idx].access();
  return true;
}

bool
PPred::TableBloom::insert(const Entry& obj) {
  insertions++;
  uint64_t max_time = 0;
  size_t idx = 0;
  if (find(obj)) {
    return true;
  }
  for (size_t i = 0; i < prediction_table.size(); i++) {
    if (max_time < prediction_table[i].get_lru()) {
      max_time = prediction_table[i].get_lru();
      idx = i;
    }
  }
  if (prediction_table[idx].get_access() >= threshold) {
    bf.insert(prediction_table[idx]);
  }
  prediction_table[idx] = obj;
  prediction_table[idx].access();
  return true;
}

void
PPred::TableBloom::tick(void) {
  for (auto it = this->prediction_table.begin();
    it != this->prediction_table.end(); it++) {
    it->tick();
  }
}

void
PPred::TableBloom::print() {
  //TODO JIMMY
  // std::cout << "\n\n";
  // for (auto it = this->prediction_table.begin();
  //   it != this->prediction_table.end(); it++) {
  //   it->print();
  // }
}
