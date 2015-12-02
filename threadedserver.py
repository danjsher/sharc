import select 
import socket 
import sys 
import threading 
import Queue
import os

class Server: 
    def __init__(self, host): 
        self.host = host 
        self.port = 12345 
        self.backlog = 5 
        self.size = 1024 
        self.server = None 
        self.threads = [] 
        self.command_queue = Queue.Queue(5)
        
    def open_socket(self): 
        try: 
            self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
            self.server.bind((self.host,self.port)) 
            self.server.listen(5) 
        except socket.error, (value,message): 
            if self.server: 
                self.server.close() 
            print "Could not open socket: " + message 
            sys.exit(1) 

    def run(self): 
        self.open_socket() 
        input = [self.server,sys.stdin]
        cmdIssuer = CommandIssuer(self.command_queue)
        cmdIssuer.start()
        self.threads.append(cmdIssuer)
        running = 1 
        while running: 
            inputready,outputready,exceptready = select.select(input,[],[]) 

            for s in inputready: 

                if s == self.server: 
                    # handle the server socket 
                    c = Client(self.server.accept(), self.command_queue) 
                    c.start() 
                    self.threads.append(c) 

                elif s == sys.stdin: 
                    # handle standard input 
                    junk = sys.stdin.readline() 
                    running = 0 

        # close all threads 

        self.server.close() 
        for c in self.threads: 
            c.join() 

class Client(threading.Thread): 
    def __init__(self,(client,address), queue): 
        threading.Thread.__init__(self) 
        self.client = client 
        self.address = address 
        self.size = 1024 
        self.command_queue = queue
        
    def run(self): 
        running = 1 
        while running: 
            data = self.client.recv(self.size)
            
            if data:
                params = data.split()
                self.command_queue.put(data)
                self.client.send(data) 
            else: 
                self.client.close() 
                running = 0 

class CommandIssuer(threading.Thread):
    def __init__(self, queue):
        threading.Thread.__init__(self)
        self.command_queue = queue
        
    def run(self):
        running = 1
        while running:
            if not self.command_queue.empty():
                params = self.command_queue.get().split()
                num = float(params[4])
                pinky_pulse_width = 100
                if ( num < 735 ):
                    pinky_pulse_width = 100
                else:
                    pinky_pulse_width = (10.0 - 5.0*((num-735.0)/288.0))*20.0
                
                cmd = "echo 0=" + str(pinky_pulse_width) + " > /dev/servoblaster"
                print cmd
                os.system(cmd)
                
if __name__ == "__main__": 
    s = Server(sys.argv[1]) 
    s.run()

