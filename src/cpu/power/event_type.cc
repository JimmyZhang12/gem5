#include <map>
#include <string>

#include "event_type.hh"

std::map<int, std::string> PPred::event_t_name = {
  {BRANCH_T,     "BRCH_T"},
  {BRANCH_NT,    "BRCH_N"},
  {BRANCH_MP,    "BRCH_M"},
  {MEM_MP,       "MEMORV"},
  {ICACHE_MISS,  "ICHE_M"},
  {DCACHE_MISS,  "DCHE_M"},
  {L2_MISS,      "L2_C_M"},
  {L3_MISS,      "L3_C_M"},
  {DTLB_MISS,    "DTLB_M"},
  {ITLB_MISS,    "ITLB_M"},
  {DUMMY_EVENT,  "DUMMYE"}
  // {FETCH,        "FETCH"},
  // {ICACHE_FETCH, "ICACHE_FETCH"},
  // {ICACHE_BLOCK, "ICACHE_BLOCK"},
  // {COMMIT_BLOCK, "COMMIT_BLOCK"},
  // {IQ_FULL,      "IQ_FULL"},
  // {LSQ_FULL,     "LSQ_FULL"}
};
