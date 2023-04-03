import sys
import socket
import time

from _thread import *
import threading  

# [FUNCTION] read_write_async() is an asynchronous data handler.
# The function is threaded (ran in parallel) with the center server processor
# in order to transmit asynchronous (user input) data from the user client
# to the vehicle client.
# PARAMETERS: connIn (socket object of user client), connOut (socket object of vehicle client)
# addr (Address of user client connecting to thread)
# RETURNS: void
def read_write_async(connIn, connOut, addr): 
    print(f"[NEW CONNECTION] {addr} connected to r-w-a thread.")
    while True:
        try:
            msg = connIn.recv(16)
            connOut.send(msg)
        except: 
            print(f"Packet receive attempt to {addr} failed. Closing connection.")
            connIn.close()
            break

# [FUNCTION] read_write_sync() is a sychrnous data handler.
# The funnction is threaded with the center server processor in order to transmit
# data packets gathered from the vehicle in a sychronous manner. The data packets
# are transmitted from the vehicle client -> the user client.
# PARAMETERS: connIn (socket object of vehicle client), connOut (socket object of user client)
# addr (Address of user client connecting to thread)
# RETURNS: void
def read_write_sync(connIn, connOut, addr):
    print(f"[NEW CONNECTION] {addr} connected to r-w-s thread.")
    while True:
        try:
            msg = connIn.recv(4096)
            connOut.send(msg)
        except:
            print(f"Packet send attempt to {addr} failed. Closing connection.")
            connOut.close()
            break

def main(): 

    # Define host information (IP Address of TeloCar-IOT Server)
    host = "45.79.77.67"

    print(f"host found on ip: {host}")

    # Server Setup, Create server on host + port, bind to port
    port = 1234
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
    s.bind((host, port)) 
    print("socket binded to port", port) 

    # Listen on the server for initial client, 10 second timeout
    s.listen(5)
    s.settimeout(10) 
    print("socket is listening") 

    # dictionary of open sockets
    sockdict = {}

    # Initialize server socket with Jetson Nano/RPi client object. Ensure that
    # the Jetson Nano/RPi is the first client to enter the server, otherwise
    # risk breaking the server.
    try:
        # Create a specialized socket just for the RPi on boot
        rpiclientsocket, rpiaddress = s.accept()
        sockdict[rpiclientsocket] = rpiaddress
        print(f"Connection from {rpiaddress} has been established! [THIS IS THE JETSON NANO]")
        time.sleep(3)

        while True:
            msg = rpiclientsocket.recv(4096)
            if not msg:
                rpiclientsocket.close()
                print("Jetson disconnected. Closing socket.")
                print("Unable to continue process. Terminating script.")
                break
            else:
                # print(msg.decode('utf-8')) # test to see if data comes through
                pass
            try:
                conn, addr = s.accept()       
                print(f"Connection from {addr} has been established!")
                sockdict[conn] = addr
                print(f"Socket dictionary: {sockdict}")
                thread_sync = threading.Thread(target = read_write_sync, args=(rpiclientsocket, conn, addr))
                thread_sync.start()
                thread_async = threading.Thread(target = read_write_async, args=(conn, rpiclientsocket, addr))
                thread_async.start()
            except:
                pass   
    except:
        print("Jetson not found. Please run program with connected device.")            

if __name__ == '__main__': 
    main() 