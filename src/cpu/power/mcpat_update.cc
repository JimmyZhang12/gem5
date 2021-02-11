#include <string>
#include "mcpat.hh"
#include "enums/OpClass.hh"

//TODO, case conditions really should be enums to avoid magic numbers
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
     {"system.cpu.rename.int_rename_lookups)" , 28},
     {"system.cpu.rename.RenamedOperands" , 33},
     {"system.cpu.rename.int_rename_lookups" , 30},
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
     {"system.cpu.icache.ReadReq_accesses" , 56},
     {"system.cpu.icache.ReadReq_misses" , 57},
     {"system.cpu.icache.replacements" , 58},
     {"system.cpu.dtlb.tags.data_accesses" , 59},
     {"system.cpu.dtlb.replacements" , 60},
     {"system.cpu.dcache.ReadReq_accesses" , 61},
     {"system.cpu.dcache.ReadReq_misses" , 62},
     {"system.cpu.dcache.WriteReq_accesses" , 63},
     {"system.cpu.dcache.WriteReq_misses" , 64},
     {"system.cpu.dcache.replacements" , 65},
     {"system.cpu.branchPred.BTBLookups" , 66},
     {"system.cpu.commit.branches" , 67},
     {"system.l2.ReadExReq_accesses" , 68},
     {"system.l2.ReadCleanReq_accesses" , 69},
     {"system.l2.ReadSharedReq_accesses" , 70},
     {"system.l2.ReadCleanReq_misses" , 71},
     {"system.l2.ReadExReq_misses" , 72},
     {"system.l2.WritebackDirty_accesses" , 73},
     {"system.l2.WritebackClean_accesses" , 75},
     {"system.l2.WritebackDirty_hits" , 76},
     {"system.l2.replacements" , 77},
     {"system.l3.ReadExReq_accesses" , 78},
     {"system.l3.ReadCleanReq_accesses" , 79},
     {"system.l3.ReadSharedReq_accesses" , 80},
     {"system.l3.ReadCleanReq_misses" , 81},
     {"system.l3.ReadExReq_misses" , 82},
     {"system.l3.WritebackDirty_accesses" , 83},
     {"system.l3.WritebackClean_accesses" , 85},
     {"system.l3.WritebackDirty_hits" , 86},
     {"system.l3.replacements" , 87},
     {"system.mem_ctrls.writeReqs" , 91},
     {"system.mem_ctrls.readReqs" , 90}
});




