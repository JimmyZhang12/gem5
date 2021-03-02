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
#include "cpu/power/ppred_unit.hh"

#include <algorithm>
#include <iostream>

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "debug/PPredStat.hh"

#include <chrono> 
using namespace std::chrono; 

double PPredStat::voltage = 0;
double PPredStat::current = 0;

PPredStat::PPredStat(const PPredStatParams *params)
  : ClockedObject(params),
  tickEvent([this]{ tick(); }, "PPredStat tick",
            false, Event::Power_Event_Pri),
  cycles(params->cycle_period),
  clkDomain(params->stat_clk_domain),
  frequency(params->frequency),
  ncores(params->ncores),
  mcpat_output_path(params->mcpat_output_path),
  mp(Mcpat(params->powerpred)),
  _pdn(pdn(params->ind,
    params->res,
    params->cap,
    params->vdc,
    params->frequency) 
  ),
  count(0),
  delay(1),
  max_delay(params->debug_print_delay),
  power_start_delay(params->power_start_delay),
  count_init(0),
  run_verilog(params->run_verilog)
 { 
  xml_path = mcpat_output_path + "/serial_mp.xml";
  first_time = true;
  mcpat_ready = false;
  begin = false;

  // if (run_verilog){
  //   vpi_shm::init(frequency, ncores);
  //   vpi_shm::set_ttn(frequency, cycles);
  // }

  if (!tickEvent.scheduled()) {
    DPRINTF(PPredStat, "Scheduling next tick at %lu\n", \
        clockEdge(Cycles(1)));
    schedule(tickEvent, clockEdge(Cycles(1)));
  }
}

/**
 * Global Stat Dump
 */
void
PPredStat::tick(void)
{ 
  if (Stats::pythonGetProfiling()) { //TODO move pythonGetProfiling to C++ side only
    begin = true;
    count++;

    if (mcpat_ready){
      mp.init_wrapper(xml_path, mcpat_output_path);
      
      // auto stop = high_resolution_clock::now(); 
      // auto duration = duration_cast<microseconds>(stop - start); 
      // std::cout <<"init_wrapper:" <<duration.count() << endl; 

      if (delay >= max_delay && max_delay > 0){ //debug
        mp.save_output(mcpat_output_path);
        std::cout<<"mcpat proc_internal:" << std::endl;
        mp.print_power();



        std::cout << "---power = :" << mp.power << "\n";
        std::cout << "---supply_current = :" << current << "\n";
        std::cout << "---supply_voltage = :" << voltage << "\n";

        if(run_verilog){
          std::string xml_path_serial = mcpat_output_path + "/mp.xml";
          mp.run_with_xml(xml_path_serial, mcpat_output_path);

          Stats::runVerilog();
          Stats::reset();
          mp.stat_storage = Mcpat::_stat_storage();
          mp.stat_storage_prev = Mcpat::_stat_storage_prev();
          mp.proc.XML->reset_stats();        
        }

        std::cout << '\n' << "Press a key to continue...";
        do {
        } while (cin.get() != '\n');
      }
      else{
        delay++;
      }



    }
    else{
      Stats::pythonGenerateXML();
      mp.init(xml_path);
      mcpat_ready = true;
    }

    current = _pdn.get_current(mp.power);
    voltage = _pdn.get_voltage(mp.power);
  
    //dump every cycle number of ticks
    if (count == cycles){
      count = 0;
      Stats::dump();
    }

  }
  else{
    if (power_start_delay > 0){
      count_init++;
      if (count_init > power_start_delay){
        Stats::pythonBeginProfile();
        Stats::reset();
      }
    }
    else{
      Stats::pythonBeginProfile();
      Stats::reset();   
    }

  }


  if (!tickEvent.scheduled()) {
    DPRINTF(PPredStat, "Scheduling next tick at %lu\n", \
        clockEdge(Cycles(1)));
    schedule(tickEvent, clockEdge(Cycles(1)));
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
