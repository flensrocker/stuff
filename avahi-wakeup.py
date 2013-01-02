#!/usr/bin/python

import netifaces

def get_mac(interface):
  return netifaces.ifaddresses(interface)[netifaces.AF_LINK][0]['addr']

