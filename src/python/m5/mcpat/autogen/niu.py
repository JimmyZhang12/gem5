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
# niu.py
#
# Network Interface Unit class definition

from xml.etree import ElementTree
from xml.dom import minidom

class NIU:
  """ On chip 10Gb Ethernet NIC, including XAUI Phy and
  MAC controller. For a minimum IP packet size of 84B
  at 10Gb/s, a new packet arrives every 67.2ns.  the
  low bound of clock rate of a 10Gb MAC is 150Mhz Note:
  McPAT does not track individual nic, instead, it
  takes the total accesses and calculate the average
  power per nic or per channel. This is sufficent for
  most application. """

  name = "niu"
  id = "niu"

  parameters = \
  {
    "type" : ["0","1: low power; 0 high performance"],
    "clockrate" : ["350","Clock Rate in MHz"],
    "number_units" : \
      ["0","unlike PCIe and memory controllers, each Ethernet"
        "controller only have one port"]
  }
  stats = \
  {
    "duty_cycle" : ["1.0","achievable max load lteq 1.0"],
    "total_load_perc" : \
      ["0.0","ratio of total achived load to total achivable bandwidth"]
  }

  def __init__(self, component_id, component_name, \
                stat_dict, config_dict, sim_dict):
    self.name = component_name
    self.id = component_id

    # Init the NIU Parameters and Stats:
    #parameters["type"][0]=
    #parameters["clockrate"][0]=
    #parameters["number_units"][0]=
    #stats["duty_cycle"][0]=
    #stats["total_load_perc"][0]=

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