void Mcpat::set_mcpat_stat(Stats::Info* stat){
    switch(stat_map[stat->name]) {
    case(24): //system.cpu.numCycles
        stat_storage.numCycles = static_cast<Stats::ScalarInfo*>(stat)->result();
        proc.XML->sys.core[0].total_cycles= stat_storage.numCycles;
        break;
    case(25): //system.cpu.idleCycles
        stat_storage.idleCycles = static_cast<Stats::ScalarInfo*>(stat)->result();
        proc.XML->sys.core[0].idle_cycles= stat_storage.idleCycles;
        break;
    case(2): //system.cpu.iq.iqInstsIssued
        proc.XML->sys.core[0].total_instructions = static_cast<Stats::ScalarInfo*>(stat)->result() + 1;
        break;
    case(16): //system.cpu.iq.FU_type_0
        //to core 0 from thread 0
        proc.XML->sys.core[0].int_instructions = 1;
        proc.XML->sys.core[0].int_instructions += static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::No_OpClass];
        proc.XML->sys.core[0].int_instructions += static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::IntAlu];
        proc.XML->sys.core[0].int_instructions += static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::IntDiv];
        proc.XML->sys.core[0].int_instructions += static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::IprAccess];

        proc.XML->sys.core[0].fp_instructions = 0;
        proc.XML->sys.core[0].fp_instructions += static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::FloatAdd];
        proc.XML->sys.core[0].fp_instructions += static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::FloatCmp];
        proc.XML->sys.core[0].fp_instructions += static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::FloatCvt];
        proc.XML->sys.core[0].fp_instructions += static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::FloatMult];
        proc.XML->sys.core[0].fp_instructions += static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::FloatDiv];
        proc.XML->sys.core[0].fp_instructions += static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::FloatSqrt];
        
        proc.XML->sys.core[0].load_instructions = 0;
        proc.XML->sys.core[0].load_instructions += static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::MemRead];
        proc.XML->sys.core[0].load_instructions += static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::InstPrefetch];

        proc.XML->sys.core[0].store_instructions = static_cast<Stats::Vector2dInfo*>(stat)->cvec[Enums::MemWrite];
        break;
    case(17): //system.cpu.branchPred.condPredicted
        proc.XML->sys.core[0].branch_instructions= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(18): //system.cpu.branchPred.condIncorrect
        proc.XML->sys.core[0].branch_mispredictions= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(19): //system.cpu.commit.committedOps
        proc.XML->sys.core[0].committed_instructions= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(20): //system.cpu.commit.int_insts
        proc.XML->sys.core[0].committed_int_instructions= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(21): //system.cpu.commit.fp_insts
        proc.XML->sys.core[0].committed_fp_instructions= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(26): //system.cpu.rob.rob_reads
        proc.XML->sys.core[0].ROB_reads= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(27): //system.cpu.rob.rob_writes
        proc.XML->sys.core[0].ROB_writes= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(28): //system.cpu.rename.int_rename_lookups
        stat_storage.int_rename_lookups = static_cast<Stats::ScalarInfo*>(stat)->result();
        proc.XML->sys.core[0].rename_reads= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(33): //system.cpu.rename.RenamedOperands
        stat_storage.RenamedOperands = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(34): //system.cpu.rename.fp_rename_lookups
        stat_storage.fp_rename_lookups = static_cast<Stats::ScalarInfo*>(stat)->result();
        proc.XML->sys.core[0].fp_rename_reads= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(35): //system.cpu.rename.RenameLookups
        stat_storage.RenameLookups = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(36): //system.cpu.iq.int_inst_queue_reads
        proc.XML->sys.core[0].inst_window_reads= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(37): //system.cpu.iq.int_inst_queue_writes
        proc.XML->sys.core[0].inst_window_writes= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(38): //system.cpu.iq.int_inst_queue_wakeup_accesses
        proc.XML->sys.core[0].inst_window_wakeup_accesses= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(39): //system.cpu.iq.fp_inst_queue_reads
        proc.XML->sys.core[0].fp_inst_window_reads= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(40): //system.cpu.iq.fp_inst_queue_writes
        proc.XML->sys.core[0].fp_inst_window_writes= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(41): //system.cpu.iq.fp_inst_queue_wakeup_accesses
        proc.XML->sys.core[0].fp_inst_window_wakeup_accesses= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(42): //system.cpu.int_regfile_reads
        proc.XML->sys.core[0].int_regfile_reads= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(43): //system.cpu.fp_regfile_reads
        proc.XML->sys.core[0].float_regfile_reads= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(44): //system.cpu.int_regfile_writes
        proc.XML->sys.core[0].int_regfile_writes= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(45): //system.cpu.fp_regfile_writes
        proc.XML->sys.core[0].float_regfile_writes= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(46): //system.cpu.commit.function_calls
        proc.XML->sys.core[0].function_calls= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(47): //system.cpu.workload.num_syscalls
        proc.XML->sys.core[0].context_switches= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(51): //system.cpu.iq.int_alu_accesses
        proc.XML->sys.core[0].ialu_accesses= static_cast<Stats::ScalarInfo*>(stat)->result();
        proc.XML->sys.core[0].cdb_alu_accesses= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(50): //system.cpu.iq.fu_full
        proc.XML->sys.core[0].mul_accesses= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(53): //system.cpu.iq.fp_alu_accesses
        proc.XML->sys.core[0].fpu_accesses= static_cast<Stats::ScalarInfo*>(stat)->result();
        proc.XML->sys.core[0].cdb_mul_accesses= static_cast<Stats::ScalarInfo*>(stat)->result();
        proc.XML->sys.core[0].cdb_fpu_accesses= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(54): //system.cpu.itlb.tags.data_accesses
        proc.XML->sys.core[0].itlb.total_accesses= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(55): //system.cpu.itlb.replacements
        proc.XML->sys.core[0].itlb.total_misses= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(56): //system.cpu.icache.ReadReq_accesses
        proc.XML->sys.core[0].icache.read_accesses= static_cast<Stats::VectorInfo*>(stat)->total();
        break;
    case(57): //system.cpu.icache.ReadReq_misses
        proc.XML->sys.core[0].icache.read_misses= static_cast<Stats::VectorInfo*>(stat)->total();
        break;
    case(58): //system.cpu.icache.replacements
        proc.XML->sys.core[0].icache.conflicts= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(59): //system.cpu.dtlb.tags.data_accesses
        proc.XML->sys.core[0].dtlb.total_accesses= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(60): //system.cpu.dtlb.replacements
        proc.XML->sys.core[0].dtlb.total_misses= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(61): //system.cpu.dcache.ReadReq_accesses
        proc.XML->sys.core[0].dcache.read_accesses= static_cast<Stats::VectorInfo*>(stat)->total();
        break;
    case(62): //system.cpu.dcache.ReadReq_misses
        proc.XML->sys.core[0].dcache.read_misses= static_cast<Stats::VectorInfo*>(stat)->total();
        break;
    case(63): //system.cpu.dcache.WriteReq_accesses
        proc.XML->sys.core[0].dcache.write_accesses= static_cast<Stats::VectorInfo*>(stat)->total();
        break;
    case(64): //system.cpu.dcache.WriteReq_misses
        proc.XML->sys.core[0].dcache.write_misses= static_cast<Stats::VectorInfo*>(stat)->total();
        break;
    case(65): //system.cpu.dcache.replacements
        proc.XML->sys.core[0].dcache.conflicts= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(66): //system.cpu.branchPred.BTBLookups
        proc.XML->sys.core[0].BTB.read_accesses= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(67): //system.cpu.commit.branches
        proc.XML->sys.core[0].BTB.write_accesses= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(68): //system.l2.ReadExReq_accesses
        stat_storage.l2_ReadExReq_accesses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(69): //system.l2.ReadCleanReq_accesses
        stat_storage.l2_ReadCleanReq_accesses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(70): //system.l2.ReadSharedReq_accesses
        stat_storage.l2_ReadSharedReq_accesses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(71): //system.l2.ReadCleanReq_misses
        stat_storage.l2_ReadCleanReq_misses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(72): //system.l2.ReadExReq_misses
        stat_storage.l2_ReadExReq_misses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(73): //system.l2.WritebackDirty_accesses
        stat_storage.l2_WritebackDirty_accesses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(75): //system.l2.WritebackClean_accesses
        stat_storage.l2_WritebackClean_accesses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(76): //system.l2.WritebackDirty_hits
        stat_storage.l2_WritebackDirty_hits = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;     
    case(77): //system.l2.replacements
        proc.XML->sys.L2[0].conflicts= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(78): //system.l3.ReadExReq_accesses
        stat_storage.l3_ReadExReq_accesses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(79): //system.l3.ReadCleanReq_accesses
        stat_storage.l3_ReadCleanReq_accesses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(80): //system.l3.ReadSharedReq_accesses
        stat_storage.l3_ReadSharedReq_accesses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(81): //system.l3.ReadCleanReq_misses
        stat_storage.l3_ReadCleanReq_misses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(82): //system.l3.ReadExReq_misses
        stat_storage.l3_ReadExReq_misses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(83): //system.l3.WritebackDirty_accesses
        stat_storage.l3_WritebackDirty_accesses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(85): //system.l3.WritebackClean_accesses
        stat_storage.l3_WritebackClean_accesses = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(86): //system.l3.WritebackDirty_hits
        stat_storage.l3_WritebackDirty_hits = static_cast<Stats::ScalarInfo*>(stat)->result();
        break;  
    case(87): //system.l3.replacements
        proc.XML->sys.L3[0].conflicts= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(90): //system.mem_ctrls.readReqs
        proc.XML->sys.mc.memory_reads= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    case(91): //system.mem_ctrls.writeReqs
        proc.XML->sys.mc.memory_writes= static_cast<Stats::ScalarInfo*>(stat)->result();
        break;
    default:
        break;
    }
}