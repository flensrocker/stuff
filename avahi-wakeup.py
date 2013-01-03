#!/usr/bin/python

import avahi
import dbus
import dbus.service
import gobject
import netifaces
import socket
import struct
import re

from dbus.mainloop.glib import DBusGMainLoop

service_type = '_host-wakeup._tcp'
service_port = 6666;

dbus_interface = 'de.yavdr.avahiwakeup'
interface = 'eth0'


def get_mac(interface):
    return netifaces.ifaddresses(interface)[netifaces.AF_LINK][0]['addr'].lower()

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


class HostService(dbus.service.Object):
    def __init__(self, bus):
        bus_name = dbus.service.BusName(dbus_interface, bus = bus)
        dbus.service.Object.__init__(self, bus_name, '/Hosts')
        self.Hosts = {}

    @dbus.service.method(dbus_interface, in_signature = 's', out_signature = 'b')
    def Wakeup(self, host):
        if not host:
            return False
        lowerHost = host.lower()
        if lowerHost not in self.Hosts:
            return False
        broadcast = get_broadcast_addr(interface, netifaces.AF_INET)
        if not broadcast:
            return False
        print "wake up " + host + " with MAC " + self.Hosts[lowerHost] + " on broadcast address " + broadcast
        wake_on_lan(self.Hosts[lowerHost], broadcast)
        return True

    @dbus.service.method(dbus_interface, in_signature = 'ss', out_signature = 'b')
    def Add(self, host, mac):
        if not host or not mac:
            return False
        lowerHost = host.lower().encode('ascii', 'ignore')
        lowerMac = mac.lower().encode('ascii', 'ignore')
        if (lowerHost in self.Hosts) and (self.Hosts[lowerHost] == lowerMac):
            return False
        print "add host " + host + " with MAC " + mac
        self.Hosts[lowerHost] = lowerMac
        print self.Hosts
        return True

    @dbus.service.method(dbus_interface, in_signature = 's', out_signature = 'b')
    def Remove(self, host):
        if not host:
            return False
        lowerHost = host.lower().encode('ascii', 'ignore')
        if lowerHost not in self.Hosts:
            return False
        print "remove host " + host
        del self.Hosts[lowerHost]
        print self.Hosts
        return True

    @dbus.service.method(dbus_interface, in_signature = '', out_signature = 'b')
    def Publish(self):
        txts = []
        for host in self.Hosts:
            txts.append("host=%s,mac=%s" % (host, self.Hosts[host]))
        avahiService.Publish(txts)
        return True


class AvahiService:
    def __init__(self, avahi_server, name, type, port):
        self.server = avahi_server
        self.name = name
        self.type = type
        self.port = port

    def Publish(self, txts):
        for txt in txts:
            print "publish: " + txt
        if not hasattr(self, 'group'):
            o = bus.get_object(avahi.DBUS_NAME, self.server.EntryGroupNew())
            self.group = dbus.Interface(o, avahi.DBUS_INTERFACE_ENTRY_GROUP)
        else:
            self.group.Reset()
        self.group.AddService(avahi.IF_UNSPEC, avahi.PROTO_UNSPEC, dbus.UInt32(0),
                         self.name, self.type, '', '', dbus.UInt16(self.port), txts)
        self.group.Commit()


def avahi_error_handler(*args):
    print 'avahi-error: ' + args[0]

def avahi_service_resolved(*args):
    name = args[2]
    address = args[7]
    port = args[8]
    txts = args[9]
    publish = False
    for t in txts:
        s = "".join(chr(b) for b in t)
        match = re.search("host=(.+),mac=(.+)", s)
        host = match.group(1)
        mac = match.group(2)
        if match and host:
            print "found host %s with mac %s" % (host, mac)
            if hostService.Add(host, mac):
                publish = True
    if publish:
        hostService.Publish()

def avahi_new_handler(interface, protocol, name, stype, domain, flags):
    # if flags & avahi.LOOKUP_RESULT_LOCAL:
    #     pass
    # else:
    avahi_server.ResolveService(interface, protocol, name, stype, 
                domain, avahi.PROTO_UNSPEC, dbus.UInt32(0), 
                reply_handler = avahi_service_resolved, error_handler = avahi_error_handler)


if __name__ == '__main__':
    DBusGMainLoop(set_as_default=True)
    bus = dbus.SystemBus()

    avahi_server = dbus.Interface(bus.get_object(avahi.DBUS_NAME, avahi.DBUS_PATH_SERVER), avahi.DBUS_INTERFACE_SERVER)

    hostname = socket.gethostname().lower()
    mac = get_mac(interface)
    hostService = HostService(bus)
    hostService.Add(hostname, mac)
    print 'host ' + hostname + ' has MAC ' + mac

    avahi_browser = dbus.Interface(bus.get_object(avahi.DBUS_NAME,
                    avahi_server.ServiceBrowserNew(avahi.IF_UNSPEC, avahi.PROTO_INET, service_type, 'local', dbus.UInt32(0))),
                    avahi.DBUS_INTERFACE_SERVICE_BROWSER)
    avahi_browser.connect_to_signal("ItemNew", avahi_new_handler)

    avahiService = AvahiService(avahi_server, 'avahi-wakeup on ' + hostname, service_type, service_port)
    
    hostService.Publish()

    gobject.MainLoop().run()
