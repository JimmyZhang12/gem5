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

iter = 0
mcpat_trees = []

class Device(object):
  def __init__(self, name="", data={}, depth=0):
    self.name = name
    self.data = data
    self.depth = depth
  def __str__(self):
    return "  "*self.depth+self.name+" "+str(self.data)
  def __repr__(self):
    return "  "*self.depth+self.name+" "+str(self.data)

class Node(object):
  def __init__(self, children=[], device=None):
    self.children = children
    self.device = device
  def __str__(self):
    modules = []
    modules.append(str(self.device))
    for child in self.children:
      modules.append(str(child))
    return "\n".join(modules)
  def __repr__(self):
    modules = []
    modules.append(str(self.device))
    for child in self.children:
      modules.append(str(child))
    return "\n".join(modules)

class Epoch(object):
  def __init__(self, device_list = []):
    if len(device_list) != 0:
      self.dev_tree = self.build(device_list)
    else:
      self.dev_tree = None
  def __str__(self):
    return str(self.dev_tree)
  def __repr__(self):
    return str(self.dev_tree)

  def build(self, devices):
    """ Base Cases """
    if len(devices) == 0:
      return None
    if len(devices) == 1 and isinstance(devices[0], Device):
      return Node([], devices[0])

    """ Recursive Case """
    root = devices[0]
    children = []
    sublist = []
    for dev in devices[1:]:
      if dev.depth == root.depth + 1:
        node = self.build(sublist)
        if node != None:
          children.append(node)
          #print(node)
        sublist = []
      sublist.append(dev)
    node = self.build(sublist)
    if node != None:
      children.append(node)

    """ Post-Order Build Tree: """
    return Node(children, root)

  def find(self, path):
    def _find(path, subtree):
      #print(path, subtree.device.name)

      """ Base Case """
      if path.split(":")[0] == subtree.device.name \
        and len(path.split(":")) == 1:
        return subtree.device
      elif len(path.split(":")) == 1:
        return None

      """ Recursive Case """
      for i in subtree.children:
        if i.device.name == path.split(":")[1]:
          return _find(":".join(path.split(":")[1:]), i)
      return None

    return _find(path, self.dev_tree)

  def print_csv_line_data(self, file):
    devices = []
    def _traverse(subtree, devices):
      devices.append(subtree.device)
      """ Base Case """
      if subtree == None:
        return
      """ Recursive Case """
      for i in subtree.children:
        _traverse(i, devices)
      return
    _traverse(self.dev_tree, devices)
    for i in devices:
      for key,value in i.data.items():
        file.write(str(value)+",")
    file.write("\n")
    return

  def print_csv_line_header(self, file):
    devices = []
    def _traverse(subtree, devices):
      devices.append(subtree.device)

      """ Base Case """
      if subtree == None:
        return

      """ Recursive Case """
      for i in subtree.children:
        _traverse(i, devices)
      return

    _traverse(self.dev_tree, devices)

    for i in devices:
      file.write(i.name+" ")
      for key,value in i.data.items():
        file.write(str(key)+",")
    file.write("\n")
    return

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
    ret = []
    sub = []
    for i in lines:
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

def run_mcpat(xml, print_level, opt_for_clk, ofile, errfile):
  from m5 import options
  mcpat_exe = os.path.join(options.mcpat_path, "mcpat")
  mcpat = [mcpat_exe,
    "-infile",
    xml,
    "-print_level",
    print_level,
    "-opt_for_clk",
    opt_for_clk]
  #print(" ".join(mcpat))
  with open(ofile, "w") as ostd, open(errfile, "w") as oerr:
    p = subprocess.Popen(mcpat, stdout=ostd, stderr=oerr)
    p.wait()

def parse_stats(stat_file):
  epoch = []
  stats = {}
  with open(stat_file, "r") as sf:
    for line in sf:
      if line.strip() == "":
        continue
      elif "End Simulation Statistics" in line:
        epoch.append(stats)
      elif "Begin Simulation Statistics" in line:
        stats = {}
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
        stats["stats."+sstr.split(' ')[0]] = stat
  print("Read "+str(len(epoch))+" Epochs")
  return epoch

def print_stats(stats):
  for sname, stat in stats.items():
    if(stat[0] == "single"):
      print(sname+" "+stat[1])
    elif(stat[0] == "ruby_multi"):
      print(sname+" "+" ".join(" ".join(x) for x in stat[1]))

def parse_config(config_file):
  config = {}
  hierarchy = ""
  with open(config_file, "r") as cf:
    lines = cf.readlines()
    for line in lines:
      cstr = re.sub('\s+', ' ', line).strip()
      if(cstr == ''):
        continue
      if("[" in cstr and "]" in cstr):
        hierarchy="config."+cstr.replace("[", "").replace("]", "")+"."
        continue
      else:
        config[hierarchy+cstr.split('=')[0]] = cstr.split('=')[1].strip()
  return config

