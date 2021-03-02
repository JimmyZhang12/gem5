#include <string>
#include "mcpat.hh"
#include "enums/OpClass.hh"
#include "cpu/power/ppred_unit.hh"

//TODO: case conditions really should be enums to avoid magic numbers
//TODO: possible optimization is to avoid resetting formula stats which does nothing
std::unordered_map<std::string, int>
Mcpat::stat_map = unordered_map<std::string, int>({
     {"system.cpu.numCycles" , 24},
     {"system.cpu.idleCycles" , 25},
     {"system.cpu.iq.iqInstsIssued" , 2},
     {"system.cpu.iq.FU_type" , 16},
     {"system.cpu.branchPred.condPredicted" , 17},
     {"system.cpu.branchPred.condIncorrect" , 18},
     {"system.cpu.commit.committedOps" , 19},
     {"system.cpu.commit.int_insts" , 20},
     {"system.cpu.commit.fp_insts" , 21},
     {"system.cpu.rob.rob_reads" , 26},
     {"system.cpu.rob.rob_writes" , 27},
     {"system.cpu.rename.int_rename_lookups" , 28},
     {"system.cpu.rename.RenamedOperands" , 33},
     {"system.cpu.rename.RenameLookups" , 35},
     {"system.cpu.rename.fp_rename_lookups" , 34},
     {"system.cpu.iq.int_inst_queue_reads" , 36},
     {"system.cpu.iq.int_inst_queue_writes" , 37},
     {"system.cpu.iq.int_inst_queue_wakeup_accesses" , 38},
     {"system.cpu.iq.fp_inst_queue_reads" , 39},
     {"system.cpu.iq.fp_inst_queue_writes" , 40},
     {"system.cpu.iq.fp_inst_queue_wakeup_accesses" , 41},
     {"system.cpu.int_regfile_reads" , 42},
     {"system.cpu.fp_regfile_reads" , 43},
     {"system.cpu.int_regfile_writes" , 44},
     {"system.cpu.fp_regfile_writes" , 45},
     {"system.cpu.commit.function_calls" , 46},
     {"system.cpu.workload.num_syscalls" , 47},
     {"system.cpu.iq.int_alu_accesses" , 51},
     {"system.cpu.iq.fp_alu_accesses" , 53},
     {"system.cpu.iq.fu_full" , 50},
     {"system.cpu.itlb.tags.data_accesses" , 54},
     {"system.cpu.itlb.replacements" , 55},
     {"system.cpu.icache.ReadReq_accesses" , 56}, //formula
     {"system.cpu.icache.ReadReq_misses" , 57},
     {"system.cpu.icache.replacements" , 58},
     {"system.cpu.dtlb.tags.data_accesses" , 59},
     {"system.cpu.dtlb.replacements" , 60},
     {"system.cpu.dcache.ReadReq_accesses" , 61}, //formula
     {"system.cpu.dcache.ReadReq_misses" , 62},
     {"system.cpu.dcache.WriteReq_accesses" , 63}, //formula
     {"system.cpu.dcache.WriteReq_misses" , 64},
     {"system.cpu.dcache.replacements" , 65},
     {"system.cpu.branchPred.BTBLookups" , 66},
     {"system.cpu.commit.branches" , 67},
     {"system.l2.ReadExReq_accesses" , 68}, //formula
     {"system.l2.ReadCleanReq_accesses" , 69}, //formula
     {"system.l2.ReadSharedReq_accesses" , 70}, //formula
     {"system.l2.ReadCleanReq_misses" , 71},
     {"system.l2.ReadExReq_misses" , 72},
     {"system.l2.WritebackDirty_accesses" , 73}, //formula
     {"system.l2.WritebackClean_accesses" , 75}, //formula
     {"system.l2.WritebackDirty_hits" , 76},
     {"system.l2.replacements" , 77},
     {"system.l3.ReadExReq_accesses" , 78}, //formula
     {"system.l3.ReadCleanReq_accesses" , 79}, //formula
     {"system.l3.ReadSharedReq_accesses" , 80}, //formula
     {"system.l3.ReadCleanReq_misses" , 81},
     {"system.l3.ReadExReq_misses" , 82},
     {"system.l3.WritebackDirty_accesses" , 83}, //formula
     {"system.l3.WritebackClean_accesses" , 85}, //formula
     {"system.l3.WritebackDirty_hits" , 86},
     {"system.l3.replacements" , 87},
     {"system.mem_ctrls.readReqs" , 90},
     {"system.mem_ctrls.writeReqs" , 91},
     {"system.membus.pkt_count", 92},
     {"system.tol2bus.pkt_count", 93}, 
     {"system.tol3bus.pkt_count", 94},
     {"system.cpu.branchPred.indirectHits", 95},
     {"system.cpu.branchPred.indirectMisses", 96},
     //also use stats to send events to power predictors
     {"system.cpu.dcache.overall_misses", 150}, //formula
     {"system.cpu.icache.overall_misses", 151}, //formula
     {"system.l2.overall_misses", 152}, //formula
     {"system.l3.overall_misses", 153}, //formula
     {"system.cpu.dtb_walker_cache.overall_misses", 154},
     {"system.cpu.itb_walker_cache.overall_misses", 155}


});

