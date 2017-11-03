from Tkinter import *
import serial
#import time
from struct import unpack
#import numpy as np
import matplotlib
matplotlib.use("TkAgg")
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib import pyplot as plt
import csv

savetofile = 1

t_cur = [0] * 500
V_cur = [0] * 500
m = 0

class PIDGUI:
    def __init__(self, master):
        self.master = master
        master.title("Lock Em Up Ultra Pro Version")

        self.S_button = Checkbutton(master, text="Run/Stop (S)", command=lambda:self.sendchar('S'))
        self.S_button.grid(row=1, column=0, sticky=W)

        self.N_button = Checkbutton(master, text="Send input (N)", command=lambda:self.sendchar('N'))
        self.N_button.grid(row=1, column=1, sticky=W)

        self.O_button = Checkbutton(master, text="Send output (O)", command=lambda:self.sendchar('O'))
        self.O_button.grid(row=1, column=2, sticky=W)

        self.V_button = Checkbutton(master, text="Verbose mode (V)", command=lambda:self.sendchar('V'))
        self.V_button.grid(row=1, column=3, sticky=W)

        self.P_label = Label(master, text="P:")
        self.P_label.grid(row=1, column=4, sticky=W)

        self.P_entry = Entry(master, width=6)
        self.P_entry.grid(row=1, column=5, sticky=W)

        self.I_label = Label(master, text="I:")
        self.I_label.grid(row=1, column=6, sticky=W)

        self.I_entry = Entry(master, width=6)
        self.I_entry.grid(row=1, column=7, sticky=W)

        self.D_label = Label(master, text="D:")
        self.D_label.grid(row=1, column=8, sticky=W)

        self.D_entry = Entry(master, width=6)
        self.D_entry.grid(row=1, column=9, sticky=W)

        self.Send_button = Button(master, text="Send", command=self.sendall)
        self.Send_button.grid(row=1, column=10, sticky=W)

        self.close_button = Button(master, text="Close", command=self.quit)
        self.close_button.grid(row=1, column=11, sticky=W)

        self.fig = plt.figure(1)
        plt.ion()
        plt.plot(t_cur, V_cur)

        self.canvas = FigureCanvasTkAgg(self.fig, master=root)
        self.plot_widget = self.canvas.get_tk_widget()

        self.plot_widget.grid(row=0, column=0, columnspan=10)

    def sendall(self):

        try:
            self.send("P", int(self.P_entry.get()))
            self.send("I", int(self.I_entry.get()))
            self.send("D", int(self.D_entry.get()))
            print("New PID values sent to Teensy")
            return True
        except ValueError:
            print("You can only write integers under 30K in the textboxes. Write only that and try again")
            return False

    def send(self, char, num):

        p1 = (num // 256 ** 1) % 256
        p2 = (num // 256 ** 0) % 256
        ser.write(bytearray([char, p1, p2]))

    def sendchar(self, char):
        ser.write(char)

    def update(self):
        plt.clf()
        plt.xlim([min(t_cur), max(t_cur) + 0.1])
        plt.ylim([min(V_cur), max(V_cur) + 0.1])
        plt.plot(t_cur, V_cur)
        self.fig.canvas.draw()

    def quit(self):

        root.after_cancel(after)
        self.master.quit()

ser = serial.Serial(
    port='/dev/cu.usbmodem3175531',  # when port is given here it is automatically opened
    timeout=0.001,  # semi random value, think more about this
)

t_list = []
V_list = []
updatecount = 0

def read():
    outt = []
    while ser.inWaiting() > 0:
        abytes = ser.inWaiting()
        outt = ser.read(abytes)
        outt = list(outt)

    while len(outt) > 1:

        if outt[0] == 'R':

            if outt[1] == 'P' or outt[1] == 'I' or outt[1] == 'D' or outt[1] == 'V':
                try:
                    numret = unpack('H', outt[3] + outt[2])[0]
                    print("Teensy says: " + outt[0] + outt[1] + str(numret))
                    del outt[0:4]
                except IndexError:
                    print('Probably lost a message')
                    outt = []

            elif outt[1] == 'O' or outt[1] == 'S' or outt[1] == 'N': #it seems like it doesn't print N the first time. bleh
                print("Teensy says: " + outt[0] + outt[1])
                del outt[0:2]

        elif outt[0] == 'T':

            if outt[1] == 'U':
                try:
                    global m
                    numret_t = unpack('H', outt[3] + outt[2])[0]
                    numret_V = unpack('H', outt[5] + outt[4])[0]
                    t_list.append(numret_t + 65536*m)
                    V_list.append(numret_V)
                    t_cur.append(numret_t + 65536*m)
                    V_cur.append(numret_V)
                    if len(t_cur) > 500:
                        del t_cur[0]
                        del V_cur[0]
                    if t_cur[-1] + 40000 < t_cur[-2]:
                        t_cur[-1] += 65536
                        t_list[-1] += 65536
                        m += 1
                    del outt[0:6]
                except IndexError:
                    print("Probably lost a data point")
                    outt = []

    global updatecount

    updatecount += 1

    if updatecount == 10:
        my_gui.update()
        updatecount = 0

    global after

    after = root.after(1, read) # reschedule event after 1 ms


root = Tk()
my_gui = PIDGUI(root)

after = root.after(1, read)
root.mainloop()

if savetofile:

    with open("t.csv","w") as outfile :
        writer = csv.writer(outfile, lineterminator='\n')
        for val in t_list:
            writer.writerow([val])

    with open("V.csv","w") as outfile:
        writer = csv.writer(outfile, lineterminator='\n')
        for val in V_list:
            writer.writerow([val])