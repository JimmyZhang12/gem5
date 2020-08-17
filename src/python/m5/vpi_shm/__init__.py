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

import math
import os
import sys
import re
import subprocess
import math
from threading import Thread
from time import sleep

import _m5.vpi_shm as vpi

thread = None

valid_pdn_types = ["HARVARD", "HARVARD_M", "HARVARD_L",
                   "HARVARD_D", "ARM", "INTEL_M", "INTEL_DT"]

def initialize(name):
  from m5 import options
  pdn_type = options.power_supply_type
  if pdn_type not in valid_pdn_types:
    pdn_types = ",".join(valid_pdn_types)
    print("Error, Invalid PDN Type: \""+
          pdn_type+"\", must be of type: "+pdn_types)
    sys.exit(1)

  time_to_next = str(vpi.get_time_to_next())+"p"

  global thread
  """ This function will launch the docker container for the verilog
  simulation. """
  def run_command(command):
    process = subprocess.Popen(command, stdout=subprocess.PIPE)
    while True:
      output = process.stdout.readline()
      if output == '' and process.poll() is not None:
        break
      #if output:
        #print(output.strip(), flush=True)
    rc = process.poll()
    return rc

  def verilog_thread(name, pdn_type, ttn):
    """ This is the thread function for executing the verilog sim """
    run_command([os.path.join(options.ncverilog_path,"run_cadence.sh"), \
                name, pdn_type, ttn])

  if os.path.exists(os.path.join("/dev/shm", name)):
    os.remove(os.path.join("/dev/shm", name))

  thread = Thread(target=verilog_thread, args=[name, pdn_type, \
              time_to_next])
  thread.setDaemon(True)
  thread.start()

  # Wait for the container to launch and the sim to run
  while not os.path.isfile(os.path.join("/dev/shm", name)):
    sleep(1)
  vpi.create_shm(0, name)
  return

def set_driver_signals(load, term_sim, i=0):
  vpi.set_driver_signals(load, term_sim, i)

def get_voltage():
  return vpi.get_voltage()

def get_current():
  return vpi.get_current()

def ack_supply():
  return vpi.ack_supply()

def mp_get_freq(i = 0):
  return vpi.mp_get_freq(i)

def mp_get_voltage_set(i = 0):
  return vpi.mp_get_voltage_set()

def mp_get_ncores():
  return vpi.mp_get_ncores()

def stop():
  subprocess.Popen(['reset']).wait()
