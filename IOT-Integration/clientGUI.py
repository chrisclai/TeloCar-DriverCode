import sys
import socket
import tkinter as tk
import random
import math
import time
import datetime
import json
from _thread import *
import threading
import requests

# if len(sys.argv) != 2:
        # print("Usage: python3 clientThread.py <hostID>")
        # sys.exit(1)

# serverIP = sys.argv[1]
serverIP = '45.79.77.67'

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((serverIP, 1234))
print(f"System connected successfully to {serverIP}")

# TKINTER DEFAULT VARIABLES
HEIGHT = 640
WIDTH = 480
REFRESH_RATE = 50
OFFSET=25
VALUE_OFFSET = 60
UNIT_OFFSET = 60
LABEL_BEGIN_X = 190
LABEL_BEGIN_Y = 150

# GLOBAL VARIABLES
global currentKey
currentKey = ''

# INITIALIZATION
# Creation of the program window (root)
root = tk.Tk()
root.resizable(False, False)
main_canv = tk.Canvas(root, height=HEIGHT, width=WIDTH, bg='black', highlightthickness=0)

# KEYBOARD FUNCTIONS
def keyup(k):
    global currentKey
    print('up', k.char)
    currentKey = ''
    s.send(bytes('s', 'utf-8'))

def keydown(k):
    global currentKey
    if currentKey == k.char:
        pass
    else:
        print('down', k.char)
        currentKey = k.char
        sendCommand(currentKey)

def sendCommand(currentKey):
    if currentKey == 'w':
        s.send(bytes('f', 'utf-8'))
    elif currentKey == 'a':
        s.send(bytes('l', 'utf-8'))
    elif currentKey == 's':
        s.send(bytes('b', 'utf-8'))
    elif currentKey == 'd':
        s.send(bytes('c', 'utf-8'))
    else:
        print("Not a valid key!")

# BIND
main_canv.bind("<KeyPress>", keydown)
main_canv.bind("<KeyRelease>", keyup)

main_canv.pack()
main_canv.focus_set()

# CANVAS
control_canv = tk.Canvas(main_canv, width=360, height=900, highlightthickness=0, bg='black')   
control_canv.place(x=60, y=120, anchor='nw') 

# CLASS LABEL
class tkLabelUnit:
    def __init__(self, master=root, str='text', val=0.01, unit='m', list=0, offsetX=0):
        self.label = tk.Label(master, text=str, bg='black', fg='mint cream', font=('garamond',11,), justify='right')
        self.label.place(x=LABEL_BEGIN_X+offsetX, y=LABEL_BEGIN_Y+list*OFFSET, anchor='ne')

        self.value = tk.Label(master, text=val, bg='black', fg='mint cream', font=('garamond',11,), justify='right')
        self.value.place(x=LABEL_BEGIN_X+VALUE_OFFSET+offsetX,y=LABEL_BEGIN_Y+list*OFFSET, anchor='ne')

        self.unit = tk.Label(master, text=unit, bg='black', fg='mint cream', font=('garamond',11,),justify='left')
        self.unit.place(x=LABEL_BEGIN_X+UNIT_OFFSET+offsetX,y=LABEL_BEGIN_Y+list*OFFSET, anchor='nw')

# LABEL
title = tk.Label(main_canv, text="Telo-Operated Car", font=('courier new',24,'bold italic'), justify='center', bg='black', fg='#006699')
title.place(relx=0.5,rely=0.075,anchor='center')

subtitle = tk.Label(main_canv, text="IOT Video Streaming", font=('courier new',20,'bold'), justify='center', bg='black', fg='#0099CC')
subtitle.place(relx=0.5,rely=0.13,anchor='center')

instructions = tk.Label(main_canv, text="Click on the GUI, then use the WASD keys\nto remotely control the car. Sensor\ndata from the car can be seen below.", font=('garamond',16,'italic'), justify='center', bg='black', fg='#FFFF81')
instructions.place(relx=0.5,rely=0.225,anchor='center')

# BUTTONS
exitButton = tk.Button(main_canv, text="EXIT", font=('courier new',18,'bold'), command=exit, justify='center', padx=40, pady=10, bg='black', fg='red')
exitButton.place(relx=0.5,rely=0.9,anchor='center')

# UPDATE FUNCTION
# Will assign random numbers to values whenever called.
def updateData():
    # Recursive function to update values. 

    root.after(REFRESH_RATE, updateData)

# UPDATE / REFRESH
# This is start calling the update function which is recursive.
# The recursion is essentially the update / represh.
root.after(REFRESH_RATE, updateData)

# END
root.mainloop()