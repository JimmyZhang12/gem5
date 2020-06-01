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
# noc.py
#
# NoC class definition

from xml.etree import ElementTree
from xml.dom import minidom
from util import *

class NoC:
  def __init__(self, component_id, component_name, \
                stat_dict, config_dict, sim_dict, **kwargs):
    self.name = "noc"
    self.id = "noc"

    self.parameters = \
    {
      "clockrate" : ["1000","Clock rate in MHz"],
      "vdd" : ["0","0 means using ITRS default vdd"],
      "power_gating_vcc" : \
        ["-1","\"-1\" means using default power gating virtual power "
          "supply voltage constrained by technology and computed "
          "automatically"],
      "type" : \
        ["1","0:bus, 1:NoC , for bus no matter how many nodes sharing "
          "the bus at each time only one node can send req"],
      "horizontal_nodes" : \
        ["1","Vertical Nodes in NoC; Comes from chip layout"],
      "vertical_nodes" : \
        ["1","Horizontal Nodes in NoC; Comes from chip layout"],
      "has_global_link" : \
        ["0","1 has global link, 0 does not have global link"],
      "link_throughput" : ["1","Link throughput w.r.t clock"],
      "link_latency" : ["1","Link latency w.r.t clock"],
      "input_ports" : ["1",""],
      "output_ports" : ["1","For bus the I/O ports should be 1"],
      "flit_bits" : ["256",""],
      "chip_coverage" : \
        ["1","When multiple NOC present, one NOC will cover part of "
          "the whole chip. chip_coverage <=1"],
      "link_routing_over_percentage" : \
        ["0.5","Links can route over other components or occupy whole "
          "area.  by default, 50% of the NoC global links routes over "
          "other components"]
    }
    self.stats = \
    {
      "total_accesses" : \
        ["1","This is the number of total accesses within the whole "
          "network not for each router (pkt_count::total & bus in key)"],
      "duty_cycle" : ["1",""]
    }
    self.name = component_name
    self.id = component_id

    x,y = get_noc_dimensions(kwargs["num_cores"])

    # Init the NoC Parameters and Stats:
    self.parameters["clockrate"][0]= \
      str((1.0e-6/float(config_dict["system.clk_domain.clock"]))*1.0e12)
    self.parameters["vdd"][0]=str(float(sim_dict["voltage"]))
    self.parameters["power_gating_vcc"][0]="-1"
    self.parameters["type"][0]="1"
    self.parameters["horizontal_nodes"][0]=str(x)
    self.parameters["vertical_nodes"][0]=str(y)
    self.parameters["has_global_link"][0]=str(0)
    self.parameters["link_throughput"][0]=str(1)
    self.parameters["link_latency"][0]=str(1)
    self.parameters["input_ports"][0]=str(2)
    self.parameters["output_ports"][0]=str(2)
    self.parameters["flit_bits"][0]=str(256)
    self.parameters["chip_coverage"][0]=str(1.0)
    self.parameters["link_routing_over_percentage"][0]=str(0.5)

    # Calculate the sum of all the packet counts:
    s = 0
    for key in stat_dict:
      if "pkt_count::total" in key and "bus" in key:
        s += int(stat_dict[key][1])

    self.stats["total_accesses"][0]=str(s)
    self.stats["duty_cycle"][0]=str(1.0)

  def xml(self):
    """ Build an XML Tree from the parameters, stats, and subcomponents """
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
    return top

