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

from m5.SimObject import *
from m5.objects.ClockedObject import ClockedObject

from m5.params import *
from m5.proxy import *

class PowerPredictor(ClockedObject):
    type = 'PowerPredictor'
    cxx_class = 'PPredUnit'
    cxx_header = "cpu/power/ppred_unit.hh"
    abstract = True

    min_current = Param.Unsigned(0, "Minimum Current Supply " \
        "(Amps) of the PSU")
    max_current = Param.Unsigned(40, "Maximum Current Supply " \
        "(Amps) of the PSU")

    sys_clk_domain = Param.SrcClockDomain(Parent.clk_domain,
                         "Clk domain in which the handler is instantiated")

    period = Param.Unsigned(100, "Number of sim-cycles")
    cycle_period = Param.Unsigned(1, "Clock Cycle Resolution")
    delta = Param.Float(0.75, "Rate at which to train")
    emergency = Param.Float(0.95, "% Voltage considered a supply emergency")
    emergency_duration = Param.Unsigned(250, "Number of cycles to do a "
        "DECOR Rollback")
    clk = Param.Float(3.5e9, "Default Clock Freq")
    emergency_throttle = Param.Bool(True, "Throttle on emergency")
    voltage_set = Param.Float(True, "Voltage Set")
    cpu_id = Param.Unsigned(0, "Cpu ID")
    signature_length = Param.Unsigned(256,"Length of History Snapshot " \
        "(Figure 2)")
    action_length = Param.Unsigned(2,"Number of Throttle Actions")
    lead_time = Param.Unsigned(40,"Lead time for predictions")

class Test(PowerPredictor):
    type = 'Test'
    cxx_class = 'Test'
    cxx_header = 'cpu/power/test.hh'
    threshold = Param.Float(0.975, "For evaluation purposes")


## Harvard Power Predictor
#
# This predictor is a reconstruction of the Power Predictor from:
#
# https://ieeexplore.ieee.org/document/4798233
class HarvardPowerPredictor(PowerPredictor):
    type = "HarvardPowerPredictor"
    cxx_class = "Harvard"
    cxx_header = "cpu/power/harvard.hh"
    table_size = Param.Unsigned(128, "Size of UArch Event Table")
    bloom_filter_size = Param.Unsigned(2048, "Size of Bloom Filter")
    hysteresis = Param.Float(0.01, "The Percentage of Supply Voltage " \
        "to stop emergency throttle")
    duration = Param.Unsigned(50, "The number of cycles to throttle for")
    throttle_on_restore = Param.Bool(False, "Throttle on the Restore")

class PPredStat(ClockedObject):
    type = "PPredStat"
    cxx_class = "PPredStat"
    cxx_header = "cpu/power/ppred_stat.hh"

    stat_clk_domain = Param.SrcClockDomain(Parent.ppred_stat_clk, "Clk domain for Stat Dump")
    cycle_period = Param.Unsigned(1, "Clock Cycle Resolution")
    frequency = Param.Float(3.5e9, "Default clock freq")
    ncores = Param.Unsigned(1, "Number of cores to sim")
    mcpat_output_path = Param.String("", "xml path for mcpat")
    ind = Param.Float(0, "pdn inductance")
    cap = Param.Float(0, "pdn capacitance")
    res = Param.Float(0, "pdn resistance")
    vdc = Param.Float(0, "steady state pdn voltage")
    debug_print_delay = Param.Unsigned(0, "debug print delay")
    power_start_delay = Param.Int(1, "after how many cycles to begin power simulation")
    powerpred = Param.PowerPredictor(Parent.cpu[0].powerPred , "the power predictor")

