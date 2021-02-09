# # Copyright (c) 2005 The Regents of The University of Michigan
# # All rights reserved.
# #
# # Redistribution and use in source and binary forms, with or without
# # modification, are permitted provided that the following conditions are
# # met: redistributions of source code must retain the above copyright
# # notice, this list of conditions and the following disclaimer;
# # redistributions in binary form must reproduce the above copyright
# # notice, this list of conditions and the following disclaimer in the
# # documentation and/or other materials provided with the distribution;
# # neither the name of the copyright holders nor the names of its
# # contributors may be used to endorse or promote products derived from
# # this software without specific prior written permission.
# #
# # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# # "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# # LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# # A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# # OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# # SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# # LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# # DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# # THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# #
# # Authors: Nathan Binkert

# import _m5.mcpat_internal as mcpat_internal
# from m5.mcpat import autogen
# import os

# first_time = True



# btb_stats = \
# {
#   "read_accesses" :   \
#     ["0","Lookups into BTB; branchPred.BTBLookups"],
#   "write_accesses" :   \
#     ["0","Number of Updates to the CAM; commit.branches"],
# }
# cache_stats = \
# {
#   "duty_cycle" :   \
#     ["1.0",""],
#   "read_accesses" :   \
#     ["0", "Cache Read Accesses Total"],
#   "read_misses" :   \
#     ["0", "Cache Read Req Misses Total"],
#   "write_accesses" :   \
#     ["0", "Cache Write Accesses Total"],
#   "write_misses" :   \
#     ["0", "Cache Write Req Misses Total"],
#   "conflicts" :   \
#     ["0", "Cache Replacements"]
# }
# core_stats = \
# {
#   "total_instructions" : ["0","cpu.iq.iqInstsIssued"],
#   "int_instructions" :   \
#     ["0","iq.FU_type_0::No_OpClass + iq.FU_type_0::IntAlu +"
#       "iq.FU_type_0::IntMult + iq.FU_type_0::IntDiv +"
#       "iq.FU_type_0::IprAccess"],
#   "fp_instructions" :   \
#     ["0","cpu.iq.FU_type_0::FloatAdd + cpu.iq.FU_type_0::FloatCmp"
#       "+ cpu.iq.FU_type_0::FloatCvt + cpu.iq.FU_type_0::FloatMult +"
#       "cpu.iq.FU_type_0::FloatDiv + cpu.iq.FU_type_0::FloatSqrt"],
#   "branch_instructions" : ["0","cpu.branchPred.condPredicted"],
#   "branch_mispredictions" : ["0","cpu.branchPred.condIncorrect"],
#   "load_instructions" :   \
#     ["0","cpu.iq.FU_type_0::MemRead + cpu.iq.FU_type_0::InstPrefetch"],
#   "store_instructions" : ["0","cpu.iq.FU_type_0::MemWrite"],
#   "committed_instructions" : ["0","cpu.commit.committedOps"],
#   "committed_int_instructions" : ["0","cpu.commit.int_insts"],
#   "committed_fp_instructions" : ["0","cpu.commit.fp_insts"],
#   "pipeline_duty_cycle" :   \
#     ["1","<=1, runtime_ipc/peak_ipc; averaged for all cores if"
#       "homogeneous"],
#   "total_cycles" : ["1","cpu.numCycles"],
#   "idle_cycles" : ["0","cpu.idleCycles"],
#   "busy_cycles" : ["1","cpu.numCycles - cpu.idleCycles"],
#   "ROB_reads" : ["0","cpu.rob.rob_reads"],
#   "ROB_writes" : ["0","cpu.rob.rob_writes"],
#   "rename_reads" :   \
#     ["0","lookup in renaming logic (cpu.rename.int_rename_lookups)"],
#   "rename_writes" :   \
#     ["0","cpu.rename.RenamedOperands * "
#       "cpu.rename.int_rename_lookups / cpu.rename.RenameLookups"],
#   "fp_rename_reads" : ["0","cpu.rename.fp_rename_lookups"],
#   "fp_rename_writes" :   \
#     ["0","cpu.rename.RenamedOperands * "
#       "cpu.rename.fp_rename_lookups / cpu.rename.RenameLookups"],
#   "inst_window_reads" : ["0","cpu.iq.int_inst_queue_reads"],
#   "inst_window_writes" : ["0","cpu.iq.int_inst_queue_writes"],
#   "inst_window_wakeup_accesses" :   \
#     ["0","cpu.iq.int_inst_queue_wakeup_accesses"],
#   "fp_inst_window_reads" : ["0","cpu.iq.fp_inst_queue_reads"],
#   "fp_inst_window_writes" : ["0","cpu.iq.fp_inst_queue_writes"],
#   "fp_inst_window_wakeup_accesses" :   \
#     ["0","cpu.iq.fp_inst_queue_wakeup_accesses"],
#   "int_regfile_reads" : ["0","cpu.int_regfile_reads"],
#   "float_regfile_reads" : ["0","cpu.fp_regfile_reads"],
#   "int_regfile_writes" : ["1","cpu.int_regfile_writes"],
#   "float_regfile_writes" : ["1","cpu.fp_regfile_writes"],
#   "function_calls" : ["0","cpu.commit.function_calls"],
#   "context_switches" : ["0","cpu.workload.num_syscalls"],
#   "ialu_accesses" : ["1","cpu.iq.int_alu_accesses"],
#   "fpu_accesses" : ["1","cpu.iq.fp_alu_accesses"],
#   "mul_accesses" : ["1","cpu.iq.fu_full::FloatMult"],
#   "cdb_alu_accesses" : ["1","cpu.iq.int_alu_accesses"],
#   "cdb_mul_accesses" : ["1","cpu.iq.fp_alu_accesses"],
#   "cdb_fpu_accesses" : ["1","cpu.iq.fp_alu_accesses"],
#   "IFU_duty_cycle" : ["0.25",""],
#   "LSU_duty_cycle" : ["0.25",""],
#   "MemManU_I_duty_cycle" : ["0.25",""],
#   "MemManU_D_duty_cycle" : ["0.25",""],
#   "ALU_duty_cycle" : ["1",""],
#   "MUL_duty_cycle" : ["0.3",""],
#   "FPU_duty_cycle" : ["0.3",""],
#   "ALU_cdb_duty_cycle" : ["1",""],
#   "MUL_cdb_duty_cycle" : ["0.3",""],
#   "FPU_cdb_duty_cycle" : ["0.3",""]
# }
# directory_stats = \
# {
#   "read_accesses" : ["0","Read Accesses to the directory controller"],
#   "write_accesses" : ["0","Write Accesses to the directory controller"],
#   "read_misses" : ["0","Read Misses"],
#   "write_misses" : ["0","Write Misses"],
#   "conflicts" : ["0","Conflicts"]
# }
# flash_stats = \
# {
#   "duty_cycle" : ["1.0","achievable max load <= 1.0"],
#   "total_load_perc" :   \
#     ["0.0","Percentage of total achived load to total achivable"
#       "bandwidth"]
# }
# mc_stats = \
# {
#   "memory_accesses" :   \
#     ["0","mem_ctrls.writeReqs + mem_ctrls.readReqs"],
#   "memory_reads" : ["0","mem_ctrls.readReqs"],
#   "memory_writes" : ["0","mem_ctrls.writeReqs"]
# }
# niu_stats = \
# {
#   "duty_cycle" : ["1.0","achievable max load lteq 1.0"],
#   "total_load_perc" :   \
#     ["0.0","ratio of total achived load to total achivable bandwidth"]
# }
# noc_stats = \
# {
#   "total_accesses" :   \
#     ["1","This is the number of total accesses within the whole "
#       "network not for each router (pkt_count::total & bus in key)"],
#   "duty_cycle" : ["1",""]
# }
# pcie_stats = \
# {
#   "duty_cycle" : ["1.0","achievable max load <= 1.0"],
#   "total_load_perc" :   \
#     ["0.0","Percentage of total achived load to total achivable "
#       "bandwidth"]
# }
# system_stats = \
# {
#   "total_cycles" : ["1", "Total CPU Cycles"],
#   "idle_cycles" : ["1", "Total Idle Cycles"],
#   "busy_cycles" : ["0", "Total Busy Cycles (Total - Idle)"]
# }
# tlb_stats = \
# {
#   "total_accesses" :   \
#     ["","Total Acceses; dtb_walker_cache.tags.data_accesses"],
#   "total_misses" :   \
#     ["","Total Misses; dtb_walker_cache.tags.data_accesses"],
#   "conflicts" : ["0","Conflicts to entries in the TLB"],
# }

# def test():
#   from m5 import options

#   mcpat_output_path = os.path.join(options.mcpat_out, options.mcpat_testname)
#   mcpat_serial = os.path.join(mcpat_output_path, "mcpat_serial.txt")
#   if(options.mcpat_save_space):
#       i_f = os.path.join(mcpat_output_path,"mp.xml")
#       # o_f = os.path.join(mcpat_output_path,"mp.out")
#       # e_f = os.path.join(mcpat_output_path,"mp.err")
#   else:
#       i_f = os.path.join(mcpat_output_path,"mp_"+str(iter)+".xml")
#       # o_f = os.path.join(mcpat_output_path,"mp_"+str(iter)+".out")
#       # e_f = os.path.join(mcpat_output_path,"mp_"+str(iter)+".err")

#   mp = mcpat_internal.Mcpat()

#   if first_time:
#     mp.init(i_f,mcpat_serial)
#   else:
#     pass

# def update_stats(stat_strings):
#   stat_dict = autogen.util.build_gem5_stat_dict(build_gem5_stat_dict)



