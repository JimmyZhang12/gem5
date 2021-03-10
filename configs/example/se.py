# Copyright (c) 2012-2013 ARM Limited
# All rights reserved.
#
# The license below extends only to copyright in the software and shall
# not be construed as granting a license to any other intellectual
# property including but not limited to intellectual property relating
# to a hardware implementation of the functionality of the software
# licensed hereunder.  You may use the software subject to the license
# terms below provided that you ensure that this notice is replicated
# unmodified and in its entirety in all distributions of the software,
# modified or unmodified, in source code or in binary form.
#
# Copyright (c) 2006-2008 The Regents of The University of Michigan
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
# Authors: Steve Reinhardt

# Simple test script
#
# "m5 test.py"

from __future__ import print_function
from __future__ import absolute_import

import optparse
import sys
import os
import math

import m5
from m5.defines import buildEnv
from m5.objects import *
from m5.util import addToPath, fatal, warn

addToPath('../')

from ruby import Ruby

from common import Options
from common import Simulation
from common import CacheConfig
from common import CpuConfig
from common import ObjectList
from common import MemConfig
from common.FileSystemConfig import config_filesystem
from common.Caches import *
from common.cpu2000 import *

def get_processes(options):
    """Interprets provided options and returns a list of processes"""

    multiprocesses = []
    inputs = []
    outputs = []
    errouts = []
    pargs = []

    workloads = options.cmd.split(';')
    if options.input != "":
        inputs = options.input.split(';')
    if options.output != "":
        outputs = options.output.split(';')
    if options.errout != "":
        errouts = options.errout.split(';')
    if options.options != "":
        pargs = options.options.split(';')

    idx = 0
    for wrkld in workloads:
        process = Process(pid = 100 + idx)
        process.executable = wrkld
        process.cwd = os.getcwd()

        if options.env:
            with open(options.env, 'r') as f:
                process.env = [line.rstrip() for line in f]

        if len(pargs) > idx:
            process.cmd = [wrkld] + pargs[idx].split()
        else:
            process.cmd = [wrkld]

        if len(inputs) > idx:
            process.input = inputs[idx]
        if len(outputs) > idx:
            process.output = outputs[idx]
        if len(errouts) > idx:
            process.errout = errouts[idx]

        multiprocesses.append(process)
        idx += 1

    if options.smt:
        assert(options.cpu_type == "DerivO3CPU")
        return multiprocesses, idx
    else:
        return multiprocesses, 1


parser = optparse.OptionParser()
Options.addCommonOptions(parser)
Options.addSEOptions(parser)
Options.addO3CPUOptions(parser)

if '--ruby' in sys.argv:
    Ruby.define_options(parser)

(options, args) = parser.parse_args()

if args:
    print("Error: script doesn't take any positional arguments")
    sys.exit(1)

multiprocesses = []
numThreads = 1

if options.bench:
    apps = options.bench.split("-")
    if len(apps) != options.num_cpus:
        print("number of benchmarks not equal to set num_cpus!")
        sys.exit(1)

    for app in apps:
        try:
            if buildEnv['TARGET_ISA'] == 'alpha':
                exec("workload = %s('alpha', 'tru64', '%s')" % (
                        app, options.spec_input))
            elif buildEnv['TARGET_ISA'] == 'arm':
                exec("workload = %s('arm_%s', 'linux', '%s')" % (
                        app, options.arm_iset, options.spec_input))
            else:
                exec("workload = %s(buildEnv['TARGET_ISA', 'linux', '%s')" % (
                        app, options.spec_input))
            multiprocesses.append(workload.makeProcess())
        except:
            print("Unable to find workload for %s: %s" %
                  (buildEnv['TARGET_ISA'], app),
                  file=sys.stderr)
            sys.exit(1)
elif options.cmd:
    multiprocesses, numThreads = get_processes(options)
else:
    print("No workload specified. Exiting!\n", file=sys.stderr)
    sys.exit(1)


(CPUClass, test_mem_mode, FutureClass) = Simulation.setCPUClass(options)
CPUClass.numThreads = numThreads

# Check -- do not allow SMT with multiple CPUs
if options.smt and options.num_cpus > 1:
    fatal("You cannot use SMT with multiple CPUs!")

np = options.num_cpus
system = System(cpu = [CPUClass(cpu_id=i) for i in range(np)],
                mem_mode = test_mem_mode,
                mem_ranges = [AddrRange(options.mem_size)],
                cache_line_size = options.cacheline_size)

if numThreads > 1:
    system.multi_thread = True

# Create a top-level voltage domain
system.voltage_domain = VoltageDomain(voltage = options.sys_voltage)

# Create a source clock for the system and set the clock period
system.clk_domain = SrcClockDomain(clock =  options.sys_clock,
                                   voltage_domain = system.voltage_domain)

# Create a CPU voltage domain
system.cpu_voltage_domain = VoltageDomain()

