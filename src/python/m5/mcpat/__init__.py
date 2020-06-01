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

from util import *
from node import Node
from device import Device
from epoch import Epoch

iter = 0
mcpat_trees = []
stat_trace=[]

def m5_to_mcpat(voltage, temperature):
  from m5 import options

  global iter
  global mcpat_trees
  global stat_trace

  iter += 1
  m5_stats_file = os.path.join(options.outdir, options.stats_file)
  m5_config_file = os.path.join(options.outdir, options.dump_config)
  mcpat_path = options.mcpat_path
  mcpat_template = options.mcpat_template
  mcpat_output_path = os.path.join(options.mcpat_out, options.mcpat_testname)
  testname = options.mcpat_testname

  epoch = parse_stats(m5_stats_file)[-1]
  config = parse_config(m5_config_file)

  if not os.path.isdir(mcpat_output_path):
    os.mkdir(mcpat_output_path)

  t_f = options.mcpat_template
  i_f = os.path.join(mcpat_output_path,"mp_"+str(iter)+".xml")
  o_f = os.path.join(mcpat_output_path,"mp_"+str(iter)+".out")
  e_f = os.path.join(mcpat_output_path,"mp_"+str(iter)+".err")
  with open(t_f, "r") as tf, open(i_f, "w") as inf:
    in_xml = tf.readlines()
    out_xml = []
    s = {}
    for line in in_xml:
      out_xml.append(replace(line, epoch, config, voltage, temperature, s))
    # To trace the stats reported
    stat_trace.append(s)
    inf.writelines(out_xml)
  run_mcpat(i_f, "5", "1", o_f, e_f)
  mcpat_trees.append(parse_output(o_f))
  #print(mcpat_trees[-1].find("Processor").data)

def dump():
  dump_stats(mcpat_trees, stat_trace)

def get_last_p(voltage):
  global mcpat_trees
  data_line = []
  for key, value in mcpat_trees[-1].find("Processor").data.items():
    data_line.append(value.split(" ")[0])
  # Calculate Total Power:
  return calc_total_power(data_line)

def get_last_r(voltage):
  global mcpat_trees
  data_line = []
  for key, value in mcpat_trees[-1].find("Processor").data.items():
    data_line.append(value.split(" ")[0])
  # Calculate Total Power:
  power = calc_total_power(data_line)
  return calc_req(power, voltage)

def get_last_i(voltage):
  global mcpat_trees
  data_line = []
  for key, value in mcpat_trees[-1].find("Processor").data.items():
    data_line.append(value.split(" ")[0])
  # Calculate Total Power:
  power = calc_total_power(data_line)
  return power/voltage;
