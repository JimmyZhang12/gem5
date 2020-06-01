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

def initialize(name, step):
  from m5 import options
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

  def verilog_thread(name, step):
    """ This is the thread function for executing the verilog sim """
    run_command([os.path.join(options.ncverilog_path,"run_cadence.sh"), \
                name, str(step)])

  if os.path.exists(os.path.join("/dev/shm", name)):
    os.remove(os.path.join("/dev/shm", name))

  thread = Thread(target=verilog_thread, args=[name, step])
  thread.setDaemon(True)
  thread.start()

  # Wait for the container to launch and the sim to run
  while not os.path.isfile(os.path.join("/dev/shm", name)):
    sleep(1)
  vpi.create_shm(0, name)
  return

def set_driver_signals(voltage_setpoint, load, term_sim):
  vpi.set_driver_signals(voltage_setpoint, load, term_sim)

def get_voltage():
  return vpi.get_voltage()

def get_current():
  return vpi.get_current()

def ack_supply():
  return vpi.ack_supply()

def stop():
  subprocess.Popen(['reset']).wait()
