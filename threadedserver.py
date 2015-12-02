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

def calcPulseWidth(adc_reading, adc_min, adc_max):
    if( adc_reading < adc_min):
        return 200
    elif(adc_reading > adc_max):
        return 100

    pulse_width = (10.0 - 5.0*((adc_reading - adc_min)/(adc_max-adc_min)))*20.0

    return pulse_width
                
class CommandIssuer(threading.Thread):
    def __init__(self, queue):
        threading.Thread.__init__(self)
        self.command_queue = queue
        
    def run(self):
        running = 1
        while running:
            if not self.command_queue.empty():
                params = self.command_queue.get().split()

                (thumb_adc_reading,
                 index_adc_reading,
                 mid_adc_reading,
                 ring_adc_reading,
                 pinky_adc_reading) = (float(params[0]),
                                       float(params[1]),
                                       float(params[2]),
                                       float(params[3]),
                                       float(params[4]))


                thumb_pw = calcPulseWidth(thumb_adc_reading, THUMB_MAX, THUMB_MIN)
                index_pw = calcPulseWidth(index_adc_reading, INDEX_MAX, INDEX_MIN)
                mid_pw   = calcPulseWidth(mid_adc_reading, MID_MAX, MID_MIN)
                ring_pw  = calcPulseWidth(ring_adc_reading, RING_MAX, RING_MIN)
                pinky_pw = calcPulseWidth(pinky_adc_reading, PINKY_MAX, PINKY_MIN)
                
                
                thumb_cmd = "echo 0=" + str(thumb_pw) + " > /dev/servoblaster"
                index_cmd = "echo 1=" + str(index_pw) + " > /dev/servoblaster"
                mid_cmd   = "echo 2=" + str(mid_pw) + " > /dev/servoblaster"
                ring_cmd  = "echo 3=" + str(ring_pw) + " > /dev/servoblaster"
                pinky_cmd = "echo 4=" + str(pinky_pw) + " > /dev/servoblaster"

                print thumb_cmd
                print index_cmd
                print mid_cmd
                print ring_cmd
                print pinky_cmd

                ####################################################
                #TAKEN OUT FOR TESTING BUT IT"LL NEED TO BE BACK IN#
                ####################################################
                
                #os.system(cmd)

                ####################################################
                
if __name__ == "__main__": 
    s = Server(sys.argv[1]) 
    s.run()

