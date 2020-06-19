#include <map>
#include <string>

#include "event_type.hh"

std::map<int, std::string> PPred::event_t_name = {
  {BRANCH_T,     "BRANCH_T"},
  {BRANCH_NT,    "BRANCH_NT"},
  {FETCH,        "FETCH"},
  {ICACHE_FETCH, "ICACHE_FETCH"},
  {ICACHE_BLOCK, "ICACHE_BLOCK"},
  {COMMIT_BLOCK, "COMMIT_BLOCK"}
//  {IQ,           "IQ"},
//  {L2,           "L2"},
//  {FLUSH,        "FLUSH"},
//  {DL1,          "DL1"},
//  {DTLB,         "DTLB"}
};
