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
# tlb.py
#
# Translation Lookaside Buffer class definition

from xml.etree import ElementTree
from xml.dom import minidom

class TLB:
  def __init__(self, component_id, component_name, \
                stat_dict, config_dict, sim_dict):
    self.name = "tlb"
    self.id = "tlb"

    self.parameters = \
    {
      "number_entries" : ["64","TLB Size; tlb.size"],
    }
    self.stats = \
    {
      "total_accesses" : \
        ["","Total Acceses; dtb_walker_cache.tags.data_accesses"],
      "total_misses" : \
        ["","Total Misses; dtb_walker_cache.tags.data_accesses"],
      "conflicts" : ["0","Conflicts to entries in the TLB"],
    }

    self.name = component_name
    self.id = component_id

    # Init the TLB Parameters and Stats:
    self.parameters["number_entries"][0]=str(int(config_dict["size"]))
    self.stats["total_accesses"][0]= \
      str(int(stat_dict["tags.tag_accesses"][1]))
    self.stats["total_misses"][0]= \
      str(int(stat_dict["replacements"][1]))
    self.stats["conflicts"][0]=str(int(stat_dict["replacements"][1]))

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

