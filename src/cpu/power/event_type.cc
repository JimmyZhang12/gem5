#include <map>
#include <string>

#include "event_type.hh"

std::map<int, std::string> PPred::event_t_name = {
  {BRANCH_T,     "BRANCH_T"},
  {BRANCH_NT,    "BRANCH_NT"},
  {BRANCH_MP,    "BRANCH_MISS"},
  {MEM_MP,       "MEM_ORDER_VIO"},
  {ICACHE_MISS,      "ICACHE_MISS"},
  {DCACHE_MISS,      "DCACHE_MISS"},
  {L2_MISS,      "L2_MISS"},
  {L3_MISS,      "L3_MISS"},
  {DTLB_MISS,    "DTLB_MISS"},
  {ITLB_MISS,    "ITLB_MISS"},
  {DUMMY_EVENT,  "DUMMY_EVENT"}
  // {FETCH,        "FETCH"},
  // {ICACHE_FETCH, "ICACHE_FETCH"},
  // {ICACHE_BLOCK, "ICACHE_BLOCK"},
  // {COMMIT_BLOCK, "COMMIT_BLOCK"},
  // {IQ_FULL,      "IQ_FULL"},
  // {LSQ_FULL,     "LSQ_FULL"}
};
