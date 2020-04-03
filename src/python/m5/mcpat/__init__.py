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
