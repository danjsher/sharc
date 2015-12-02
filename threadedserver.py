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

        self.client.close()
        print 'Connection closed.'

def calcPulseWidth(adc_reading, adc_min, adc_max, pwm_min, pwm_max):
    if( adc_reading < adc_min):
        return 20*pwm_max
    elif(adc_reading > adc_max):
        return 20*(pwm_max-pwm_min)

    pulse_width = (pwm_max - pwm_min*((adc_reading - adc_min)/(adc_max-adc_min)))*20.0

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

                # Calculate pulse widths based on adc values from sleeve

                thumb_pw = calcPulseWidth(thumb_adc_reading, 725, 1023, 5.0, 10.0)
                index_pw = calcPulseWidth(index_adc_reading, 745, 930, 7.0, 10.5)
                mid_pw   = calcPulseWidth(mid_adc_reading, 730, 1023, 7.0, 11.5)
                ring_pw  = calcPulseWidth(ring_adc_reading, 680, 1023, 6.4, 10.4)
                pinky_pw = calcPulseWidth(pinky_adc_reading, 735, 1023, 5.0, 10.0)

                # construct echo commands
                
                thumb_cmd = "echo 0=" + str(thumb_pw) + " > /dev/servoblaster"
                index_cmd = "echo 1=" + str(index_pw) + " > /dev/servoblaster"
                mid_cmd   = "echo 2=" + str(mid_pw) + " > /dev/servoblaster"
                ring_cmd  = "echo 3=" + str(ring_pw) + " > /dev/servoblaster"
                pinky_cmd = "echo 4=" + str(pinky_pw) + " > /dev/servoblaster"

                # print for debug and monitoring
                
                print thumb_cmd
                print index_cmd
                print mid_cmd
                print ring_cmd
                print pinky_cmd

                # Execute bash commands to write to servoblaster kernel 
                os.system(pinky_cmd)
                os.system(ring_cmd)
                os.system(mid_cmd)
                os.system(index_cmd)
                os.system(thumb_cmd)
                
                
if __name__ == "__main__": 
    s = Server(sys.argv[1]) 
    s.run()

