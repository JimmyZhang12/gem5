/*
 * Copyright (c) 2020, University of Illinois
 * All rights reserved
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

#include "cpu/power/ppred_stat.hh"

#include <algorithm>

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "debug/PPredStat.hh"

PPredStat::PPredStat(const Params *params)
  : ClockedObject(params),
  tickEvent([this]{ tick(); }, "PPredStat tick",
            false, Event::Power_Event_Pri),
  cycles(params->cycle_period),
  clkDomain(params->stat_clk_domain)
{
  /* Do Nothing */
  first_time = true;
  begin = false;
  if (!tickEvent.scheduled()) {
    DPRINTF(PPredStat, "Scheduling next tick at %lu\n", \
        clockEdge(Cycles(cycles)));
    schedule(tickEvent, clockEdge(Cycles(cycles)));
  }
}

/**
 * Global Stat Dump
 */
void
PPredStat::tick(void)
{
  if (Stats::pythonGetProfiling()) {
    if (first_time) {
      Stats::reset();
      first_time = false;
    }
    else {
      Stats::dump();
      Stats::reset();
      begin = true;
    }
  }
  if (!tickEvent.scheduled()) {
    DPRINTF(PPredStat, "Scheduling next tick at %lu\n", \
        clockEdge(Cycles(cycles)));
    schedule(tickEvent, clockEdge(Cycles(cycles)));
  }
}


/**
 * get_begin:
 * @return True if stats have begun
 */
bool
PPredStat::get_begin() const {
  return this->begin;
}

PPredStat*
PPredStatParams::create()
{
  return new PPredStat(this);
}
