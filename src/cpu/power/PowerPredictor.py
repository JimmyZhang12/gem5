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
    signature_length = Param.Unsigned(1,"Length of History Snapshot " \
        "(Figure 2)")

class Test(PowerPredictor):
    type = 'Test'
    cxx_class = 'Test'
    cxx_header = 'cpu/power/test.hh'

class SimplePowerPredictor(PowerPredictor):
    type = 'SimplePowerPredictor'
    cxx_class = 'Simple'
    cxx_header = 'cpu/power/simple.hh'

    num_entries = Param.Unsigned(1024, "Entries in predictor table lookup")
    num_correlation_bits = Param.Unsigned(10, "Number of bits to form " \
        "a correlation")
    pc_start = Param.Unsigned(10, "how many bits to shift the pc by")
    quantization_levels = Param.Unsigned(256, "Default number of levels " \
        "to quantize the current supply prediction by")
    error_array_size = Param.Unsigned(100, "size of the rolling error array")
    confidence_level = Param.Float(0.10, "Prediction error before enabiling " \
        "the auxiliary circuit.")
    limit = Param.Float(10, "Limit on the istep")

class SimpleHistoryPowerPredictor(PowerPredictor):
    type = 'SimpleHistoryPowerPredictor'
    cxx_class = 'SimpleHistory'
    cxx_header = 'cpu/power/simple_history.hh'

    num_entries = Param.Unsigned(1024, "Entries in predictor table lookup")
    nbits_pc = Param.Unsigned(4, "Number of bits to form " \
        "a correlation")
    pc_start = Param.Unsigned(10, "how many bits to shift the pc by")
    history_size = Param.Unsigned(1, "How many PC values to use in the "
        "history")
    quantization_levels = Param.Unsigned(256, "Default number of levels " \
        "to quantize the current supply prediction by")
    error_array_size = Param.Unsigned(100, "size of the rolling error array")
    confidence_level = Param.Float(0.10, "Prediction error before enabiling " \
        "the auxiliary circuit.")
    limit = Param.Float(10, "Limit on the istep")

class PerceptronPredictor(PowerPredictor):
    type = "PerceptronPredictor"
    cxx_class = "PerceptronPredictor"
    cxx_header = "cpu/power/perceptron_predictor.hh"

    signature_length = Param.Unsigned(256,"Length of History Snapshot")
    training_output = Param.String("td.csv","CSV of output training data")
    hysteresis = Param.Float(0.01, "The Percentage of Supply Voltage " \
        "to stop emergency throttle")
    duration = Param.Unsigned(50, "The number of cycles to throttle for")

class HarvardPowerPredictor(PowerPredictor):
    type = "HarvardPowerPredictor"
    cxx_class = "Harvard"
    cxx_header = "cpu/power/harvard.hh"

    table_size = Param.Unsigned(128, "Size of UArch Event Table")
    bloom_filter_size = Param.Unsigned(2048, "Size of Bloom Filter")
    hysteresis = Param.Float(0.01, "The Percentage of Supply Voltage " \
        "to stop emergency throttle")
    duration = Param.Unsigned(50, "The number of cycles to throttle for")

class uArchEventPredictor(PowerPredictor):
    type = "uArchEventPredictor"
    cxx_class = "uArchEventPredictor"
    cxx_header = "cpu/power/uarch_event.hh"
    """ Stores the address of the last microarch event and the
    instruction responsible for it """

    table_size = Param.Unsigned(128, "Size of UArch Event Table")
    hysteresis = Param.Float(0.01, "The Percentage of Supply Voltage " \
        "to stop emergency throttle")
    duration = Param.Unsigned(50, "The number of cycles to throttle for")

class IdealSensor(PowerPredictor):
    type = "IdealSensor"
    cxx_class = "Sensor"
    cxx_header = "cpu/power/sensor.hh"

    threshold = Param.Float(0.975, "The Percentage of Supply Voltage " \
        "to trigger an emergency throttle")
    hysteresis = Param.Float(0.01, "The Percentage of Supply Voltage " \
        "to stop emergency throttle")
    duration = Param.Unsigned(20, "The number of cycles to throttle for")
    latency = Param.Unsigned(0, "Latency before the throttling action " \
        "is taken")

class DecorOnly(PowerPredictor):
    type = "DecorOnly"
    cxx_class = "DecorOnly"
    cxx_header = "cpu/power/decor_only.hh"
