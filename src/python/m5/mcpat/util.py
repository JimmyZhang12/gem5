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
from file_read_backwards import FileReadBackwards
from collections import defaultdict
from m5.SimObject import SimObject
from m5.util import fatal
from m5.params import *

from node import Node
from device import Device
from epoch import Epoch

def parse_output(output_file):
  def strip_header(lines):
    start = False
    ret = []
    for line in lines:
      if("Processor:" in line):
        start = True
      if start:
        ret.append(line)
    return ret

  def strip_space(lines):
    ret = []
    last_line_star = False
    start_core = False
    for line in lines:
      if "Core:" in line:
        start_core = True
        if last_line_star:
          #Fix spacing after ******
          ret.append("  "+line)
          last_line_star = False
      elif "*****" in line:
        last_line_star = True
      elif "Device Type=" in line or "     Local Predictor:" in line:
        continue
      else:
        if last_line_star:
          #Fix spacing after ******
          ret.append("  "+line)
          last_line_star = False
        elif start_core:
          ret.append(line.replace(" ", "", 2))
        else:
          ret.append(line)
    return ret

  def line_to_dict(line):
    ret = {}
    temp = line.split(":")[0].split("=")
    ret["lspace"] = len(temp[0]) - len(temp[0].lstrip())
    return ret

  def split_list(lines):
    core_id = 0
    ret = []
    sub = []
    for i in lines:
      if "Core:" in i:
        i = i.replace("Core:", "Core"+str(core_id)+":")
        core_id += 1

      if i == "\n":
        ret.append(sub)
        sub = []
      else:
        sub.append(i.rstrip())
    return ret

  def to_devices(intermediate_dev_list):
    ret = []
    for dev in intermediate_dev_list:
      data = {}
      #print(dev)
      for attr in dev[1:]:
        data[attr.split("=")[0].strip()] = attr.split("=")[1].strip()
      ret.append(Device(dev[0].split(":")[0].strip(), data, \
        int(math.floor((len(dev[0]) - len(dev[0].lstrip()))/2))))
      if ret[-1].depth == 4:
        ret[-1].depth = 3
      if ret[-1].depth == 5:
        ret[-1].depth = 3
      if ret[-1].depth == 6:
        ret[-1].depth = 4
    return ret

  """ Returns an Epochs """
  with open(output_file, "r") as of:
    lines = of.readlines()
    lines = strip_header(lines)
    lines = strip_space(lines)
    temp = split_list(lines)
    dev_list = to_devices(temp)
    epoch = Epoch(dev_list)

    return epoch

#first time running mcpat?
first_time = True

def run_mcpat(xml, print_level, opt_for_clk, ofile, errfile):
  global first_time
  from m5 import options
  mcpat_output_path = os.path.join(options.mcpat_out,
                                   options.mcpat_testname)
  mcpat_exe = os.path.join(options.mcpat_path, "mcpat")
  mcpat_serial = os.path.join(mcpat_output_path, "mcpat_serial.txt")

  #if first time first generate a checkpoint before mcpat calculation
  if(first_time):
    mcpat = [mcpat_exe,
      "-i",
      xml,
      "-p",
      "5",
      "--serial_create=true",
      "--serial_file="+mcpat_serial]
    first_time = False
    with open(ofile, "w") as ostd, open(errfile, "w") as oerr:
      p = subprocess.Popen(mcpat, stdout=ostd, stderr=oerr)
      p.wait()
  
  mcpat = [mcpat_exe,
    "-i",
    xml,
    "-p",
    "5",
    "--serial_restore=true",
    "--serial_file="+mcpat_serial]

  with open(ofile, "w") as ostd, open(errfile, "w") as oerr:
    p = subprocess.Popen(mcpat, stdout=ostd, stderr=oerr)
    p.wait()

def get_data(path, mcpat_trees):
  data = {}
  def filter(value):
    if "nan" in value.split(" ")[0] or "inf" in value.split(" ")[0]:
      # McPAT Messed Up?
      return "0"
    else:
      return value.split(" ")[0]

  for key, value in mcpat_trees[-1].find(path).data.items():
    data[key] = filter(value.split(" ")[0])
  return data

def calc_total_power(data, power_gating = False, scale_factor=1.0):
  # Add Runtime Dynamic to Gate Leakage and Subthreshold Leakage with Power
  # Gating
  print("calc_total_power=",power_gating)
  print("gate leakage ", float(data["Gate Leakage"]))
  print("Subthreshold Leakage with power gating ", float(data["Subthreshold Leakage with power gating"]))
  print("Runtime Dynamic ", float(data["Runtime Dynamic"]))
  print("sum ", float(data["Gate Leakage"]) + \
    float(data["Subthreshold Leakage with power gating"]) + \
    float(data["Runtime Dynamic"])*scale_factor)

  if power_gating:
    return (float(data["Gate Leakage"]) + \
           float(data["Subthreshold Leakage with power gating"]) + \
           float(data["Runtime Dynamic"]))*scale_factor
  return (float(data["Gate Leakage"]) + \
         float(data["Subthreshold Leakage"]) + \
         float(data["Runtime Dynamic"]))*scale_factor

def calc_req(power, voltage):
  return voltage*voltage/power

def dump_stats(mcpat_trees):
  ''' Dumps the tree data to csv '''
  from m5 import options

  mcpat_output_path = os.path.join(options.mcpat_out,
                                   options.mcpat_testname)
  testname = options.mcpat_testname
  cfile = os.path.join(mcpat_output_path, testname+"_all.csv")
  sfile = os.path.join(mcpat_output_path, testname+".csv")
  with open(sfile, "w") as csv, \
       open(cfile, "w") as full_dump:
    i = 0
    # Print the header line:
    mcpat_trees[0].print_csv_line_header(full_dump)
    # Print the header line:
    for epoch in mcpat_trees:
      epoch.print_csv_line_data(full_dump)
      data = get_data("Processor", mcpat_trees)

      # Calculate Total Power:
      power = calc_total_power(data)
      data = []
      req = calc_req(power, 1.0)
      data.append(str(i*float(options.power_profile_interval)))
      data.append(str(req))
      data.append(str(power))
      csv.write(",".join(data)+"\n")
      i+=1
