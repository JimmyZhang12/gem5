#include <map>
#include <string>

#include "event_type.hh"

//https://www.gem5.org/documentation/general_docs/cpu_models/O3CPU##Pipeline-stages

std::map<int, std::string> PPred::event_t_name = {
  {BRANCH_T,     "BRANCH_T"},
  {BRANCH_NT,    "BRANCH_NT"},
  {BRANCH_MP,    "BRANCH_MISS"},
  {FETCH,        "FETCH"},
  {TLB_STALL,    "TLB_STALL"},
  {ICACHE_STALL, "ICACHE_STALL"},
  {COMMIT_BLOCK, "COMMIT_BLOCK"},
  {IQ_FULL,      "IQ_FULL"},
  {LSQ_FULL,     "LSQ_FULL"},
  {LOAD_EX,      "LOAD_EX"},
  {LOAD_WB,      "LOAD_WB"},
  {LOAD_CFETCH,  "LOAD_CFETCH"},

//LSQUnit::executeStore()
  {STORE_EXECUTE, "STORE_EXECUTE"},
//IEW::tick()->LSQUnit::writebackStores()
  {STORE_WB, "STORE_WB"},
//IEW::tick()->IEW::dispatchInsts()
  {INSTR_DISPATCH, "INSTR_DISPATCH"},
//IEW::tick()->InstructionQueue::scheduleReadyInsts()
  {INSTR_ISSUE, "INSTR_ISSUE"},
//IEW::executeInsts())
  {INSTR_EXECUTE, "INSTR_EXECUTE"}, //non memory 
//Commit::commitInsts()
  {INSTR_COMMIT, "INSTR_COMMIT"},
//memory mispeculation
  {MEM_MP, "MEM_MP"},

  {EMPTY_EVENT,  "EMPTY_EVENT"},
  {DUMMY_EVENT2, "DUMMY_EVENT2"}

};
