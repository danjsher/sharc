import socket
import sys
import threading
import Queue

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

server_address = ('localhost', 12345)
#server_address = ('192.168.23.1', 12345)
print >>sys.stderr, 'connecting to %s port %s' % server_address
sock.connect(server_address)

send_buffer = Queue.Queue()

send_buffer.put(5)

print send_buffer.get()

while True:
    message = raw_input("Type your message to send then press enter: ")
    sock.sendall(message)

sock.close()

