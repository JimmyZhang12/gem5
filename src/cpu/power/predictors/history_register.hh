#ifndef __CPU_POWER_HISTORY_REGISTER_HH__
#define __CPU_POWER_HISTORY_REGISTER_HH__

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>

#include "base/statistics.hh"
#include "base/types.hh"
#include "cpu/inst_seq.hh"
#include "cpu/power/event_type.hh"
#include "cpu/power/ml/array.h"
#include "cpu/power/predictors/prediction_table.hh"
#include "cpu/static_inst.hh"
#include "sim/sim_object.hh"

namespace PPred {

class HistoryRegister {
  /**
   * Event History of Execution
   */
  std::deque<event_t> signature;
  std::deque<uint64_t> pc_history;


  public:
    /**
     * PC of last taken branch
     */
    uint64_t pc;

    
    std::deque<int> entry_head_time;

    /**
     * Default Constructor
     */
    HistoryRegister(size_t len = 4);

    /**
     * resize
     * Resize the history register
     * @param len The length of the new history register
     */
    void resize(size_t len = 4);

    void tick();

    /**
     * Convert the History Register to an Event type
     * @return Entry type that can be hashed or looked up in a CAM
     */
    Entry get_entry();
    Entry get_entry_drop_front(int events_to_drop);
    Entry get_entry_drop_back(int events_to_drop);

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
    void add_event(event_t event);

    /**
     * Set the internal PC value, called from the cpu.tick() function.
     * W
     */
    void set_pc(uint64_t pc);

    /**
     * Friend ostream& operator<<
     * Write the contents of the History Register out to a stream
     * @param os The output stream
     * @param t This
     * @return output stream reference
     */
    // friend std::ostream& operator<<(std::ostream& os, const HistoryRegister& t) {
    //   // Print PC Followed by all the uArchEvent IDs
    //   for (size_t i = 0; i < t.signature.size(); i++) {
    //     os << ",";
    //     os << std::dec << t.signature[i];
    //   }

    //   return os;
    // }

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
    std::deque<event_t> get_signature() const {
      return signature;
    }

    /**
     * get_pc
     * Return the PC value
     * @return Anchor PC
     */
    uint64_t get_pc() const {
      return pc;
    }


    std::string to_str();
};

} // namespace PPred

#endif // __CPU_POWER_HISTORY_REGISTER_HH__