var = '''
     cpu_numCycles" , 24},
     cpu_idleCycles" , 25},
     cpu_iq_iqInstsIssued" , 2},
     cpu_iq_FU_type" , 16},
     cpu_branchPred_condPredicted" , 17},
     cpu_branchPred_condIncorrect" , 18},
     cpu_commit_committedOps" , 19},
     cpu_commit_int_insts" , 20},
     cpu_commit_fp_insts" , 21},
     cpu_rob_rob_reads" , 26},
     cpu_rob_rob_writes" , 27},
     cpu_rename_int_rename_lookups" , 28},
     cpu_rename_RenamedOperands" , 33},
     cpu_rename_RenameLookups" , 35},
     cpu_rename_fp_rename_lookups" , 34},
     cpu_iq_int_inst_queue_reads" , 36},
     cpu_iq_int_inst_queue_writes" , 37},
     cpu_iq_int_inst_queue_wakeup_accesses" , 38},
     cpu_iq_fp_inst_queue_reads" , 39},
     cpu_iq_fp_inst_queue_writes" , 40},
     cpu_iq_fp_inst_queue_wakeup_accesses" , 41},
     cpu_int_regfile_reads" , 42},
     cpu_fp_regfile_reads" , 43},
     cpu_int_regfile_writes" , 44},
     cpu_fp_regfile_writes" , 45},
     cpu_commit_function_calls" , 46},
     cpu_workload_num_syscalls" , 47},
     cpu_iq_int_alu_accesses" , 51},
     cpu_iq_fp_alu_accesses" , 53},
     cpu_iq_fu_full" , 50},
     cpu_itlb_tags_data_accesses" , 54},
     cpu_itlb_replacements" , 55},
     cpu_icache_ReadReq_accesses" , 56}, //formula
     cpu_icache_ReadReq_misses" , 57},
     cpu_icache_replacements" , 58},
     cpu_dtlb_tags_data_accesses" , 59},
     cpu_dtlb_replacements" , 60},
     cpu_dcache_ReadReq_accesses" , 61}, //formula
     cpu_dcache_ReadReq_misses" , 62},
     cpu_dcache_WriteReq_accesses" , 63}, //formula
     cpu_dcache_WriteReq_misses" , 64},
     cpu_dcache_replacements" , 65},
     cpu_branchPred_BTBLookups" , 66},
     cpu_commit_branches" , 67},
     l2_ReadExReq_accesses" , 68}, //formula
     l2_ReadCleanReq_accesses" , 69}, //formula
     l2_ReadSharedReq_accesses" , 70}, //formula
     l2_ReadCleanReq_misses" , 71},
     l2_ReadExReq_misses" , 72},
     l2_WritebackDirty_accesses" , 73}, //formula
     l2_WritebackClean_accesses" , 75}, //formula
     l2_WritebackDirty_hits" , 76},
     l2_replacements" , 77},
     l3_ReadExReq_accesses" , 78}, //formula
     l3_ReadCleanReq_accesses" , 79}, //formula
     l3_ReadSharedReq_accesses" , 80}, //formula
     l3_ReadCleanReq_misses" , 81},
     l3_ReadExReq_misses" , 82},
     l3_WritebackDirty_accesses" , 83}, //formula
     l3_WritebackClean_accesses" , 85}, //formula
     l3_WritebackDirty_hits" , 86},
     l3_replacements" , 87},
     mem_ctrls_readReqs" , 90},
     mem_ctrls_writeReqs" , 91},
     membus_pkt_count", 92},
     tol2bus_pkt_count", 93}, 
     tol3bus_pkt_count", 94},
     cpu_branchPred_indirectHits", 95},
     cpu_branchPred_indirectMisses", 96},
     //also use stats to send events to power predictors
     cpu_dcache_overall_misses", 150}, //formula
     cpu_icache_overall_misses", 151}, //formula
     l2_overall_misses", 152}, //formula
     l3_overall_misses", 153}, //formula
     cpu_dtb_walker_cache_overall_misses", 154},
     cpu_itb_walker_cache_overall_misses", 155} '''

#     case(25): //system.cpu.idleCycles
#         proc.XML->sys.core[0].idle_cycles= static_cast<Stats::ScalarInfo*>(stat)->result() - stat_storage.cpu_idleCycles;
#         stat_storage.cpu_idleCycles = static_cast<Stats::ScalarInfo*>(stat)->result();
#         break;

var = var.split('\n')
for i in var:
     pr = "stat_storage."+ i.split('"')[0].strip()
     print(pr)