import os
import sys
import re
import pickle
import subprocess
import math
from collections import defaultdict

from device import Device

class Node(object):
  def __init__(self, children=[], device=None):
    self.children = children
    self.device = device
  def __str__(self):
    modules = []
    modules.append(str(self.device))
    for child in self.children:
      modules.append(str(child))
    return "\n".join(modules)
  def __repr__(self):
    modules = []
    modules.append(str(self.device))
    for child in self.children:
      modules.append(str(child))
    return "\n".join(modules)
