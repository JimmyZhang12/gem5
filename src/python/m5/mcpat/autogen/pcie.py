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
# pcie.py
#
# PCIe class definition

from xml.etree import ElementTree
from xml.dom import minidom

class PCIE:
  """ On chip PCIe controller, including Phy. For a minimum PCIe packet
  size of 84B at 8Gb/s per lane (PCIe 3.0), a new packet arrives every
  84ns. The low bound of clock rate of a PCIe per lane logic is 120Mhz
  Note: McPAT does not track individual pcie controllers, instead, it
  takes the total accesses and calculate the average power per pcie
  controller or per channel. This is sufficent for most application. """

  name = "pcie"
  id = "pcie"

  parameters = \
  {
    "type" : ["0","1: low power; 0 high performance"],
    "withPHY" : ["1",""],
    "clockrate" : ["350","Clock Rate in MHz"],
    "number_units" : ["0",""],
    "num_channels" : ["8","Possible values: 2 ,4 ,8 ,16 ,32"]
  }
  stats = \
  {
    "duty_cycle" : ["1.0","achievable max load <= 1.0"],
    "total_load_perc" : \
      ["0.0","Percentage of total achived load to total achivable "
        "bandwidth"]
  }

  def __init__(self, component_id, component_name, stat_dict, config_dict):
    self.name = component_name
    self.id = component_id

    # Init the PCIE Parameters and Stats:
    #parameters["type"][0]=
    #parameters["withPHY"][0]=
    #parameters["clockrate"][0]=
    #parameters["number_units"][0]=
    #parameters["num_channels"][0]=
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

