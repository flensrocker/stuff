#!/usr/bin/python

import netifaces

def get_mac(interface):
  return netifaces.ifaddresses(interface)[netifaces.AF_LINK][0]['addr']

def get_broadcast_addr(interface, protocol):
  return netifaces.ifaddresses(interface)[protocol][0]['broadcast']

# get_mac('eth0')
# get_broadcast_addr('eth0', netifaces.AF_INET)
