import os
import sys
import re
import pickle
import subprocess
import math
from collections import defaultdict

class Device(object):
  def __init__(self, name="", data={}, depth=0):
    self.name = name
    self.data = data
    self.depth = depth
  def __str__(self):
    return "  "*self.depth+self.name+" "+str(self.data)
  def __repr__(self):
    return "  "*self.depth+self.name+" "+str(self.data)
