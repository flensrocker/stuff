#!/usr/bin/python

import netifaces
import socket
import struct


def get_mac(interface):
  return netifaces.ifaddresses(interface)[netifaces.AF_LINK][0]['addr']

def get_broadcast_addr(interface, protocol):
  return netifaces.ifaddresses(interface)[protocol][0]['broadcast']

def wake_on_lan(macaddress, broadcast):
  if len(macaddress) == 12:
      pass
  elif len(macaddress) == 12 + 5:
      sep = macaddress[2]
      macaddress = macaddress.replace(sep, '')
  else:
      raise ValueError('Incorrect MAC address format')
  data = ''.join(['FFFFFFFFFFFF', macaddress * 20])
  send_data = '' 
  for i in range(0, len(data), 2):
      send_data = ''.join([send_data, struct.pack('B', int(data[i: i + 2], 16))])
  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
  sock.sendto(send_data, (broadcast, 7))

# mac = get_mac('eth0')
# broadcast = get_broadcast_addr('eth0', netifaces.AF_INET)
# wake_on_lan(mac, broadcast)
