// #include <fcntl.h>
// #include <semaphore.h>
// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
#include <string>
// #include <sys/mman.h>
// #include <sys/shm.h>
// #include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>

#include "XML_Parse.h"
#include "processor.h"
#include "xmlParser.h"

#include "base/statistics.hh"
#include "base/stats/group.hh"
#include "base/stats/info.hh"
#include "base/stats/output.hh"
#include "base/stats/types.hh"
#include "base/cast.hh"
#include "base/cprintf.hh"
#include "base/intmath.hh"
#include "base/str.hh"
#include "base/types.hh"
#include "sim/root.hh"

class Mcpat{
    public:
    Mcpat();

    typedef const std::vector< Stats::Info * > & Stat_list;

    void init(std::string xml_dir);
    void reset();
    void compute(std::string output_path);
    void init_wrapper(std::string xml_dir, std::string output_path);
    void update();
    void save_output(std::string fname, Processor &proc_t);
    ParseXML get_xml();

    void update_stats();
    void update_stats_helper(Stats::Group* group, std::string path);
    void print_power(Processor &proc_t);

    void test(std::string filepath);

    Processor proc; //eventually make the private

    void update_system_stats();

    void set_mcpat_stat(Stats::Info*);

    private:
    static std::unordered_map<std::string, int> stat_map;

    struct _stat_storage
    {
        double numCycles,
            idleCycles,
            RenamedOperands,
            int_rename_lookups,
            fp_rename_lookups,
            RenameLookups,
            l2_ReadExReq_accesses,
            l2_ReadCleanReq_accesses,
            l2_ReadSharedReq_accesses,
            l2_ReadCleanReq_misses,
            l2_ReadExReq_misses,
            l2_WritebackDirty_accesses,
            l2_WritebackClean_accesses,
            l2_WritebackDirty_hits,
            l3_ReadExReq_accesses,
            l3_ReadCleanReq_accesses,
            l3_ReadSharedReq_accesses,
            l3_ReadCleanReq_misses,
            l3_ReadExReq_misses,
            l3_WritebackDirty_accesses,
            l3_WritebackClean_accesses,
            l3_WritebackDirty_hits;
    };
    _stat_storage stat_storage;

    ParseXML *proc_serial_xml;
    ParseXML *xml;
};





