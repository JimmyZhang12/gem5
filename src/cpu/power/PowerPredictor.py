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
    max_current = Param.Unsigned(20, "Maximum Current Supply " \
        "(Amps) of the PSU")

    period = Param.Unsigned(250, "Number of cpu-cycles per epoch")

class TestPowerPredictor(PowerPredictor):
    type = 'TestPowerPredictor'
    cxx_class = 'Test'
    cxx_header = 'cpu/power/test.hh'

    num_entries = Param.Unsigned(1024, "Entries in predictor table lookup")
    num_correlation_bits = Param.Unsigned(10, "Number of bits to form " \
        "a correlation")
    pc_start = Param.Unsigned(10, "how many bits to shift the pc by")


