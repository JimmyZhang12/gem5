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
# system.py
#
# Create a valid system from subcomponents

from xml.etree import ElementTree
from xml.dom import minidom

from core import Core
from cache import Cache
from noc import NoC
from mc import MemoryController
from niu import NIU
from pcie import PCIE
from flash import FlashController
from directory import Directory

from util import *

class System:
  name = "system"
  id = "system"

  cores = None
  l2cache = None
  l1directory = None
  l2directory = None
  l3 = None
  noc = None
  mem = None
  niu = None
  pcie = None
  flash = None

  # Parameters are a dictionary with the key as the parameter name,
  # and then a value comment pair
  parameters = \
  {
    "number_of_cores" : ["1","The number of cores"],
    "number_of_L1Directories" : ["0","The number of L1 Directories"],
    "number_of_L2Directories" : ["0","The number of L2 Directories"],
    "number_of_L2s" : ["1","The number of L2s in each Cluster"],
    "Private_L2" : ["1","1: Private; 0: shared/coherent"],
    "number_of_L3s" : ["1","Number of L3 Caches"],
    "number_of_NoCs" : ["1","Number of NoCs"],
    "homogeneous_cores" : \
      ["1","1: Homogeneous cores; 0: Heterogeneous core statistics"],
    "homogeneous_L2s" : \
      ["1","1: Homogeneous L2; Heterogeneous L2 statistics"],
    "homogeneous_L1Directories" : \
      ["1","1: Homogeneous L1 Directory; Heterogeneous L1 Directory "
        "statistics"],
    "homogeneous_L2Directories" : \
      ["1","1: Homogeneous L2 Directory; Heterogeneous L2 Directory "
        "statistics"],
    "homogeneous_L3s" : \
      ["1","1: Homogeneous L3; Heterogeneous L3 statistics"],
    "homogeneous_ccs" : \
      ["1","1: Homogeneous Cache Controller; Heterogeneous Cache "
        "Controller statistics"],
    "homogeneous_NoCs" : \
      ["1","1: Homogeneous Network on Chip; Heterogeneous Network on "
        "Chip statistics"],
    "core_tech_node" : ["22","Tech Process Node"],
    "target_core_clockrate" : ["1000","Core Clock Rate in MHz"],
    "temperature" : ["300","System Temperature in Kelvin"],
    "number_cache_levels" : ["3",""],
    "interconnect_projection_type" : \
      ["0","0: Aggressive Wire Technology; 1: Conservative Wire "
        "Technology"],
    "device_type" : \
      ["0","0: High Performance Type; 1: Low Standby Power; 2: Low "
        "Operating Power"],
    "longer_channel_device" : \
      ["1","0: No use; 1: Use when possible"],
    "power_gating" : ["1","0: not enabled; 1: enabled"],
    "machine_bits" : ["64","n-Bit machine"],
    "virtual_address_width" : ["64","n-Bit virtual address"],
    "physical_address_width" : ["52","m-Bit physical address"],
    "virtual_memory_page_size" : ["4096","Virtual memory page size"]
  }

  # Stats are a dictionary with the key as the parameter name, and
  # then a value comment pair
  stats = \
  {
    "total_cycles" : ["1", "Total CPU Cycles"],
    "idle_cycles" : ["1", "Total Idle Cycles"],
    "busy_cycles" : ["0", "Total Busy Cycles (Total - Idle)"]
  }

  def __init__(self, component_id, component_name, \
                stat_dict, config_dict, sim_dict):
    """ In the constructor, the parameters are populated from the
    config and the stats are populated from the stats. The
    subcomponents are also constructed and populated hierarchically
    """

    num_cpu = 0
    num_l3 = 0
    for i in config_dict["system.children"].split(" "):
      if "cpu" in i and "_" not in i:
        num_cpu+=1
      if "l3" in i and i != "tol3bus":
        num_l3+=1

    # Intialize the Parameters based on the config
    self.parameters["number_of_cores"][0] = str(num_cpu)
    self.parameters["number_of_L1Directories"][0] = str(num_cpu)
    self.parameters["number_of_L2Directories"][0] = str(num_cpu)
    self.parameters["number_of_L2s"][0] = str(num_cpu)
    self.parameters["Private_L2"][0] = str(1)
    self.parameters["number_of_L3s"][0] = str(num_l3)
    self.parameters["number_of_NoCs"][0] = str(1)
    self.parameters["homogeneous_cores"][0] = \
      str(1) if num_cpu == 1 else str(0)
    self.parameters["homogeneous_L2s"][0] = \
      str(1) if num_cpu == 1 else str(0)
    self.parameters["homogeneous_L1Directories"][0] = \
      str(1) if num_cpu == 1 else str(0)
    self.parameters["homogeneous_L2Directories"][0] = \
      str(1) if num_cpu == 1 else str(0)
    self.parameters["homogeneous_L3s"][0] = str(1)
    self.parameters["homogeneous_ccs"][0] = str(1)
    self.parameters["homogeneous_NoCs"][0] = str(1)
    self.parameters["core_tech_node"][0] = \
      config_dict["system.tech_node"] \
      if "system.tech_node" in config_dict.keys() else "90"
    self.parameters["target_core_clockrate"][0] = \
      str((1.0e-6/float(config_dict["system.clk_domain.clock"]))*1.0e12)
    self.parameters["temperature"][0] = str(sim_dict["temperature"])
    self.parameters["number_cache_levels"][0] = \
      str(3) if num_l3 != 0 else str(2)
    self.parameters["device_type"][0] = str(0)
    self.parameters["longer_channel_device"][0] = str(1)
    self.parameters["power_gating"][0] = str(1)
    self.parameters["machine_bits"][0] = str(64)
    self.parameters["virtual_address_width"][0] = str(64)
    self.parameters["physical_address_width"][0] = str(52)
    self.parameters["virtual_memory_page_size"][0] = str(4096)

    # Intialize the Parameters based on the stats
    self.stats["total_cycles"][0] = \
      str(int(stat_dict["system.cpu.numCycles"][1])) if(num_cpu==1) \
      else str(int(stat_dict["system.cpu0.numCycles"][1]))
    self.stats["idle_cycles"][0] = \
      str(int(stat_dict["system.cpu.idleCycles"][1])) if(num_cpu==1) \
      else str(int(stat_dict["system.cpu0.idleCycles"][1]))
    self.stats["busy_cycles"][0] = \
      str(int(stat_dict["system.cpu.numCycles"][1]) \
      -int(stat_dict["system.cpu.idleCycles"][1])) \
      if(num_cpu==1) else str(int(stat_dict["system.cpu0.numCycles"][1]) \
      -int(stat_dict["system.cpu0.idleCycles"][1]))

    assert(self.stats["total_cycles"] > 0)

    # Intialize all the devices
    self.core = \
    [ \
      Core \
      ( \
        self.id+".core"+str(i), \
        "core"+str(i), \
        prune_dict("system.cpu." if num_cpu==1 else "system.cpu"+str(i) \
          +".", stat_dict), \
        prune_dict("system.cpu." if num_cpu==1 else "system.cpu"+str(i) \
          +".", config_dict), \
        sim_dict \
      ) \
      for i in range(int(self.parameters["number_of_cores"][0])) \
    ]
    self.l2cache = \
    [ \
      Cache \
      ( \
        self.id+".L2"+str(i), \
        "L2"+str(i), \
        prune_dict("system.l2." if num_cpu==1 else "system.l2"+str(i) \
          +".", stat_dict), \
        prune_dict("system.l2." if num_cpu==1 else "system.l2"+str(i) \
          +".", config_dict), \
        sim_dict \
      ) \
      for i in range(int(self.parameters["number_of_cores"][0])) \
    ]
    self.l1directory = \
    [ \
      Directory \
      ( \
        self.id+".L1Directory"+str(i), \
        "L1Directory"+str(i), \
        stat_dict, \
        config_dict, \
        sim_dict \
      ) \
      for i in range(int(self.parameters["number_of_cores"][0])) \
    ]
    self.l2directory = \
    [ \
      Directory \
      ( \
        self.id+".L2Directory"+str(i), \
        "L2Directory"+str(i), \
        stat_dict, \
        config_dict, \
        sim_dict \
      ) \
      for i in range(int(self.parameters["number_of_cores"][0])) \
    ]
    self.l3 = \
    [ \
      Cache \
      ( \
        self.id+".L3"+str(i), \
        "L3"+str(i), \
        prune_dict("system.l3." if num_l3==1 else "system.l3"+str(i) \
          +".", stat_dict), \
        prune_dict("system.l3." if num_l3==1 else "system.l3"+str(i) \
          +".", config_dict), \
        sim_dict \
      ) \
      for i in range(int(self.parameters["number_of_L3s"][0])) \
    ]
    self.noc = \
    [ \
      NoC \
      ( \
        self.id+".NoC"+str(i), \
        "noc"+str(i), \
        stat_dict, \
        config_dict, \
        sim_dict, \
        num_cores=num_cpu \
      ) \
      for i in range(int(self.parameters["number_of_NoCs"][0])) \
    ]
    self.mc = MemoryController \
    ( \
      self.id+".mc", \
      "mc", \
      prune_dict("system.mem_ctrls.",stat_dict), \
      prune_dict("system.mem_ctrls.",config_dict), \
      sim_dict \
    )
    self.niu = NIU \
    ( \
      self.id+".niu", \
      "niu", \
      stat_dict, \
      config_dict, \
      sim_dict \
    )
    self.pcie = PCIE \
    ( \
      self.id+".pcie", \
      "pcie", \
      stat_dict, \
      config_dict \
    )
    self.flash = FlashController \
    ( \
      self.id+".flashc", \
      "flashc", \
      stat_dict, \
      config_dict \
    )

  def xml(self):
    """ Build an XML Tree from the parameters, stats, and
    subcomponents """
    top = ElementTree.Element('component', id=self.id, name=self.name)
    for key in sorted(self.parameters):
      top.append(ElementTree.Comment( \
        ", ".join(['param', key, self.parameters[key][1]])))
      top.append(ElementTree.Element( \
        'param', name=key, value=self.parameters[key][0]))
    for key in sorted(self.stats):
      top.append(ElementTree.Comment( \
        ", ".join(['stat', key, self.stats[key][1]])))
      top.append(ElementTree.Element( \
        'stat', name=key, value=self.stats[key][0]))
    for i in range(len(self.core)):
      top.append(self.core[i].xml())
    for l in self.l1directory:
      top.append(l.xml())
    for l in self.l2directory:
      top.append(l.xml())
    for l in self.l2cache:
      top.append(l.xml())
    for c in self.l3:
      top.append(c.xml())
    for n in self.noc:
      top.append(n.xml())
    top.append(self.mc.xml())
    top.append(self.niu.xml())
    top.append(self.pcie.xml())
    top.append(self.flash.xml())
    return top