# Create a separate clock domain for the CPUs
system.cpu_clk_domain = [SrcClockDomain(clock = options.sys_clock,
                                       voltage_domain =
                                       system.cpu_voltage_domain)
                                       for i in range(np)]

# If elastic tracing is enabled, then configure the cpu and attach the elastic
# trace probe
if options.elastic_trace_en:
    CpuConfig.config_etrace(CPUClass, system.cpu, options)

# All cpus belong to a common cpu_clk_domain, therefore running at a common
# frequency.
for cpu, clk in zip(system.cpu, system.cpu_clk_domain):
    cpu.clk_domain = clk

if ObjectList.is_kvm_cpu(CPUClass) or ObjectList.is_kvm_cpu(FutureClass):
    if buildEnv['TARGET_ISA'] == 'x86':
        system.kvm_vm = KvmVM()
        for process in multiprocesses:
            process.useArchPT = True
            process.kvmInSE = True
    else:
        fatal("KvmCPU can only be used in SE mode with x86")

# Sanity check
if options.simpoint_profile:
    if not ObjectList.is_noncaching_cpu(CPUClass):
        fatal("SimPoint/BPProbe should be done with an atomic cpu")
    if np > 1:
        fatal("SimPoint generation not supported with more than one CPUs")

# Clock for dumping stats
system.ppred_stat_clk = SrcClockDomain(clock = options.sys_clock, \
                                   voltage_domain = system.voltage_domain)

system.ppred_stat = PPredStat( \
    cycle_period = options.power_pred_cpu_cycles, \
    frequency = options.power_pred_cpu_freq, \
    ncores = np, \
    mcpat_output_path = options.mcpat_output_path, \
    vdc = options.power_pred_voltage, \
    ind = options.pdn_ind, \
    cap = options.pdn_cap,\
    res = options.pdn_res, \
    debug_print_delay = options.debug_print_delay, \
    power_start_delay = options.power_start_delay, \
    run_verilog = bool(options.run_verilog_power_sim), \
)

system.ppred_stat.clk_domain = system.ppred_stat_clk

