import os
import sys
import re
import pickle
import subprocess
import math
from collections import defaultdict

from node import Node
from device import Device

class Epoch(object):
  def __init__(self, device_list = []):
    if len(device_list) != 0:
      self.dev_tree = self.build(device_list)
    else:
      self.dev_tree = None
  def __str__(self):
    return str(self.dev_tree)
  def __repr__(self):
    return str(self.dev_tree)

  def build(self, devices):
    """ Base Cases """
    if len(devices) == 0:
      return None
    if len(devices) == 1 and isinstance(devices[0], Device):
      return Node([], devices[0])

    """ Recursive Case """
    root = devices[0]
    children = []
    sublist = []
    for dev in devices[1:]:
      if dev.depth == root.depth + 1:
        node = self.build(sublist)
        if node != None:
          children.append(node)
          #print(node)
        sublist = []
      sublist.append(dev)
    node = self.build(sublist)
    if node != None:
      children.append(node)

    """ Post-Order Build Tree: """
    return Node(children, root)

  def find(self, path):
    def _find(path, subtree):
      #print(path, subtree.device.name)

      """ Base Case """
      if path.split(":")[0] == subtree.device.name \
        and len(path.split(":")) == 1:
        return subtree.device
      elif len(path.split(":")) == 1:
        return None

      """ Recursive Case """
      for i in subtree.children:
        if i.device.name == path.split(":")[1]:
          return _find(":".join(path.split(":")[1:]), i)
      return None

    return _find(path, self.dev_tree)

  def print_csv_line_data(self, file):
    devices = []
    def _traverse(subtree, devices):
      devices.append(subtree.device)
      """ Base Case """
      if subtree == None:
        return
      """ Recursive Case """
      for i in subtree.children:
        _traverse(i, devices)
      return
    _traverse(self.dev_tree, devices)
    for i in devices:
      for key,value in i.data.items():
        file.write(str(value)+",")
    file.write("\n")
    return

  def print_csv_line_header(self, file):
    devices = []
    def _traverse(subtree, devices):
      devices.append(subtree.device)

      """ Base Case """
      if subtree == None:
        return

      """ Recursive Case """
      for i in subtree.children:
        _traverse(i, devices)
      return

    _traverse(self.dev_tree, devices)

    for i in devices:
      file.write(i.name+" ")
      for key,value in i.data.items():
        file.write(str(key)+",")
    file.write("\n")
    return
