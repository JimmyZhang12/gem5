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
#include <unordered_set>

#include "XML_Parse.h"
#include "processor.h"
#include "xmlParser.h"

#include "cpu/power/event_type.hh"

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

class PPredUnit; //forward declare

class Mcpat{
    public:
        Mcpat(PPredUnit* _powerPred);
        Mcpat() = default;

        void init(std::string xml_dir);
        void init_stat_map_helper(Stats::Group* group, std::string path);

        void reset();

        void init_wrapper(std::string xml_dir, std::string output_path);
        void run_with_xml(std::string xml_dir, std::string output_path);

        void save_output(std::string fname, Processor &proc_t);
        void save_output(std::string output_path);
        
        void update_stats();

        void print_power(Processor &proc_t);
        void print_power();

        double get_power(Processor &proc_t);
        
        double power;


        Processor proc; //eventually make the private

        void set_mcpat_stat(Stats::Info*, std::string name);
        void init_mcpat_stat(Stats::Info*, std::string name);

        void print_stats(Stats::Info*);

        PPredUnit* powerPred;
    
    private:
        static std::unordered_map<std::string, int> stat_map;
        static std::unordered_map<std::string, Stats::Info*> name_to_stat;
        static std::unordered_set <std::string> stat_names; 

    public:
        struct _stat_storage
        {
            double cpu_numCycles,
                cpu_idleCycles,
                cpu_iq_iqInstsIssued,
                cpu_iq_FU_type_int_instructions,
                cpu_iq_FU_type_fp_instructions,
                cpu_iq_FU_type_load_instructions,
                cpu_iq_FU_type_store_instructions,

                cpu_branchPred_condPredicted,
                cpu_branchPred_condIncorrect,
                cpu_commit_committedOps,
                cpu_commit_int_insts,
                cpu_commit_fp_insts,
                cpu_rob_rob_reads,
                cpu_rob_rob_writes,
                cpu_rename_int_rename_lookups,
                cpu_rename_RenamedOperands,
                cpu_rename_RenameLookups,
                cpu_rename_fp_rename_lookups,
                cpu_iq_int_inst_queue_reads,
                cpu_iq_int_inst_queue_writes,
                cpu_iq_int_inst_queue_wakeup_accesses,
                cpu_iq_fp_inst_queue_reads,
                cpu_iq_fp_inst_queue_writes,
                cpu_iq_fp_inst_queue_wakeup_accesses,
                cpu_int_regfile_reads,
                cpu_fp_regfile_reads,
                cpu_int_regfile_writes,
                cpu_fp_regfile_writes,
                cpu_commit_function_calls,
                cpu_workload_num_syscalls,
                cpu_iq_int_alu_accesses,
                cpu_iq_fp_alu_accesses,
                cpu_iq_fu_full,
                cpu_itlb_tags_data_accesses,
                cpu_itlb_replacements,
                cpu_icache_ReadReq_accesses,
                cpu_icache_ReadReq_misses,
                cpu_icache_replacements,
                cpu_dtlb_tags_data_accesses,
                cpu_dtlb_replacements,
                cpu_dcache_ReadReq_accesses,
                cpu_dcache_ReadReq_misses,
                cpu_dcache_WriteReq_accesses,
                cpu_dcache_WriteReq_misses,
                cpu_dcache_replacements,
                cpu_branchPred_BTBLookups,
                cpu_commit_branches,
                l2_ReadExReq_accesses,
                l2_ReadCleanReq_accesses,
                l2_ReadSharedReq_accesses,
                l2_ReadCleanReq_misses,
                l2_ReadExReq_misses,
                l2_WritebackDirty_accesses,
                l2_WritebackClean_accesses,
                l2_WritebackDirty_hits,
                l2_replacements,
                l3_ReadExReq_accesses,
                l3_ReadCleanReq_accesses,
                l3_ReadSharedReq_accesses,
                l3_ReadCleanReq_misses,
                l3_ReadExReq_misses,
                l3_WritebackDirty_accesses,
                l3_WritebackClean_accesses,
                l3_WritebackDirty_hits,
                l3_replacements,
                mem_ctrls_readReqs,
                mem_ctrls_writeReqs,
                membus_pkt_count,
                tol2bus_pkt_count,
                tol3bus_pkt_count,
                cpu_branchPred_indirectHits,
                cpu_branchPred_indirectMisses,
                //also use stats to send events to power predictors,
                cpu_dcache_overall_misses,
                cpu_icache_overall_misses,
                l2_overall_misses,
                l3_overall_misses,
                cpu_dtb_walker_cache_overall_misses,
                cpu_itb_walker_cache_overall_misses;
        };

        struct _stat_storage_prev
        {
            double cpu_numCycles,
                cpu_idleCycles,
                cpu_rename_int_rename_lookups,
                cpu_rename_RenamedOperands,
                cpu_rename_RenameLookups,
                cpu_rename_fp_rename_lookups,
               
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
                l3_WritebackDirty_hits,

                membus_pkt_count,
                tol2bus_pkt_count,
                tol3bus_pkt_count,

                //also use stats to send events to power predictors,
                cpu_dcache_overall_misses,
                cpu_icache_overall_misses,
                l2_overall_misses,
                l3_overall_misses,
                cpu_dtb_walker_cache_overall_misses,
                cpu_itb_walker_cache_overall_misses;
        };

        _stat_storage stat_storage;
        _stat_storage_prev stat_storage_prev;

        ParseXML *proc_serial_xml;
        ParseXML *xml;
};





