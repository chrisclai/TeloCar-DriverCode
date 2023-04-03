import sys
import serial
import time
import socket
import cv2

from _thread import *
import threading

DATA_AMOUNT = 2

global control_list
control_list = [0, 0] # Speed | Angle

def control(connIn, connOut, addr): 
    print(f"[CONTROL] {addr} Successfully connected to control thread!")
    global controllist
    while True:
        try:
            msg = connIn.recv(16).decode('utf-8')
            print(msg)
            if not msg:
                pass
            else:
                if msg == "f":
                    connOut.write('f'.encode('utf-8'))
                    control_list[0] += 1
                elif msg == "b":
                    connOut.write('b'.encode('utf-8'))
                    control_list[0] -= 1
                elif msg == "l":
                    connOut.write('l'.encode('utf-8'))
                    control_list[1] -= 10
                elif msg == "c":
                    connOut.write('c'.encode('utf-8'))
                    control_list[1] += 10
                connOut.flush()
        except: 
            print("Could not send command into arduino!!!")

def maindata(connOut, addr): 
    print(f"[DATA] {addr} Successfully connected to data input thread!")
    global control_list
    time.sleep(5)
    while True:
        # create string again with all of the list elements and send them out to the GUI
        dataSend = ""
        for x in control_list:
            dataSend += str(x)  + " "
        print(dataSend)
        connOut.send(bytes(dataSend, 'utf-8'))
        time.sleep(0.1)

def Main():
    # Uncomment these lines of code if you would like to specify an address
    # if len(sys.argv) != 2:
        # print("Usage: python3 clientThread.py")
        # sys.exit(1)
    # serverIP = sys.argv[1]
    
    # Specific IP Address of server
    serverIP = "45.79.77.67"

    # Define Arduino Pro Micro
    serMicro = serial.Serial("/dev/ttyACM0", 115200) # Control Data

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((serverIP, 1234))

    print(f"Connection from {serverIP} has been established!")

    # [THREAD] Control Thread to recieve inputs from the GUI and send into Arduino
    thread_tomicro = threading.Thread(target = control, args=(s, serMicro, serverIP))
    thread_tomicro.start()
    
    # [THREAD] Recieves data from system, organizes data from other threads, and sends it off to the GUI
    thread_toUI = threading.Thread(target = maindata, args=(s, serverIP))
    thread_toUI.start()
    
if __name__ == '__main__': 
    Main() 