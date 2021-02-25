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

#ifndef __CPU_POWER_PREDICTION_TABLE_EVENT_HH__
#define __CPU_POWER_PREDICTION_TABLE_EVENT_HH__

#include <map>
#include <string>

namespace PPred {

typedef enum : int {
  BRANCH_T,
  BRANCH_NT,
  BRANCH_MP,
  MEM_MP,
  ICACHE_MISS,
  DCACHE_MISS,
  L2_MISS,
  L3_MISS,
  DTLB_MISS,
  ITLB_MISS,

  // FETCH,
  // ICACHE_FETCH,
  // ICACHE_BLOCK,
  // COMMIT_BLOCK,
  // IQ_FULL,
  // LSQ_FULL
  DUMMY_EVENT
} event_t;


extern std::map<int, std::string> event_t_name;

} // namespace PPred

/**
 * Need to extend on the hash function to hash an arbitrary class
 */
namespace std {
  template <>
  struct hash<PPred::event_t> {
    size_t operator()(const PPred::event_t& k) const {
      using type = typename std::underlying_type<PPred::event_t>::type;
      return std::hash<type>()(static_cast<type>(k));
    }
  };
} // namespace std

#endif