void Mcpat::set_mcpat_stat(Stats::Info* stat, std::string name){

    switch(stat_map[name]) {
    case(24): //system.cpu.numCycles
        proc.XML->sys.core[0].total_cycles = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_numCycles;
        stat_storage.cpu_numCycles= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(25): //system.cpu.idleCycles
        proc.XML->sys.core[0].idle_cycles = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_idleCycles;
        stat_storage.cpu_idleCycles = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(2): //system.cpu.iq.iqInstsIssued
        proc.XML->sys.core[0].total_instructions = 
            1+ (static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_iq_iqInstsIssued);
        stat_storage.cpu_iq_iqInstsIssued = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(16):{ //system.cpu.iq.FU_type_0
        //to core 0 from thread 

        int new_val = 1;
        new_val = static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::No_OpClass]
                + static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::IntAlu]
                + static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::IntDiv]
                + static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::IprAccess];
        proc.XML->sys.core[0].int_instructions = 
            1 + (new_val - stat_storage.cpu_iq_FU_type_int_instructions);
        stat_storage.cpu_iq_FU_type_int_instructions = new_val;

        new_val = 0;
        new_val = static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::FloatAdd]
            + static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::FloatCmp]
            + static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::FloatCvt]
            + static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::FloatMult]
            + static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::FloatDiv]
            + static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::FloatSqrt];
        proc.XML->sys.core[0].fp_instructions = new_val - stat_storage.cpu_iq_FU_type_fp_instructions;
        stat_storage.cpu_iq_FU_type_fp_instructions = new_val;
        
        new_val = 0;
        new_val = static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::MemRead]
            + static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::InstPrefetch];
        proc.XML->sys.core[0].load_instructions = new_val - stat_storage.cpu_iq_FU_type_load_instructions;
        stat_storage.cpu_iq_FU_type_load_instructions = new_val;

        proc.XML->sys.core[0].store_instructions = static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::MemWrite]
            - stat_storage.cpu_iq_FU_type_store_instructions;
        stat_storage.cpu_iq_FU_type_store_instructions = static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::MemWrite];
        }
        break;

      case(17): //system.cpu.branchPred.condPredicted
        proc.XML->sys.core[0].branch_instructions = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_branchPred_condPredicted;
        stat_storage.cpu_branchPred_condPredicted = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(18): //system.cpu.branchPred.condIncorrect
        proc.XML->sys.core[0].branch_mispredictions = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_branchPred_condIncorrect;
        stat_storage.cpu_branchPred_condIncorrect = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(19): //system.cpu.commit.committedOps
        proc.XML->sys.core[0].committed_instructions = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_commit_committedOps;
        stat_storage.cpu_commit_committedOps = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(20): //system.cpu.commit.int_insts
        proc.XML->sys.core[0].committed_int_instructions = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_commit_int_insts;
        stat_storage.cpu_commit_int_insts = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(21): //system.cpu.commit.fp_insts
        proc.XML->sys.core[0].committed_fp_instructions = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_commit_fp_insts;
        stat_storage.cpu_commit_fp_insts = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(26): //system.cpu.rob.rob_reads
        proc.XML->sys.core[0].ROB_reads = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_rob_rob_reads;
        stat_storage.cpu_rob_rob_reads = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(27): //system.cpu.rob.rob_writes
        proc.XML->sys.core[0].ROB_writes = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_rob_rob_writes;
        stat_storage.cpu_rob_rob_writes = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(28): //system.cpu.rename.int_rename_lookups)
        proc.XML->sys.core[0].rename_reads = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_rename_int_rename_lookups;
        stat_storage.cpu_rename_int_rename_lookups = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(33): //system.cpu.rename.RenamedOperands
        stat_storage_prev.cpu_rename_RenamedOperands = stat_storage.cpu_rename_RenamedOperands;
        stat_storage.cpu_rename_RenamedOperands = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(34): //system.cpu.rename.fp_rename_lookups
        proc.XML->sys.core[0].fp_rename_reads = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_rename_fp_rename_lookups;
        stat_storage.cpu_rename_fp_rename_lookups = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(35): //system.cpu.rename.RenameLookups
        stat_storage_prev.cpu_rename_RenameLookups = stat_storage.cpu_rename_RenameLookups;
        stat_storage.cpu_rename_RenameLookups = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(36): //system.cpu.iq.int_inst_queue_reads
        proc.XML->sys.core[0].inst_window_reads = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_iq_int_inst_queue_reads;
        stat_storage.cpu_iq_int_inst_queue_reads = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(37): //system.cpu.iq.int_inst_queue_writes
        proc.XML->sys.core[0].inst_window_writes = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_iq_int_inst_queue_writes;
        stat_storage.cpu_iq_int_inst_queue_writes = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(38): //system.cpu.iq.int_inst_queue_wakeup_accesses
        proc.XML->sys.core[0].inst_window_wakeup_accesses = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_iq_int_inst_queue_wakeup_accesses;
        stat_storage.cpu_iq_int_inst_queue_wakeup_accesses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(39): //system.cpu.iq.fp_inst_queue_reads
        proc.XML->sys.core[0].fp_inst_window_reads = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_iq_fp_inst_queue_reads;
        stat_storage.cpu_iq_fp_inst_queue_reads = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(40): //system.cpu.iq.fp_inst_queue_writes
        proc.XML->sys.core[0].fp_inst_window_writes = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_iq_fp_inst_queue_writes;
        stat_storage.cpu_iq_fp_inst_queue_writes = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(41): //system.cpu.iq.fp_inst_queue_wakeup_accesses
        proc.XML->sys.core[0].fp_inst_window_wakeup_accesses = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_iq_fp_inst_queue_wakeup_accesses;
        stat_storage.cpu_iq_fp_inst_queue_wakeup_accesses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(42): //system.cpu.int_regfile_reads
        proc.XML->sys.core[0].int_regfile_reads = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_int_regfile_reads;
        stat_storage.cpu_int_regfile_reads = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(43): //system.cpu.fp_regfile_reads
        proc.XML->sys.core[0].float_regfile_reads = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_fp_regfile_reads;
        stat_storage.cpu_fp_regfile_reads = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(44): //system.cpu.int_regfile_writes
        proc.XML->sys.core[0].int_regfile_writes = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_int_regfile_writes;
        stat_storage.cpu_int_regfile_writes = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(45): //system.cpu.fp_regfile_writes
        proc.XML->sys.core[0].float_regfile_writes = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_fp_regfile_writes;
        stat_storage.cpu_fp_regfile_writes = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(46): //system.cpu.commit.function_calls
        proc.XML->sys.core[0].function_calls = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_commit_function_calls;
        stat_storage.cpu_commit_function_calls = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    
    case(47): //system.cpu.workload.num_syscalls
        proc.XML->sys.core[0].context_switches = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_workload_num_syscalls;
        stat_storage.cpu_workload_num_syscalls = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(51): //system.cpu.iq.int_alu_accesses
        proc.XML->sys.core[0].ialu_accesses = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_iq_int_alu_accesses;
        stat_storage.cpu_iq_int_alu_accesses = static_cast<Stats::ScalarInfo*>(stat)->result();

        proc.XML->sys.core[0].cdb_alu_accesses = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_iq_int_alu_accesses;
        break;

    case(50): //system.cpu.iq.fu_full
        proc.XML->sys.core[0].mul_accesses = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_iq_fu_full;
        stat_storage.cpu_iq_fu_full = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
        
    case(53): //system.cpu.iq.fp_alu_accesses
        proc.XML->sys.core[0].fpu_accesses = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_iq_fp_alu_accesses;
        proc.XML->sys.core[0].cdb_mul_accesses = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_iq_fp_alu_accesses;
        proc.XML->sys.core[0].cdb_fpu_accesses = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_iq_fp_alu_accesses;

        stat_storage.cpu_iq_fp_alu_accesses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;


    case(54): //system.cpu.itlb.tags.data_accesses
        proc.XML->sys.core[0].itlb.total_accesses = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_itlb_tags_data_accesses;
        stat_storage.cpu_itlb_tags_data_accesses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(55): //system.cpu.itlb.replacements
        proc.XML->sys.core[0].itlb.total_misses = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_itlb_replacements;
        stat_storage.cpu_itlb_replacements = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(56): //system.cpu.icache.ReadReq_accesses
        proc.XML->sys.core[0].icache.read_accesses = static_cast<Stats::VectorInfo*>(stat)->total() - stat_storage.cpu_icache_ReadReq_accesses;
        stat_storage.cpu_icache_ReadReq_accesses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(57): //system.cpu.icache.ReadReq_misses
        proc.XML->sys.core[0].icache.read_misses = static_cast<Stats::VectorInfo*>(stat)->total() - stat_storage.cpu_icache_ReadReq_misses;
        stat_storage.cpu_icache_ReadReq_misses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(58): //system.cpu.icache.replacements
        proc.XML->sys.core[0].icache.conflicts = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_icache_replacements;
        stat_storage.cpu_icache_replacements = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(59): //system.cpu.dtlb.tags.data_accesses
        proc.XML->sys.core[0].dtlb.total_accesses = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_dtlb_tags_data_accesses;
        stat_storage.cpu_dtlb_tags_data_accesses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(60): //system.cpu.dtlb.replacements
        proc.XML->sys.core[0].dtlb.total_misses = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_dtlb_replacements;
        stat_storage.cpu_dtlb_replacements = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(61): //system.cpu.dcache.ReadReq_accesses
        proc.XML->sys.core[0].dcache.read_accesses = static_cast<Stats::VectorInfo*>(stat)->total() - stat_storage.cpu_dcache_ReadReq_accesses;
        stat_storage.cpu_dcache_ReadReq_accesses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(62): //system.cpu.dcache.ReadReq_misses
        proc.XML->sys.core[0].dcache.read_misses = static_cast<Stats::VectorInfo*>(stat)->total() - stat_storage.cpu_dcache_ReadReq_misses;
        stat_storage.cpu_dcache_ReadReq_misses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(63): //system.cpu.dcache.WriteReq_accesses
        proc.XML->sys.core[0].dcache.write_accesses = static_cast<Stats::VectorInfo*>(stat)->total() - stat_storage.cpu_dcache_WriteReq_accesses;
        stat_storage.cpu_dcache_WriteReq_accesses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(64): //system.cpu.dcache.WriteReq_misses
        proc.XML->sys.core[0].dcache.write_misses = static_cast<Stats::VectorInfo*>(stat)->total() - stat_storage.cpu_dcache_WriteReq_misses;
        stat_storage.cpu_dcache_WriteReq_misses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(65): //system.cpu.dcache.replacements
        proc.XML->sys.core[0].dcache.conflicts = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_dcache_replacements;
        stat_storage.cpu_dcache_replacements = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(66): //system.cpu.branchPred.BTBLookups
        proc.XML->sys.core[0].BTB.read_accesses = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_branchPred_BTBLookups;
        stat_storage.cpu_branchPred_BTBLookups = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(67): //system.cpu.commit.branches
        proc.XML->sys.core[0].BTB.write_accesses = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_commit_branches;
        stat_storage.cpu_commit_branches = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(68): //system.l2.ReadExReq_accesses
        stat_storage_prev.l2_ReadExReq_accesses = stat_storage.l2_ReadExReq_accesses;
        stat_storage.l2_ReadExReq_accesses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(69): //system.l2.ReadCleanReq_accesses
        stat_storage_prev.l2_ReadCleanReq_accesses = stat_storage.l2_ReadCleanReq_accesses;
        stat_storage.l2_ReadCleanReq_accesses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(70): //system.l2.ReadSharedReq_accesses
        stat_storage_prev.l2_ReadSharedReq_accesses = stat_storage.l2_ReadSharedReq_accesses;
        stat_storage.l2_ReadSharedReq_accesses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(71): //system.l2.ReadCleanReq_misses
        stat_storage_prev.l2_ReadCleanReq_misses = stat_storage.l2_ReadCleanReq_misses;
        stat_storage.l2_ReadCleanReq_misses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(73): //system.l2.WritebackDirty_accesses
        stat_storage_prev.l2_WritebackDirty_accesses = stat_storage.l2_WritebackDirty_accesses;
        stat_storage.l2_WritebackDirty_accesses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(75): //system.l2.WritebackClean_accesses
        stat_storage_prev.l2_WritebackClean_accesses = stat_storage.l2_WritebackClean_accesses;
        stat_storage.l2_WritebackClean_accesses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(76): //system.l2.WritebackDirty_hits
        stat_storage_prev.l2_WritebackDirty_hits = stat_storage.l2_WritebackDirty_hits;
        stat_storage.l2_WritebackDirty_hits = static_cast<Stats::VectorInfo*>(stat)->total();
        break; 

    case(77): //system.l2.replacements
        proc.XML->sys.L2->conflicts = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.l2_replacements;
        stat_storage.l2_replacements = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(78): //system.l3.ReadExReq_accesses
        stat_storage_prev.l3_ReadExReq_accesses = stat_storage.l3_ReadExReq_accesses;
        stat_storage.l3_ReadExReq_accesses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(79): //system.l3.ReadCleanReq_accesses
        stat_storage_prev.l3_ReadCleanReq_accesses = stat_storage.l3_ReadCleanReq_accesses;
        stat_storage.l3_ReadCleanReq_accesses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(80): //system.l3.ReadSharedReq_accesses
        stat_storage_prev.l3_ReadSharedReq_accesses = stat_storage.l3_ReadSharedReq_accesses;
        stat_storage.l3_ReadSharedReq_accesses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(81): //system.l3.ReadCleanReq_misses
        stat_storage_prev.l3_ReadCleanReq_misses = stat_storage.l3_ReadCleanReq_misses;
        stat_storage.l3_ReadCleanReq_misses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(82): //system.l3.ReadExReq_misses
        stat_storage_prev.l3_ReadExReq_misses = stat_storage.l3_ReadExReq_misses;
        stat_storage.l3_ReadExReq_misses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(83): //system.l3.WritebackDirty_accesses
        stat_storage_prev.l3_WritebackDirty_accesses = stat_storage.l3_WritebackDirty_accesses;
        stat_storage.l3_WritebackDirty_accesses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(85): //system.l3.WritebackClean_accesses
        stat_storage_prev.l3_WritebackClean_accesses = stat_storage.l3_WritebackClean_accesses;
        stat_storage.l3_WritebackClean_accesses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(86): //system.l3.WritebackDirty_hits
        stat_storage_prev.l3_WritebackDirty_hits = stat_storage.l3_WritebackClean_accesses;
        stat_storage.l3_WritebackClean_accesses = static_cast<Stats::VectorInfo*>(stat)->total();
        break;

    case(87): //system.l3.replacements
        proc.XML->sys.L3->conflicts = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.l3_replacements;
        stat_storage.l3_replacements = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(90): //system.mem_ctrls.readReqs
        proc.XML->sys.mc.memory_reads = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.mem_ctrls_readReqs;
        stat_storage.mem_ctrls_readReqs = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(91): //system.mem_ctrls.writeReqs
        proc.XML->sys.mc.memory_writes = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.mem_ctrls_writeReqs;
        stat_storage.mem_ctrls_writeReqs = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    
    case(92): //system.membus.pkt_count
        stat_storage_prev.membus_pkt_count = stat_storage.membus_pkt_count;
        stat_storage.membus_pkt_count = static_cast<Stats::Vector2dInfo*>(stat)->total();
        break;

    case(93): //system.tol2bus.pkt_count
        stat_storage_prev.tol2bus_pkt_count = stat_storage.tol2bus_pkt_count;
        stat_storage.tol2bus_pkt_count = static_cast<Stats::Vector2dInfo*>(stat)->total();
        break;

    case(94): //system.tol3bus.pkt_count
        stat_storage_prev.tol3bus_pkt_count = stat_storage.tol3bus_pkt_count;
        stat_storage.tol3bus_pkt_count = static_cast<Stats::Vector2dInfo*>(stat)->total();
        break;

    case(95): //system.cpu.branchPred.indirectHits
        proc.XML->sys.core[0].BTB.read_accesses = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_branchPred_indirectHits;
        stat_storage.cpu_branchPred_indirectHits = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;

    case(96): //system.cpu.branchPred.indirectMisses
        proc.XML->sys.core[0].BTB.read_accesses = static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_branchPred_indirectMisses;
        stat_storage.cpu_branchPred_indirectMisses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    

    //  {"system.dcache.overall_misses", 92},
    //  {"system.icache.overall_misses", 93},
    //  {"system.l2.overall_misses", 94},
    //  {"system.l3.overall_misses", 95},
    //  {"system.cpu.dtb_walker_cache.overall_misses", 96},
    //  {"system.cpu.itb_walker_cache.overall_misses", 97}

    case(150): //system.dcache.overall_misses
        // std::cout << "***dcache: " << static_cast<Stats::VectorInfo*>(stat)->total() << "\n";
        if (stat_storage.cpu_dcache_overall_misses != static_cast<Stats::VectorInfo*>(stat)->total()){
            if (powerPred) {
                powerPred->historyInsert(PPred::DCACHE_MISS);
            }
            stat_storage.cpu_dcache_overall_misses = static_cast<Stats::VectorInfo*>(stat)->total();
        }
        break;
    case(151): //system.icache.overall_misses
        // std::cout << "***icache: " << static_cast<Stats::VectorInfo*>(stat)->total() << "\n";
        if (stat_storage.cpu_icache_overall_misses != static_cast<Stats::VectorInfo*>(stat)->total()){
            if (powerPred) {
                powerPred->historyInsert(PPred::ICACHE_MISS);
            }
            stat_storage.cpu_icache_overall_misses = static_cast<Stats::VectorInfo*>(stat)->total();
        }
        break;
    case(152): //system.l2.overall_misses
        // std::cout << "***l2: " << static_cast<Stats::VectorInfo*>(stat)->total() << "\n";
        if (stat_storage.l2_overall_misses != static_cast<Stats::VectorInfo*>(stat)->total()){
            if (powerPred) {
                powerPred->historyInsert(PPred::L2_MISS);
            }
            stat_storage.l2_overall_misses = static_cast<Stats::VectorInfo*>(stat)->total();
        }
        break;
    case(153): //system.l3.overall_misses
        // std::cout << "***l3: " << static_cast<Stats::VectorInfo*>(stat)->total() << "\n";
        if (stat_storage.l3_overall_misses != static_cast<Stats::VectorInfo*>(stat)->total()){
            if (powerPred) {
                powerPred->historyInsert(PPred::L3_MISS);
            }
            stat_storage.l3_overall_misses = static_cast<Stats::VectorInfo*>(stat)->total();
        }
        break;
    case(154): //system.cpu.dtb_walker_cache.overall_misses
        // std::cout << "***dtb_walker_cache: " << static_cast<Stats::VectorInfo*>(stat)->total() << "\n";
        if (stat_storage.cpu_dtb_walker_cache_overall_misses != static_cast<Stats::VectorInfo*>(stat)->total()){
            if (powerPred) {
                powerPred->historyInsert(PPred::DTLB_MISS);
            }
            stat_storage.cpu_dtb_walker_cache_overall_misses = static_cast<Stats::VectorInfo*>(stat)->total();
        }
        break;
    case(155): //system.cpu.itb_walker_cache.overall_misses
        // std::cout << "***itb_walker_cache: " << static_cast<Stats::VectorInfo*>(stat)->total() << "\n";
        if (stat_storage.cpu_itb_walker_cache_overall_misses != static_cast<Stats::VectorInfo*>(stat)->total()){
            if (powerPred) {
                powerPred->historyInsert(PPred::ITLB_MISS);
            }
            stat_storage.cpu_itb_walker_cache_overall_misses = static_cast<Stats::VectorInfo*>(stat)->total();
        }
        break;

    default:
        break;
    }
}




