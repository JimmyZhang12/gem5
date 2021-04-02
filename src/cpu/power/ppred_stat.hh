/*
 * Copyright (c) 2020 University of Illinois
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

#ifndef __PPRED_STAT_DUMP_HH__
#define __PPRED_STAT_DUMP_HH__

#include <string>
#include <cmath>

#include "base/statistics.hh"
#include "base/types.hh"

#include "cpu/inst_seq.hh"
#include "cpu/power/predictors/history_register.hh"
#include "cpu/static_inst.hh"

// #include "debug/PowerPred.hh"
#include "params/PPredStat.hh"
#include "python/pybind11/vpi_shm.h"
#include "sim/clocked_object.hh"
#include "sim/global_event.hh"
#include "sim/sim_object.hh"
#include "sim/stat_control.hh"

#include "mcpat.hh"
#include "pdn.hh"
#include "write_data.hh"

class PPredUnit;//forward declare to avoid circular includes

class PPredStat : public ClockedObject
{
  public:
    typedef PPredStatParams Params;

    /**
     * Constructor
     * @param params The params object
     */
    PPredStat(const Params *p);

    /**
     * tick: Dump the stats
     */
    void tick(void);

    /**
     * get_begin:
     * @return True if stats have begun
     */
    bool get_begin() const;

    void clk_throttle(double new_clk);

    void clk_restore();

    double get_voltage() const {return voltage;}

    double get_current() const {return current;}



  private:

    void run_debug();

    /** The tick event used for scheduling CPU ticks. */
    EventFunctionWrapper tickEvent;

    PPredUnit* powerPred;
    SrcClockDomain* clkDomain;

    /** Cycles between stat dumps */
    const unsigned int cycles;
    const unsigned int num_dumps;
    double frequency;
    double ncores;

    const std::string mcpat_output_path;
    const std::string gem5_output_path;
    std::string xml_path;

    /** Time */
    bool first_time;



    const int debug_print_delay;
    const int power_start_delay;
    const bool run_verilog;
    const bool save_data;

    Mcpat mp;
    pdn _pdn;
    SaveData power_data;

    double voltage;
    double current;

    bool mcpat_ready=false;
    unsigned int count_init = 0;
    int count = 0;
    /** Flag to signal the PPred testing can begin */
    bool begin = false;

    
};

#endif // __PPRED_STAT_DUMP_HH__