def print_config(config):
  for cname, config in config.items():
    print(cname+" "+config)

# replace
# Replaces the REPLACE{...} with the appropriate value from the dictionary
# Returns a string with the substituted lines
stat_trace = []
def replace(xml_line, stats, config, used_stats):
  if('REPLACE{' in xml_line):
    split_line = re.split('REPLACE{|}', xml_line)
    #print(split_line)
    keys = [x.strip().split(" ") for x in re.split(',', split_line[1])]
    for i in range(len(keys)):
      for j in range(len(keys[i])):
        if "stats" in keys[i][j]:
          if keys[i][j] in stats:
            used_stats[keys[i][j]] = stats[keys[i][j]][1]
          else:
            used_stats[keys[i][j]] = "0"
        if keys[i][j] in stats:
          keys[i][j] = str(float(stats[keys[i][j]][1]))
        elif keys[i][j] in config:
          keys[i][j] = str(float(config[keys[i][j]]))
        elif "stats" in keys[i][j] or "config" in keys[i][j]:
          keys[i][j] = "0"

    split_line[1] = ",".join([" ".join(y) for y in keys])
    # Evaluate
    expr = split_line[1].split(",")
    for i in range(len(expr)):
      try:
        expr[i] = str(int(max(math.ceil(eval(expr[i])),0)))
      except:
        expr[i] = "0"
    split_line[1] = ",".join(expr)

    #print(keys)
    #print("".join(split_line))
    return "".join(split_line)
  return xml_line

def dump():
  ''' Dumps the tree data to csv '''
  from m5 import options
  global mcpat_trees
  global stat_trace

  def calc_total_power(data):
    # Add Runtime Dynamic to Gate Leakage and Subthreshold Leakage with Power
    # Gating
    return float(data[0]) + float(data[7]) + float(data[5])

  def calc_req(power, voltage):
    return voltage*voltage/power

  mcpat_output_path = os.path.join(options.mcpat_out,
                                   options.mcpat_testname)
  testname = options.mcpat_testname
  cfile = os.path.join(mcpat_output_path, testname+"_all.csv")
  gfile = os.path.join(mcpat_output_path, testname+"_g5.csv")
  sfile = os.path.join(mcpat_output_path, testname+".csv")
  with open(sfile, "w") as csv, \
       open(cfile, "w") as full_dump, \
       open(gfile, "w") as m5_csv:
    i = 0
    # Print the header line:
    mcpat_trees[0].print_csv_line_header(full_dump)
    # Print the header line:
    for key,value in sorted(stat_trace[0].items()):
      m5_csv.write(key+",")
    m5_csv.write("\n")
    for epoch in mcpat_trees:
      epoch.print_csv_line_data(full_dump)
      for key,value in sorted(stat_trace[i].items()):
        m5_csv.write(value+",")
      m5_csv.write("\n")
      data_line = []
      header = []
      for key, value in epoch.find("Processor").data.items():
        header.append(key)
        data_line.append(value.split(" ")[0])

      # Calculate Total Power:
      power = calc_total_power(data_line)
      data = []
      #data.append(str(power))
      #header.append("Total Power")
      req = calc_req(power, 1.0)
      data.append(str(i*float(options.power_profile_interval)))
      data.append(str(req))
      data.append(str(power))
      #header.append("Req")
      csv.write(",".join(data)+"\n")
      #print(",".join(data))
      i+=1


def m5_to_mcpat():
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
  i_f = os.path.join(mcpat_output_path,"mp_arm_"+str(iter)+".xml")
  o_f = os.path.join(mcpat_output_path,"mp_"+str(iter)+".out")
  e_f = os.path.join(mcpat_output_path,"mp_"+str(iter)+".err")
  with open(t_f, "r") as tf, open(i_f, "w") as inf:
    in_xml = tf.readlines()
    out_xml = []
    s = {}
    for line in in_xml:
      out_xml.append(replace(line, epoch, config, s))
    # To trace the stats reported
    stat_trace.append(s)
    inf.writelines(out_xml)
  run_mcpat(i_f, "5", "1", o_f, e_f)
  mcpat_trees.append(parse_output(o_f))
  print(mcpat_trees[-1].find("Processor").data)

  #sfile = os.path.join(mcpat_output_path, testname+".pickle")

  #with open(sfile, "w") as mpe:
  #  pickle.dump(mcpat_trees, mpe)
  #with open(sfile, "r") as mpe:
  #  mcpat_trees = pickle.load(mpe)
  #plot(mcpat_trees, testname, mcpat_output_path)
