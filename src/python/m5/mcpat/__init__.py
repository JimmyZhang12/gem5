# Copyright (c) 2020 The University of Illinois
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

import os
import sys
import re
import pickle
import subprocess
import math
from collections import defaultdict
from m5.SimObject import SimObject
from m5.util import fatal
from m5.params import *

from m5.mcpat.autogen import *

from util import *
from node import Node
from device import Device
from epoch import Epoch

rolling_avg_len=4
rolling_avg=False

power_gating = False
scale_factor = 1.0

iter = 0
mcpat_trees = []
stat_trace=[]

rolling_avg_p = [0]*rolling_avg_len

def set_flags(pg=False, sf=1.0):
  global power_gating
  global scale_factor
  power_gating = pg
  scale_factor = sf

def m5_to_mcpat(stat_list, read_from_file, voltage, freq, temperature, device_type):
  from m5 import options

  global iter
  global mcpat_trees
  global stat_trace
  global rolling_avg_p
  global rolling_avg
  global rolling_avg_len
  global scale_factor
  global power_gating

  idx = iter % rolling_avg_len

  iter += 1
  m5_stats_file = os.path.join(options.outdir, options.stats_file)
  m5_config_file = os.path.join(options.outdir, options.dump_config)
  mcpat_path = options.mcpat_path
  mcpat_output_path = os.path.join(options.mcpat_out, options.mcpat_testname)
  testname = options.mcpat_testname

  if not os.path.isdir(mcpat_output_path):
    os.mkdir(mcpat_output_path)

  if(options.mcpat_save_space):
    i_f = os.path.join(mcpat_output_path,"mp.xml")
    o_f = os.path.join(mcpat_output_path,"mp.out")
    e_f = os.path.join(mcpat_output_path,"mp.err")
  else:
    i_f = os.path.join(mcpat_output_path,"mp_"+str(iter)+".xml")
    o_f = os.path.join(mcpat_output_path,"mp_"+str(iter)+".out")
    e_f = os.path.join(mcpat_output_path,"mp_"+str(iter)+".err")

  generate_xml(m5_stats_file, m5_config_file, i_f, stat_list, \
              read_from_file, voltage=voltage, \
              frequency=freq, temperature=temperature, device_type=device_type)
  run_mcpat(i_f, "5", "1", o_f, e_f)

  mcpat_trees = [parse_output(o_f)]
  if(rolling_avg):
    data = get_data("Processor", mcpat_trees)
    rolling_avg_p[idx] = calc_total_power(data, power_gating, scale_factor)



def dump():
  dump_stats(mcpat_trees)

def get_last_p(voltage, power_gating=False, scale_factor=1.0):
  global rolling_avg
  global rolling_avg_p
  global rolling_avg_len
  data = get_data("Processor", mcpat_trees)
  if(rolling_avg):
    return sum(rolling_avg_p)/rolling_avg_len
  return calc_total_power(data, power_gating, scale_factor)

def get_last_r(voltage, power_gating=False, scale_factor=1.0):
  global rolling_avg
  global rolling_avg_p
  global rolling_avg_len
  data = get_data("Processor", mcpat_trees)
  if(rolling_avg):
    power = sum(rolling_avg_p)/rolling_avg_len
    return calc_req(power, voltage)
  power = calc_total_power(data, power_gating, scale_factor)
  return calc_req(power, voltage)

def get_last_i(voltage, power_gating=False, scale_factor=1.0):
  global rolling_avg
  global rolling_avg_p
  global rolling_avg_len
  data = get_data("Processor", mcpat_trees)
  if(rolling_avg):
    power = sum(rolling_avg_p)/rolling_avg_len
    return power/voltage
  power = calc_total_power(data, power_gating, scale_factor)
  return power/voltage

def get_runtime_dynamic(path):
  #print(path)
  #print(mcpat_trees)
  data = get_data(path, mcpat_trees)
  return float(data["Runtime Dynamic"])

