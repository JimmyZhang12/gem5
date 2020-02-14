import os
import sys
import re
from m5.SimObject import SimObject
from m5.util import fatal
from m5.params import *

epoch = 0
epoch_data_trees = None

def run():
  from m5 import options
  global epoch
  epoch += 1
  print("Running McPat"+str(epoch)+" read option: "+options.mcpat)
