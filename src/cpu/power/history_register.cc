#include "cpu/power/history_register.hh"

#include <cmath>
#include <iostream>

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "cpu/power/ml/func.h"
#include "debug/HistoryRegister.hh"
#include "python/pybind11/vpi_shm.h"
#include "sim/stat_control.hh"

/**
 * Default Constructor
 */
PPred::HistoryRegister::HistoryRegister(size_t len) {
  this->signature.resize(len, BRANCH_T);
}

/**
 * Convert the History Register to an Event type
 * @return Entry type that can be hashed or looked up in a CAM
 */
PPred::Entry PPred::HistoryRegister::get_entry() {
  std::vector<event_t> signature_temp;
  for(auto it=signature.begin(); it!=signature.end(); it++){
    signature_temp.push_back(*it);
  }
  return PPred::Entry(this->pc, signature_temp);
}

PPred::Entry PPred::HistoryRegister::get_entry(int events_to_drop) {
  auto it_sig = signature.begin();
  auto it_pc = pc_history.begin();

  std::advance(it_sig, events_to_drop);
  std::advance(it_pc, events_to_drop);
  double pc_temp = *it_pc;

  std::vector<event_t> signature_temp;
  while (it_sig != signature.end()){
    signature_temp.push_back(*it_sig);
    it_sig++;
  }

  return PPred::Entry(pc_temp, signature_temp);
}

/**
 * Add Event
 * Adds Event to the HistoryRegister; takes the internal updating PC
 * register, and external event and adds to the PC/Event registers. This is
 * so events that dont have reference to a PC can also insert events, such as
 * the memory hierarchy.
 * @param event uArch Event Type from the event_t enum
 * @return if the event is added correctly
 */
void PPred::HistoryRegister::add_event(PPred::event_t event) {
  // shift all the vector elements:
  signature.push_front(event);
  signature.pop_back();
  pc_history.push_front(pc);
  pc_history.pop_back();

}


/**
 * Set the internal PC value, called from the cpu.tick() function.
 *
 */

void PPred::HistoryRegister::set_pc(uint64_t pc) {
  this->pc = pc;
}