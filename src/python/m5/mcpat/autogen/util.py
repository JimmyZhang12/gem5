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
# util.py
#
# Misc functions to handle I/O and building of the dictionaries

import os
import sys
import re
from file_read_backwards import FileReadBackwards
from functools import reduce

def build_gem5_stat_dict(file):
  """ Build a dict of stats from the Gem5 stats.txt """
  stats = {}
  with FileReadBackwards(file, encoding="utf-8") as sf:
    for line in sf:
      if line.strip() == "":
        continue
      elif "End Simulation Statistics" in line:
        stats = {}
      elif "Begin Simulation Statistics" in line:
        return stats
      else:
        stat = []
        sstr = re.sub('\s+', ' ', line).strip()
        if('-----' in sstr):
          continue
        elif(sstr == ''):
          continue
        elif(sstr.split(' ')[1] == '|'):
          # Ruby Stats
          l = []
          for i in sstr.split('|')[1:]:
            l.append(i.strip().split(' '))
          stat.append("ruby_multi")
          stat.append(l)
        else:
          stat.append("single")
          stat.append(sstr.split(' ')[1])
        stats[sstr.split(' ')[0]] = stat
  return stats

def build_gem5_config_dict(file):
  """ Build a dict of system parameters from the Gem5 config.ini """
  config = {}
  hierarchy = ""
  with open(file, "r") as cf:
    lines = cf.readlines()
    for line in lines:
      cstr = re.sub('\s+', ' ', line).strip()
      if(cstr == ''):
        continue
      if("[" in cstr and "]" in cstr):
        hierarchy=cstr.replace("[", "").replace("]", "")+"."
        continue
      else:
        #print(hierarchy, cstr)
        config[hierarchy+cstr.split('=')[0]] = cstr.split('=')[1].strip()
  return config

def build_gem5_sim_dict(**kwargs):
  """ Build a sim_dict of parameters & stats coming directly from the runtime,
  such as instantaneous voltage, and temperature of the device"""
  sim_dict = {}
  for key, val in kwargs.items():
    sim_dict[key] = val
  return sim_dict

def prune_dict(path, d):
  """ Returns a new dictionary of all the items on the specified path """
  new = {}
  for key, val in d.items():
    if path in key:
      new[key.replace(path, "")]=val
    if "system.clk_domain.clock" in key:
      new[key.replace("system.clk_domain.","")]=val
  return new

def get_noc_dimensions(nc):
  def factors(n):
    return set(reduce(list.__add__, ([i, n//i] for i in \
                      range(1, int(n**0.5) + 1) if n % i == 0)))
  def build_pairs(n, l):
    f_pairs = []
    for i in range(len(l)):
      for j in range(len(l)):
        if l[i]*l[j] == n:
          f_pairs.append([l[i], l[j]])
    return f_pairs
  f = build_pairs(nc,list(factors(nc)))
  min_sum = 100000
  min_x = 0
  min_y = 0
  for elem in f:
    if(sum(elem) < min_sum):
      min_sum = sum(elem)
      min_x = elem[0]
      min_y = elem[1]
  return min_x, min_y

