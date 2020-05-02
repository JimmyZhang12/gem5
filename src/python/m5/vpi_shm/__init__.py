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

def set_driver_signals(voltage_setpoint, resistance, term_sim):
  vpi.set_driver_signals(voltage_setpoint, resistance, term_sim)

def get_voltage():
  return vpi.get_voltage()

def get_current():
  return vpi.get_current()

def ack_supply():
  return vpi.ack_supply()

def stop():
  subprocess.Popen(['reset']).wait()
