#!/usr/bin/python

import dbus
import dbus.service
import gobject
import netifaces
import socket
import struct

from dbus.mainloop.glib import DBusGMainLoop
DBusGMainLoop(set_as_default=True)

dbus_interface = 'de.yavdr.avahiwakeup'
interface = 'eth0'
hosts = {}


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


class dbusHostService(dbus.service.Object):
  def __init__(self, bus):
    bus_name = dbus.service.BusName(dbus_interface, bus = bus)
    dbus.service.Object.__init__(self, bus_name, '/host')

  @dbus.service.method(dbus_interface, in_signature = 's', out_signature = 'b')
  def WakeupHost(self, remote):
    lowerRemote = remote.lower()
    if lowerRemote not in hosts:
      return False
    broadcast = get_broadcast_addr(interface, netifaces.AF_INET)
    if not broadcast:
      return False
    print "wake up " + remote + " with MAC " + hosts[lowerRemote] + " on broadcast address " + broadcast
    wake_on_lan(hosts[lowerRemote], broadcast)
    return True


if __name__ == '__main__':
  hostname = socket.gethostname().lower()
  hosts[hostname] = get_mac(interface)
  hostService = dbusHostService(dbus.SystemBus())
  print 'host ' + hostname + ' has MAC ' + hosts[hostname]
  gobject.MainLoop().run()
