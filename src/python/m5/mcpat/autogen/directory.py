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
# directory.py
#
# Directory Controller class definition

from xml.etree import ElementTree
from xml.dom import minidom

class Directory:
  """ Altough there are multiple access types,
  Performance simulator needs to cast them into reads
  or writes e.g. the invalidates can be considered as
  writes """

  name = "directory"
  id = "directory"

  parameters = \
  {
    "Directory_type" : ["0",""],
    "Dir_config" : \
      ["512,4,0,1,1, 1","cam based shadowed tag. 1 directory cache"],
    "buffer_sizes" : \
      ["16, 16, 16, 16","the parameters are capacity,block_width,"
        "associativity,bank, throughput w.r.t. core clock, latency w.r.t. core"
        "clock, all the buffer related are optional"],
    "clockrate" : ["1000","Clock rate in MHz"],
    "ports" : ["1,1,1","number of r, w, and rw search ports"],
    "device_type" : ["0",""]
  }
  stats = \
  {
    "read_accesses" : ["0","Read Accesses to the directory controller"],
    "write_accesses" : ["0","Write Accesses to the directory controller"],
    "read_misses" : ["0","Read Misses"],
    "write_misses" : ["0","Write Misses"],
    "conflicts" : ["0","Conflicts"]
  }

  def __init__(self, component_id, component_name, \
                stat_dict, config_dict, sim_dict):
    self.name = component_name
    self.id = component_id

    # Init the Directory Parameters and Stats:
    #parameters["Directory_type"][0]=
    #parameters["Dir_config"][0]=
    #parameters["buffer_sizes"][0]=
    #parameters["clockrate"][0]=
    #parameters["ports"][0]=
    #parameters["device_type"][0]=
    #stats["read_accesses"][0]=
    #stats["write_accesses"][0]=
    #stats["read_misses"][0]=
    #stats["write_misses"][0]=
    #stats["conflicts"][0]=

  def xml(self):
    """ Build an XML Tree from the parameters, stats, and subcomponents """
    top = ElementTree.Element('component', id=self.id, name=self.name)
    for key in sorted(self.parameters):
      top.append(ElementTree.Comment( \
        ", ".join(['param', key, self.parameters[key][1]])))
      top.append(ElementTree.Element(
        'param', name=key, value=self.parameters[key][0]))
    for key in sorted(self.stats):
      top.append(ElementTree.Comment(
        ", ".join(['stat', key, self.stats[key][1]])))
      top.append(ElementTree.Element(
        'stat', name=key, value=self.stats[key][0]))
    return top


