import socket
import sys
import os

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

server_address = ('192.168.1.19', 12345)
print >>sys.stderr, 'starting up on %s port %s' % server_address
sock.bind(server_address)

sock.listen(1)

while True:
    print >>sys.stderr, 'waiting for a conneciton'
    (connection, client_address) = sock.accept()

    try:
        print >>sys.stderr, 'connection from', client_address

        data = connection.recv(1024)
        parameters = data.split(" ")
        
        cmd = 'echo ' + parameters[0] + '=' + parameters[1] + '% > /dev/servoblaster' 
        print(cmd)
        os.system(cmd)
            
    finally:
        connection.close()
                
