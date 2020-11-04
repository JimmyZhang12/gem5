#include <map>
#include <string>

#include "event_type.hh"

std::map<int, std::string> PPred::event_t_name = {
  {BRANCH_T,     "BRANCH_T"},
  {BRANCH_NT,    "BRANCH_NT"},
  {BRANCH_MP,    "BRANCH_MISS"},
  {FETCH,        "FETCH"},
  {ICACHE_FETCH, "ICACHE_FETCH"},
  {ICACHE_BLOCK, "ICACHE_BLOCK"},
  {COMMIT_BLOCK, "COMMIT_BLOCK"},
  {IQ_FULL,      "IQ_FULL"},
  {LSQ_FULL,     "LSQ_FULL"},
  {LOAD_EX,      "LOAD_EX"},
  {LOAD_BLOCK,   "LOAD_BLOCK"},
  {DUMMY_EVENT,  "DUMMY_EVENT"}
};