void Mcpat::print_stats(Stats::Info* stat){
    switch(stat_map[stat->name]) {
        case(2): //system.cpu.iq.iqInstsIssued
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(16): //system.cpu.iq.FU_type
            //????? CUSTOM FUNCTION NEEDED
            break;
        case(17): //system.cpu.branchPred.condPredicted
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(18): //system.cpu.branchPred.condIncorrect
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(19): //system.cpu.commit.committedOps
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(20): //system.cpu.commit.int_insts
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(21): //system.cpu.commit.fp_insts
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(24): //system.cpu.numCycles
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(25): //system.cpu.idleCycles
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(26): //system.cpu.rob.rob_reads
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(27): //system.cpu.rob.rob_writes
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(28): //system.cpu.rename.int_rename_lookups)
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(33): //system.cpu.rename.RenamedOperands
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(34): //system.cpu.rename.fp_rename_lookups
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(36): //system.cpu.iq.int_inst_queue_reads
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(37): //system.cpu.iq.int_inst_queue_writes
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(38): //system.cpu.iq.int_inst_queue_wakeup_accesses
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(39): //system.cpu.iq.fp_inst_queue_reads
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(40): //system.cpu.iq.fp_inst_queue_writes
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(41): //system.cpu.iq.fp_inst_queue_wakeup_accesses
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(42): //system.cpu.int_regfile_reads
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(43): //system.cpu.fp_regfile_reads
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(44): //system.cpu.int_regfile_writes
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(45): //system.cpu.fp_regfile_writes
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(46): //system.cpu.commit.function_calls
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(47): //system.cpu.workload.num_syscalls
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(50): //system.cpu.iq.fu_full
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(51): //system.cpu.iq.int_alu_accesses
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(53): //system.cpu.iq.fp_alu_accesses
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(54): //system.cpu.itlb.tags.data_accesses
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(55): //system.cpu.itlb.replacements
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(56): //system.cpu.icache.ReadReq_accesses
            std::cout << stat->name << " : " << static_cast<Stats::VectorInfo*>(stat)->total() << "\n";
            break;
        case(57): //system.cpu.icache.ReadReq_misses
            std::cout << stat->name << " : " << static_cast<Stats::VectorInfo*>(stat)->total() << "\n";
            break;
        case(58): //system.cpu.icache.replacements
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(59): //system.cpu.dtlb.tags.data_accesses
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(60): //system.cpu.dtlb.replacements
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(61): //system.cpu.dcache.ReadReq_accesses
            std::cout << stat->name << " : " << static_cast<Stats::VectorInfo*>(stat)->total() << "\n";
            break;
        case(62): //system.cpu.dcache.ReadReq_misses
            std::cout << stat->name << " : " << static_cast<Stats::VectorInfo*>(stat)->total() << "\n";
            break;
        case(63): //system.cpu.dcache.WriteReq_accesses
            std::cout << stat->name << " : " << static_cast<Stats::VectorInfo*>(stat)->total() << "\n";
            break;
        case(64): //system.cpu.dcache.WriteReq_misses
            std::cout << stat->name << " : " << static_cast<Stats::VectorInfo*>(stat)->total() << "\n";
            break;
        case(65): //system.cpu.dcache.replacements
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(66): //system.cpu.branchPred.BTBLookups
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(67): //system.cpu.commit.branches
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(68): //system.l2.ReadExReq_accesses
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(71): //system.l2.ReadCleanReq_misses
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(73): //system.l2.WritebackDirty_accesses
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(75): //system.l2.WritebackClean_accesses
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(77): //system.l2.replacements
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(78): //system.l3.ReadExReq_accesses
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(81): //system.l3.ReadCleanReq_misses
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(83): //system.l3.WritebackDirty_accesses
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(85): //system.l3.WritebackClean_accesses
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(87): //system.l3.replacements
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(90): //system.mem_ctrls.readReqs
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        case(91): //system.mem_ctrls.writeReqs
            std::cout << stat->name << " : " << static_cast<Stats::ScalarInfo*>(stat)->result() << "\n";
            break;
        default:
            break;
    }
}