for i in range(np):
    if options.smt:
        system.cpu[i].workload = multiprocesses
    elif len(multiprocesses) == 1:
        system.cpu[i].workload = multiprocesses[0]
    else:
        system.cpu[i].workload = multiprocesses[i]

    if options.simpoint_profile:
        system.cpu[i].addSimPointProbe(options.simpoint_interval)

    if options.checker:
        system.cpu[i].addCheckerCpu()

    if options.bp_type:
        bpClass = ObjectList.bp_list.get(options.bp_type)
        system.cpu[i].branchPred = bpClass()

    if options.power_pred_type:
        powerPredClass = ObjectList.power_pred_list.get(options.power_pred_type)
        if options.power_pred_type == "Test":
            ncb = math.floor(math.log(options.power_pred_table_size, 2))
            system.cpu[i].powerPred = \
                powerPredClass(
                    # Base
                    period=options.power_profile_interval,
                    cpu_id=i,
                    cycle_period=options.power_pred_cpu_cycles,
                    clk = options.power_pred_cpu_freq,
                    voltage_set=options.power_pred_voltage,
                    threshold=options.power_pred_voltage_threshold,
                    emergency=options.power_pred_voltage_emergency)

        elif options.power_pred_type == "IdealSensor":
            system.cpu[i].powerPred = \
                powerPredClass(
                    # Base
                    period=options.power_profile_interval,
                    cycle_period=options.power_pred_cpu_cycles,
                    cpu_id=i,
                    clk = options.power_pred_cpu_freq,
                    voltage_set=options.power_pred_voltage,
                    emergency=options.power_pred_voltage_emergency,
                    emergency_duration=100,
                    action_length=options.power_pred_actions,
                    # Specific
                    throttle_on_restore=False,
                    threshold=options.power_pred_voltage_threshold,
                    hysteresis=0.005,
                    duration=8)
        elif options.power_pred_type == "HarvardPowerPredictor":
            system.cpu[i].powerPred = \
                powerPredClass(
                    # Base
                    period=options.power_profile_interval,
                    cpu_id=i,
                    cycle_period=options.power_pred_cpu_cycles,
                    clk = options.power_pred_cpu_freq,
                    voltage_set=options.power_pred_voltage,
                    emergency=options.power_pred_voltage_emergency,
                    emergency_duration=100,
                    signature_length=64,
                    # Specific
                    throttle_on_restore=False,
                    table_size=1000,
                    bloom_filter_size=10000,
                    hysteresis=0.005,
                    throttle_duration=16,
                    lead_time_max=70,
                    lead_time_min=0,
                    events_to_drop=3,
                    hamming_distance=8)
        elif options.power_pred_type == "HarvardPowerPredictor_dev":
            system.cpu[i].powerPred = \
                powerPredClass(
                    # Base
                    period=options.power_profile_interval,
                    cpu_id=i,
                    cycle_period=options.power_pred_cpu_cycles,
                    clk = options.power_pred_cpu_freq,
                    voltage_set=options.power_pred_voltage,
                    emergency=options.power_pred_voltage_emergency,
                    emergency_duration=100,
                    signature_length=64,
                    # Specific
                    throttle_on_restore=False,
                    table_size=1000,
                    bloom_filter_size=10000,
                    hysteresis=0.005,
                    duration=8,
                    lead_time=options.power_pred_lead_time)
        elif options.power_pred_type == "DepAnalysis":
            system.cpu[i].powerPred = \
                powerPredClass(
                    # Base
                    period=options.power_profile_interval,
                    cpu_id=i,
                    cycle_period=options.power_pred_cpu_cycles,
                    clk = options.power_pred_cpu_freq,
                    voltage_set=options.power_pred_voltage,
                    emergency=options.power_pred_voltage_emergency,
                    emergency_duration=100,
                    # Specific
                    throttle_on_restore=False,
                    threshold = 4,
                    duration=8)

        system.cpu[i].powerPred.clk_domain = \
            system.cpu_clk_domain[i]
        # Give core a reference to the global stat dump
        system.cpu[i].ppred_stat = system.ppred_stat

    # Configure CPU Parameters:
    system.cpu[i].fetchWidth = options.cpu_superscalar_width
    system.cpu[i].decodeWidth = options.cpu_superscalar_width
    system.cpu[i].renameWidth = options.cpu_superscalar_width
    system.cpu[i].dispatchWidth = options.cpu_superscalar_width
    system.cpu[i].issueWidth = options.cpu_superscalar_width
    system.cpu[i].wbWidth = options.cpu_superscalar_width
    system.cpu[i].commitWidth = options.cpu_superscalar_width
    system.cpu[i].squashWidth = options.cpu_superscalar_width
    # Fetch
    system.cpu[i].fetchBufferSize = options.cpu_fetch_buffer_size
    system.cpu[i].fetchQueueSize = options.cpu_fetch_q_size
    # LSQ
    system.cpu[i].LQEntries = options.cpu_lq_size
    system.cpu[i].SQEntries = options.cpu_sq_size
    # ROB
    system.cpu[i].numRobs = options.cpu_num_robs
    system.cpu[i].numROBEntries = options.cpu_num_rob_entries
    # Regfile
    system.cpu[i].numPhysIntRegs = options.cpu_phys_int_regs
    system.cpu[i].numPhysFloatRegs = options.cpu_phys_fp_regs
    system.cpu[i].numPhysVecRegs = options.cpu_phys_vec_regs
    system.cpu[i].numPhysVecPredRegs = options.cpu_phys_vec_pred_regs
    # Instruction Queue
    system.cpu[i].numIQEntries = options.cpu_num_iq_entries
    # FU POOL
    system.cpu[i].fuPool.FUList[0].count = options.cpu_intALUcount
    system.cpu[i].fuPool.FUList[1].count = options.cpu_intMULDIVcount
    system.cpu[i].fuPool.FUList[2].count = options.cpu_fpALUcount
    system.cpu[i].fuPool.FUList[3].count = options.cpu_fpMULDIVcount
    system.cpu[i].fuPool.FUList[5].count = options.cpu_simdcount

    if options.indirect_bp_type:
        indirectBPClass = \
            ObjectList.indirect_bp_list.get(options.indirect_bp_type)
        system.cpu[i].branchPred.indirectBranchPred = indirectBPClass()

    system.cpu[i].createThreads()

system.ppred_stat.powerpred = system.cpu[0].powerPred


if options.ruby:
    Ruby.create_system(options, False, system)
    assert(options.num_cpus == len(system.ruby._cpu_ports))

    system.ruby.clk_domain = SrcClockDomain(clock = options.ruby_clock,
                                        voltage_domain = system.voltage_domain)
    for i in range(np):
        ruby_port = system.ruby._cpu_ports[i]

        # Create the interrupt controller and connect its ports to Ruby
        # Note that the interrupt controller is always present but only
        # in x86 does it have message ports that need to be connected
        system.cpu[i].createInterruptController()

        # Connect the cpu's cache ports to Ruby
        system.cpu[i].icache_port = ruby_port.slave
        system.cpu[i].dcache_port = ruby_port.slave
        if buildEnv['TARGET_ISA'] == 'x86':
            system.cpu[i].interrupts[0].pio = ruby_port.master
            system.cpu[i].interrupts[0].int_master = ruby_port.slave
            system.cpu[i].interrupts[0].int_slave = ruby_port.master
            system.cpu[i].itb.walker.port = ruby_port.slave
            system.cpu[i].dtb.walker.port = ruby_port.slave
else:
    MemClass = Simulation.setMemClass(options)
    system.membus = SystemXBar()
    system.system_port = system.membus.slave
    CacheConfig.config_cache(options, system)
    MemConfig.config_mem(options, system)
    config_filesystem(system, options)

#m5.stats.periodicStatDump(options.power_profile_initial_stats_interval)
"""1 000 000 000 000"""
root = Root(full_system = False, system = system)
Simulation.run(options, root, system, FutureClass)
