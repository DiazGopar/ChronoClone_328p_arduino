import os, pty, serial

master, slave = pty.openpty()
s_name = os.ttyname(slave)

ser = serial.Serial(s_name)

# To Write to the device
ser.write('Your text'.encode())

# To read from the device
os.read(master,1000)

while True:
    os.read(master,1000)
