import socket
import sys
import os

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

server_address = ('localhost', 12345)
#server_address = ('192.168.23.1', 12345)
print >>sys.stderr, 'starting up on %s port %s' % server_address
sock.bind(server_address)

sock.listen(1)

while True:
    print >>sys.stderr, 'waiting for a conneciton'
    (connection, client_address) = sock.accept()
    print >>sys.stderr, 'connection from', client_address
    
    while True:
        try:
            data = connection.recv(1024)
            parameters = data.split(" ")
            
            cmd = 'echo ' + parameters[0] + '=' + parameters[1] + '% > /dev/servoblaster' 
            print(cmd)
            os.system(cmd)
            
        finally:
            connection.close()
                
