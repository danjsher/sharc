import socket
import sys

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

server_address = ('localhost', 12345)
#server_address = ('192.168.23.1', 12345)
print >>sys.stderr, 'connecting to %s port %s' % server_address
sock.connect(server_address)

