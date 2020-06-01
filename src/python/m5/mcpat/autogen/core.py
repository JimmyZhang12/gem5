# Copyright (c) 2020 University of Illinois
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors: Andrew Smith
#
# core.py
#
# Core component

from xml.etree import ElementTree
from xml.dom import minidom

from tlb import TLB
from btb import BTB
from cache import *
from branchpred import Predictor

from util import *

class Core:
  def __init__(self, component_id, component_name, \
                stat_dict, config_dict, sim_dict):
    self.name = "core"
    self.id = "core"

    self.parameters = \
    {
      "clock_rate" : ["1000","Clock Rate in MHz"],
      "vdd" : ["0","0 means using ITRS default vdd"],
      "power_gating_vcc" : \
        ["-1","\"-1\" means using default power gating virtual power"
          "supply voltage constrained by technology and computed"
          "automatically"],
      "opt_local" : \
        ["0","for cores with unknown timing, set to 0 to"
          "force off the opt flag"],
      "instruction_length" : ["32",""],
      "opcode_width" : ["16",""],
      "x86" : ["1",""],
      "micro_opcode_width" : ["8",""],
      "machine_type" : \
        ["0","Specifies the machine type inorder/OoO; 1"
          "inorder; 0 OOO"],
      "number_hardware_threads" : \
        ["2","number_instruction_fetch_ports(icache ports) is always 1"
          "in single-thread processor, it only may be more than one in"
          "SMT processors. BTB ports always equals to fetch ports since"
          "branch information in consecutive branch instructions in the"
          "same fetch group can be read out from BTB once."
          "(cpu.numThreads)"],
      "fetch_width" : ["16","(cpu.fetchWidth)"],
      "number_instruction_fetch_ports" : \
        ["1","fetch_width determines the size of cachelines of L1"
          "cache block"],
      "decode_width" :  \
        ["16","decode_width determines the number of ports of the"
          "renaming table (both RAM and CAM) scheme (cpu.decodeWidth)"],
      "issue_width" : ["16","(cpu.issueWidth)"],
      "peak_issue_width" : \
        ["16","issue_width determines the number of ports of Issue"
          "window and other logic as in the complexity effective"
          "processors paper; issue_width==dispatch_width"
          "(cpu.issueWidth)"],
      "commit_width" : \
        ["16","commit_width determines the number of ports of register"
          "files (cpu.commitWidth)"],
      "fp_issue_width" : \
        ["2","Issue width of the Floating Poing Unit"],
      "prediction_width" : \
        ["1","number of branch instructions can be predicted"
          "simultaneously"],
      "pipelines_per_core" : \
        ["1,1","Current version of McPAT does not distinguish int and"
          "floating point pipelines. Theses parameters are reserved for"
          "future use. integer_pipeline and floating_pipelines, if the"
          "floating_pipelines is 0, then the pipeline is shared"],
      "pipeline_depth" : \
        ["31,31","pipeline depth of int and fp, if pipeline is shared,"
          "the second number is the average cycles of fp ops issue and"
          "exe unit"],
      "ALU_per_core" : \
        ["6","contains an adder, a shifter, and a logical unit"],
      "MUL_per_core" : ["1","For MUL and Div"],
      "FPU_per_core" : ["2","buffer between IF and ID stage"],
      "instruction_buffer_size" : \
        ["32","buffer between ID and sche/exe stage"],
      "decoded_stream_buffer_size" : ["16",""],
      "instruction_window_scheme" : \
        ["0","0 PHYREG based, 1 RSBASED. McPAT support 2 types of OoO"
          "cores, RS based and physical reg based"],
      "instruction_window_size" : ["32","(cpu.numIQEntries)"],
      "fp_instruction_window_size" : \
        ["32","The instruction issue Q as in Alpha 21264; The RS as in"
          "Intel P6 (cpu.numIQEntries)"],
      "ROB_size" : \
        ["32","Each in-flight instruction has an entry in ROB"
          "(cpu.numROBEntries)"],
      "archi_Regs_IRF_size" : \
        ["16","Number of Architectural Integer General Purpose"
          "Registers specified by the ISA: X86-64 has 16GPR"],
      "archi_Regs_FRF_size" : \
        ["32","Number of Architectural Registers specified by the ISA:"
          "MMX + XMM"],
      "phy_Regs_IRF_size" : \
        ["32","Number of Physical Integer Registers (cpu.numPhysIntRegs)"],
      "phy_Regs_FRF_size" : \
        ["32","Number of Physical FP Registers (cpu.numPhysFloatRegs)"],
      "rename_scheme" : \
        ["0","can be RAM based(0) or CAM based(1) rename scheme "
          "RAM-based scheme will have free list, status table; CAM-based "
          "scheme have the valid bit in the data field of the CAM"],
      "checkpoint_depth" : \
        ["0","RAM and CAM RAT contains checkpoints, checkpoint_depth=# "
          "of in_flight speculations; RAM-based RAT should not have more "
          "than 4 GCs (e.g., MIPS R10000).  McPAT assumes the exsistance "
          "of RRAT when the RAM-RAT having no GCs (e.g., Netburst) "
          "CAM-based RAT should have at least 1 GC and can have more than "
          "8 GCs."],
      "register_windows_size" : \
        ["0","How many windows in the windowed register file, sun "
          "processors; no register windowing is used when this number is "
          "0. In OoO cores, loads and stores can be issued whether "
          "inorder(Pentium Pro) or (OoO)out-of-order(Alpha), They will "
          "always try to execute out-of-order though."],
      "LSU_order" : ["inorder","Load/Store Unit (LSU) Ordering"],
      "store_buffer_size" : ["16","Store Queue Entries (cpu.SQEntries)"],
      "load_buffer_size" : \
        ["16","By default, in-order cores do not have load buffers "
          "(cpu.LQEntries)"],
      "memory_ports" : \
        ["2","max_allowed_in_flight_memo_instructions determines the # "
          "of ports of load and store buffer as well as the ports of "
          "Dcache which is connected to LSU. Dual-pumped Dcache can be "
          "used to save the extra read/write ports. Number of ports refer "
          "to sustain-able concurrent memory accesses"],
      "RAS_size" : ["2","Branch Predictor RAS Size"],
      "number_of_BPT" : ["2","Number of Branch Predictor Tables (BPT)"],
      "number_of_BTB" : ["2","Number of Branch Target Buffers (BTB)"]
    }
    self.stats = \
    {
      "total_instructions" : ["0","cpu.iq.iqInstsIssued"],
      "int_instructions" : \
        ["0","iq.FU_type_0::No_OpClass + iq.FU_type_0::IntAlu +"
          "iq.FU_type_0::IntMult + iq.FU_type_0::IntDiv +"
          "iq.FU_type_0::IprAccess"],
      "fp_instructions" : \
        ["0","cpu.iq.FU_type_0::FloatAdd + cpu.iq.FU_type_0::FloatCmp"
          "+ cpu.iq.FU_type_0::FloatCvt + cpu.iq.FU_type_0::FloatMult +"
          "cpu.iq.FU_type_0::FloatDiv + cpu.iq.FU_type_0::FloatSqrt"],
      "branch_instructions" : ["0","cpu.branchPred.condPredicted"],
      "branch_mispredictions" : ["0","cpu.branchPred.condIncorrect"],
      "load_instructions" : \
        ["0","cpu.iq.FU_type_0::MemRead + cpu.iq.FU_type_0::InstPrefetch"],
      "store_instructions" : ["0","cpu.iq.FU_type_0::MemWrite"],
      "committed_instructions" : ["0","cpu.commit.committedOps"],
      "committed_int_instructions" : ["0","cpu.commit.int_insts"],
      "committed_fp_instructions" : ["0","cpu.commit.fp_insts"],
      "pipeline_duty_cycle" : \
        ["1","<=1, runtime_ipc/peak_ipc; averaged for all cores if"
          "homogeneous"],
      "total_cycles" : ["1","cpu.numCycles"],
      "idle_cycles" : ["0","cpu.idleCycles"],
      "busy_cycles" : ["1","cpu.numCycles - cpu.idleCycles"],
      "ROB_reads" : ["0","cpu.rob.rob_reads"],
      "ROB_writes" : ["0","cpu.rob.rob_writes"],
      "rename_reads" : \
        ["0","lookup in renaming logic (cpu.rename.int_rename_lookups)"],
      "rename_writes" : \
        ["0","cpu.rename.RenamedOperands * "
          "cpu.rename.int_rename_lookups / cpu.rename.RenameLookups"],
      "fp_rename_reads" : ["0","cpu.rename.fp_rename_lookups"],
      "fp_rename_writes" : \
        ["0","cpu.rename.RenamedOperands * "
          "cpu.rename.fp_rename_lookups / cpu.rename.RenameLookups"],
      "inst_window_reads" : ["0","cpu.iq.int_inst_queue_reads"],
      "inst_window_writes" : ["0","cpu.iq.int_inst_queue_writes"],
      "inst_window_wakeup_accesses" : \
        ["0","cpu.iq.int_inst_queue_wakeup_accesses"],
      "fp_inst_window_reads" : ["0","cpu.iq.fp_inst_queue_reads"],
      "fp_inst_window_writes" : ["0","cpu.iq.fp_inst_queue_writes"],
      "fp_inst_window_wakeup_accesses" : \
        ["0","cpu.iq.fp_inst_queue_wakeup_accesses"],
      "int_regfile_reads" : ["0","cpu.int_regfile_reads"],
      "float_regfile_reads" : ["0","cpu.fp_regfile_reads"],
      "int_regfile_writes" : ["1","cpu.int_regfile_writes"],
      "float_regfile_writes" : ["1","cpu.fp_regfile_writes"],
      "function_calls" : ["0","cpu.commit.function_calls"],
      "context_switches" : ["0","cpu.workload.num_syscalls"],
      "ialu_accesses" : ["1","cpu.iq.int_alu_accesses"],
      "fpu_accesses" : ["1","cpu.iq.fp_alu_accesses"],
      "mul_accesses" : ["1","cpu.iq.fu_full::FloatMult"],
      "cdb_alu_accesses" : ["1","cpu.iq.int_alu_accesses"],
      "cdb_mul_accesses" : ["1","cpu.iq.fp_alu_accesses"],
      "cdb_fpu_accesses" : ["1","cpu.iq.fp_alu_accesses"],
      "IFU_duty_cycle" : ["0.25",""],
      "LSU_duty_cycle" : ["0.25",""],
      "MemManU_I_duty_cycle" : ["0.25",""],
      "MemManU_D_duty_cycle" : ["0.25",""],
      "ALU_duty_cycle" : ["1",""],
      "MUL_duty_cycle" : ["0.3",""],
      "FPU_duty_cycle" : ["0.3",""],
      "ALU_cdb_duty_cycle" : ["1",""],
      "MUL_cdb_duty_cycle" : ["0.3",""],
      "FPU_cdb_duty_cycle" : ["0.3",""]
    }

    self.predictor = None
    self.itlb = None
    self.icache = None
    self.dtlb = None
    self.dcache = None
    self.btb = None

    self.name = component_name
    self.id = component_id

    # Init the Directory Parameters and Stats:
    self.parameters["clock_rate"][0]= \
      str((1.0e-6/float(config_dict["clock"]))*1.0e12)
    self.parameters["vdd"][0]=str(float(sim_dict["voltage"]))
    self.parameters["power_gating_vcc"][0]="-1"
    self.parameters["opt_local"][0]="0"
    self.parameters["instruction_length"][0]="32"
    self.parameters["opcode_width"][0]="16"
    self.parameters["x86"][0]="1"
    self.parameters["micro_opcode_width"][0]="8"
    self.parameters["machine_type"][0]="0"
    self.parameters["number_hardware_threads"][0]= \
      str(int(config_dict["numThreads"]))
    self.parameters["fetch_width"][0]=str(int(config_dict["fetchWidth"]))
    self.parameters["number_instruction_fetch_ports"][0]="1"
    self.parameters["decode_width"][0]= \
      str(int(config_dict["decodeWidth"]))
    self.parameters["issue_width"][0]=str(int(config_dict["issueWidth"]))
    self.parameters["peak_issue_width"][0]= \
      str(int(config_dict["issueWidth"]))
    self.parameters["commit_width"][0]= \
      str(int(config_dict["commitWidth"]))
    self.parameters["fp_issue_width"][0]= \
      str(int(config_dict["fuPool.FUList2.count"]))
    self.parameters["prediction_width"][0]= \
      str(int(config_dict["branchPred.numThreads"]))
    self.parameters["pipelines_per_core"][0]="1,1"
    self.parameters["pipeline_depth"][0]="31,31"
    self.parameters["ALU_per_core"][0]= \
      str(int(config_dict["fuPool.FUList0.count"]))
    self.parameters["MUL_per_core"][0]= \
      str(int(config_dict["fuPool.FUList1.count"]))
    self.parameters["FPU_per_core"][0]= \
      str(int(config_dict["fuPool.FUList2.count"]) \
      +int(config_dict["fuPool.FUList3.count"]))
    self.parameters["instruction_buffer_size"][0]= \
      str(int(config_dict["fetchBufferSize"]))
    self.parameters["decoded_stream_buffer_size"][0]="16"
    self.parameters["instruction_window_scheme"][0]="0"
    self.parameters["instruction_window_size"][0]= \
      str(int(config_dict["numIQEntries"]))
    self.parameters["fp_instruction_window_size"][0]= \
      str(int(config_dict["numIQEntries"]))
    self.parameters["ROB_size"][0]=str(int(config_dict["numROBEntries"]))
    self.parameters["archi_Regs_IRF_size"][0]="16"
    self.parameters["archi_Regs_FRF_size"][0]="32"
    self.parameters["phy_Regs_IRF_size"][0]= \
      str(int(config_dict["numPhysIntRegs"]))
    self.parameters["phy_Regs_FRF_size"][0]= \
      str(int(config_dict["numPhysFloatRegs"]))
    self.parameters["rename_scheme"][0]="0"
    self.parameters["checkpoint_depth"][0]="0"
    self.parameters["register_windows_size"][0]="0"
    self.parameters["LSU_order"][0]="inorder"
    self.parameters["store_buffer_size"][0]= \
      str(int(config_dict["SQEntries"]))
    self.parameters["load_buffer_size"][0]= \
      str(int(config_dict["SQEntries"]))
    self.parameters["memory_ports"][0]="2"
    self.parameters["RAS_size"][0]= \
      str(int(config_dict["branchPred.RASSize"]))
    self.parameters["number_of_BPT"][0]= \
      str(int(config_dict["numThreads"]))
    self.parameters["number_of_BTB"][0]= \
      str(int(config_dict["numThreads"]))

    self.stats["total_instructions"][0]= \
      str(int(stat_dict["iq.iqInstsIssued"][1])+1)
    self.stats["int_instructions"][0]= \
      str(int(stat_dict["iq.FU_type_0::No_OpClass"][1]) \
      +int(stat_dict["iq.FU_type_0::IntAlu"][1]) \
      +int(stat_dict["iq.FU_type_0::IntMult"][1]) \
      +int(stat_dict["iq.FU_type_0::IntDiv"][1]) \
      +int(stat_dict["iq.FU_type_0::IprAccess"][1])+1)
    self.stats["fp_instructions"][0]= \
      str(int(stat_dict["iq.FU_type_0::FloatAdd"][1]) \
      +int(stat_dict["iq.FU_type_0::FloatCmp"][1]) \
      +int(stat_dict["iq.FU_type_0::FloatCvt"][1]) \
      +int(stat_dict["iq.FU_type_0::FloatMult"][1]) \
      +int(stat_dict["iq.FU_type_0::FloatDiv"][1]) \
      +int(stat_dict["iq.FU_type_0::FloatSqrt"][1]))
    self.stats["branch_instructions"][0]= \
      str(int(stat_dict["branchPred.condPredicted"][1]))
    self.stats["branch_mispredictions"][0]= \
      str(int(stat_dict["branchPred.condIncorrect"][1]))
    self.stats["load_instructions"][0]= \
      str(int(stat_dict["iq.FU_type_0::MemRead"][1]) \
      +int(stat_dict["iq.FU_type_0::InstPrefetch"][1]))
    self.stats["store_instructions"][0]= \
      str(int(stat_dict["iq.FU_type_0::MemWrite"][1]))
    self.stats["committed_instructions"][0]= \
      str(int(stat_dict["commit.committedOps"][1]))
    self.stats["committed_int_instructions"][0]= \
      str(int(stat_dict["commit.int_insts"][1]))
    self.stats["committed_fp_instructions"][0]= \
      str(int(stat_dict["commit.fp_insts"][1]))
    self.stats["pipeline_duty_cycle"][0]= \
      str(float(stat_dict["ipc_total"][1]))
    self.stats["total_cycles"][0]=str(int(stat_dict["numCycles"][1])+1)
    self.stats["idle_cycles"][0]=str(int(stat_dict["idleCycles"][1])+1)
    self.stats["busy_cycles"][0]= \
      str(int(stat_dict["numCycles"][1])-int(stat_dict["idleCycles"][1]))
    self.stats["ROB_reads"][0]=str(int(stat_dict["rob.rob_reads"][1]))
    self.stats["ROB_writes"][0]=str(int(stat_dict["rob.rob_writes"][1]))
    self.stats["rename_reads"][0]= \
      str(int(stat_dict["rename.int_rename_lookups"][1]))
    self.stats["rename_writes"][0]= \
      str(int(stat_dict["rename.RenamedOperands"][1]) \
      *int(stat_dict["rename.int_rename_lookups"][1]) \
      /(1+int(stat_dict["rename.RenameLookups"][1])))
    self.stats["fp_rename_reads"][0]= \
      str(int(stat_dict["rename.fp_rename_lookups"][1]))
    self.stats["fp_rename_writes"][0]= \
      str(int(stat_dict["rename.RenamedOperands"][1]) \
      *int(stat_dict["rename.fp_rename_lookups"][1]) \
      /(1+int(stat_dict["rename.RenameLookups"][1])))
    self.stats["inst_window_reads"][0]= \
      str(int(stat_dict["iq.int_inst_queue_reads"][1]))
    self.stats["inst_window_writes"][0]= \
      str(int(stat_dict["iq.int_inst_queue_writes"][1]))
    self.stats["inst_window_wakeup_accesses"][0]= \
      str(int(stat_dict["iq.int_inst_queue_wakeup_accesses"][1]))
    self.stats["fp_inst_window_reads"][0]= \
      str(int(stat_dict["iq.fp_inst_queue_reads"][1]))
    self.stats["fp_inst_window_writes"][0]= \
      str(int(stat_dict["iq.fp_inst_queue_writes"][1]))
    self.stats["fp_inst_window_wakeup_accesses"][0]= \
      str(int(stat_dict["iq.fp_inst_queue_wakeup_accesses"][1]))
    self.stats["int_regfile_reads"][0]= \
      str(int(stat_dict["int_regfile_reads"][1]))
    self.stats["float_regfile_reads"][0]= \
      str(int(stat_dict["fp_regfile_reads"][1]))
    self.stats["int_regfile_writes"][0]= \
      str(int(stat_dict["int_regfile_writes"][1]))
    self.stats["float_regfile_writes"][0]= \
      str(int(stat_dict["fp_regfile_writes"][1]))
    self.stats["function_calls"][0]= \
      str(int(stat_dict["commit.function_calls"][1]))
    self.stats["context_switches"][0]= \
      str(int(stat_dict["workload.numSyscalls"][1]) \
      if "workload.numSyscalls" in stat_dict.keys() else 0)
    self.stats["ialu_accesses"][0]= \
      str(int(stat_dict["iq.int_alu_accesses"][1]))
    self.stats["fpu_accesses"][0]= \
      str(int(stat_dict["iq.fp_alu_accesses"][1]))
    self.stats["mul_accesses"][0]= \
      str(int(stat_dict["iq.fu_full::FloatMult"][1]))
    self.stats["cdb_alu_accesses"][0]= \
      str(int(stat_dict["iq.int_alu_accesses"][1]))
    self.stats["cdb_mul_accesses"][0]= \
      str(int(stat_dict["iq.fp_alu_accesses"][1]))
    self.stats["cdb_fpu_accesses"][0]= \
      str(int(stat_dict["iq.fp_alu_accesses"][1]))
    self.stats["IFU_duty_cycle"][0]="0.25"
    self.stats["LSU_duty_cycle"][0]="0.25"
    self.stats["MemManU_I_duty_cycle"][0]="0.25"
    self.stats["MemManU_D_duty_cycle"][0]="0.25"
    self.stats["ALU_duty_cycle"][0]="1"
    self.stats["MUL_duty_cycle"][0]="0.3"
    self.stats["FPU_duty_cycle"][0]="0.3"
    self.stats["ALU_cdb_duty_cycle"][0]="1"
    self.stats["MUL_cdb_duty_cycle"][0]="0.3"
    self.stats["FPU_cdb_duty_cycle"][0]="0.3"

    self.predictor = Predictor \
    ( \
      self.id+".predictor", \
      "PBT", \
      prune_dict("branchPred.",stat_dict), \
      prune_dict("branchPred.",config_dict), \
      sim_dict \
    )
    self.itlb = TLB \
    ( \
      self.id+".itlb", \
      "itlb", \
      prune_dict("itb_walker_cache.",stat_dict), \
      prune_dict("itb.",config_dict), \
      sim_dict \
    )
    self.icache = ICache \
    ( \
      self.id+".icache", \
      "icache", \
      prune_dict("icache.",stat_dict), \
      prune_dict("icache.",config_dict), \
      sim_dict \
    )
    self.dtlb = TLB \
    ( \
      self.id+".dtlb", \
      "dtlb", \
      prune_dict("dtb_walker_cache.",stat_dict), \
      prune_dict("dtb.",config_dict), \
      sim_dict \
    )
    self.dcache = DCache \
    ( \
      self.id+".dcache", \
      "dcache", \
      prune_dict("dcache.",stat_dict), \
      prune_dict("dcache.",config_dict), \
      sim_dict \
    )
    self.btb = BTB \
    ( \
      self.id+".BTB", \
      "BTB", \
      prune_dict("branchPred.",stat_dict), \
      prune_dict("branchPred.",config_dict), \
      sim_dict \
    )

  def xml(self):
    """ Build an XML Tree from the parameters, stats, and
    subcomponents """
    top = ElementTree.Element('component', id=self.id, name=self.name)
    for key in sorted(self.parameters):
      top.append(ElementTree.Comment( \
        ", ".join(['param', key, self.parameters[key][1]])))
      top.append(ElementTree.Element( \
        'param', name=key, value=self.parameters[key][0]))
    for key in sorted(self.stats):
      top.append(ElementTree.Comment( \
        ", ".join(['stat', key, self.stats[key][1]])))
      top.append(ElementTree.Element( \
        'stat', name=key, value=self.stats[key][0]))
    top.append(self.predictor.xml())
    top.append(self.itlb.xml())
    top.append(self.icache.xml())
    top.append(self.dtlb.xml())
    top.append(self.dcache.xml())
    top.append(self.btb.xml())
    return top

