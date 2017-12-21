from Tkinter import *
import serial
#import time
from struct import unpack, pack
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
        self.S_button.grid(row=1, column=0, columnspan=3, sticky=W)

        self.N_button = Checkbutton(master, text="Send input (N)", command=lambda:self.sendchar('N'))
        self.N_button.grid(row=1, column=3, columnspan=3, sticky=W)

        self.E_button = Checkbutton(master, text="Send input ref (E)", command=lambda: self.sendchar('E'))
        self.E_button.grid(row=1, column=6, columnspan=3, sticky=W)

        self.O_button = Checkbutton(master, text="Send output (O)", command=lambda:self.sendchar('O'))
        self.O_button.grid(row=1, column=9, columnspan=3, sticky=W)

        self.V_button = Checkbutton(master, text="Verbose mode (V)", command=lambda:self.sendchar('V'))
        self.V_button.grid(row=1, column=12, columnspan=3, sticky=W)

        self.P_label_s = Label(master, text="PIDamp:")
        self.P_label_s.grid(row=2, column=0, sticky=E)

        self.P_entry_s = Entry(master, width=6)
        self.P_entry_s.grid(row=2, column=1, columnspan=2, sticky=W)

        self.I_label_s = Label(master, text="HH:")
        self.I_label_s.grid(row=2, column=3, sticky=E)

        self.I_entry_s = Entry(master, width=6)
        self.I_entry_s.grid(row=2, column=4, columnspan=2, sticky=W)
        #
        # self.D_label_s = Label(master, text="D:")
        # self.D_label_s.grid(row=2, column=6, sticky=E)
        #
        # self.D_entry_s = Entry(master, width=6)
        # self.D_entry_s.grid(row=2, column=7, columnspan=2, sticky=W)

        self.Send_button = Button(master, text="Send PIDamp and HillHeight", command=self.sendPIDamp)
        self.Send_button.grid(row=2, column=6, columnspan=3, sticky=W)

        self.P_label_b = Label(master, text="P:")
        self.P_label_b.grid(row=2, column=12, sticky=E)

        self.P_entry_b = Entry(master, width=6)
        self.P_entry_b.grid(row=2, column=13, columnspan=2, sticky=W)

        self.I_label_b = Label(master, text="I:")
        self.I_label_b.grid(row=2, column=15, sticky=E)

        self.I_entry_b = Entry(master, width=6)
        self.I_entry_b.grid(row=2, column=16, columnspan=2, sticky=W)

        self.D_label_b = Label(master, text="D:")
        self.D_label_b.grid(row=2, column=18, sticky=E)

        self.D_entry_b = Entry(master, width=6)
        self.D_entry_b.grid(row=2, column=19, columnspan=2, sticky=W)

        self.Send_button_b = Button(master, text="Send PID", command=self.sendall_b)
        self.Send_button_b.grid(row=2, column=21, sticky=W)

        self.SP_label = Label(master, text="Setpoint:")
        self.SP_label.grid(row=3, column=0, columnspan=2, sticky=E)

        self.SP_entry = Entry(master, width=6)
        self.SP_entry.grid(row=3, column=2, columnspan=2, sticky=W)

        self.SP_Send_button = Button(master, text="Send SP", command=self.sendSP)
        self.SP_Send_button.grid(row=3, column=3, columnspan=3, sticky=E)

        self.SP_Get_button = Button(master, text="Get SP", command=self.getSP)
        self.SP_Get_button.grid(row=3, column=6, columnspan=3, sticky=W)

        self.Scan_entry = Entry(master, width=6)
        self.Scan_entry.grid(row=3, column=7, columnspan=3, sticky=E)

        self.Scan_button = Button(master, text="Scan", command=self.sendScan)
        self.Scan_button.grid(row=3, column=10, columnspan=3, sticky=W)

        self.ScanStop_button = Button(master, text="Stop scan", command=self.stopScan)
        self.ScanStop_button.grid(row=3, column=12, columnspan=3, sticky=W)

        self.close_button = Button(master, text="Close", command=self.quit)
        self.close_button.grid(row=3, column=21, columnspan=3, sticky=E)

        self.fig = plt.figure(1)
        plt.ion()
        plt.plot(t_cur, V_cur)

        self.canvas = FigureCanvasTkAgg(self.fig, master=root)
        self.plot_widget = self.canvas.get_tk_widget()

        self.plot_widget.grid(row=0, column=0, columnspan=21)

    def sendPIDamp(self):

        try:
            self.send_ff("P", "U", float(self.P_entry_s.get()), 2)
            self.send_ff("I", "U", float(self.I_entry_s.get()), 0)
            print("New PIDamp and HillHeight value sent to teensy")
            return True
        except ValueError:
            print("You can only write ints in the textboxes. Write only those and try again")
            return False

    def sendall_b(self):

        try:
            self.send_ff("P", "B", float(self.P_entry_b.get()), 8)
            self.send_ff("I", "B", float(self.I_entry_b.get()), 8)
            self.send_ff("D", "B", float(self.D_entry_b.get()), 8)
            print("New big PID values sent to Teensy")
            return True
        except ValueError:
            print("You can only write ints in the textboxes. Write only those and try again")
            return False

    def sendSP(self):

        try:
            num = int(self.SP_entry.get())
            p1 = (num // 256 ** 1) % 256
            p2 = (num // 256 ** 0) % 256
            ser.write(bytearray(["Z", "S", p1, p2]))
            print("New Setpoint sent to Teensy")
            return True
        except ValueError:
            print("You can only write an int in the textbox. Write only that and try again")
            return False

    def getSP(self):

        ser.write(bytearray(["Z","G"]))
        print("Setpoint requested from Teensy")

    def sendScan(self):

        self.send_ii("C", "S", int(self.Scan_entry.get()))

    def stopScan(self):

        ser.write(bytearray(["C", "T"]))

    def send_f(self, char, num, exp):

        fnum = num*10**-exp
        fasbytearray = pack('>f', fnum)
        ser.write(char + fasbytearray)

    def send_ff(self, char1, char2, num, exp):

        fnum = num*10**-exp
        fasbytearray = pack('>f', fnum)
        out = bytearray([char1, char2]) + fasbytearray
        ser.write(out)

    def send_i(self, char, num):

        p1 = (num // 256 ** 1) % 256
        p2 = (num // 256 ** 0) % 256
        ser.write(bytearray([char, p1,p2]))

    def send_ii(self, char, char2, num):
        p1 = (num // 256 ** 1) % 256
        p2 = (num // 256 ** 0) % 256
        ser.write(bytearray([char, char2, p1, p2]))

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
try:
    ser = serial.Serial(
        port='/dev/cu.usbmodem3175531',  # when port is given here it is automatically opened
        timeout=0.001,  # semi random value, think more about this
    )
except serial.serialutil.SerialException:
    print("The serial port could not be opened. Check that the teensy is connected and not busy, close the application and try again.")

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

            if outt[1] == 'P' or outt[1] == 'I' or outt[1] == 'D':
                try:
                    numret = unpack('>f', outt[2] + outt[3] + outt[4] + outt[5])[0]
                    print("Teensy says: " + outt[0] + outt[1] + str(numret))
                    del outt[0:6]
                except IndexError:
                    print('Probably lost a message')
                    outt = []

            elif outt[1] == 'O' or outt[1] == 'S' or outt[1] == 'N' or outt[1] == 'E' or outt[1] == 'C':
                print("Teensy says: " + outt[0] + outt[1])
                del outt[0:2]

            elif outt[1] == 'Z':
                try:
                    numret = unpack('>H', outt[2] + outt[3])[0]
                    print("Teensy says: " + outt[0] + outt[1] + str(numret))
                    del outt[0:4]
                except IndexError:
                    print("Probably lost a data point")
                    outt = []

        elif outt[0] == 'T':

            if outt[1] == 'U':
                try:
                    global m
                    numret_t = unpack('>H', outt[2] + outt[3])[0]
                    numret_V = unpack('>H', outt[4] + outt[5])[0]
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

            elif outt[1] == 'Z':
                try:
                    numret = unpack('>H', outt[2] + outt[3])[0]
                    my_gui.SP_entry.delete(0,END)
                    my_gui.SP_entry.insert(0, str(numret))
                    print("Teensy says: " + outt[0] + outt[1] + str(numret))
                    del outt[0:4]
                except IndexError:
                    print("Probably lost a data point")
                    outt = []

            elif outt[1] == 'V':
                numret = unpack('>H', outt[2] + outt[3])[0]
                print("Teensy says: " + outt[0] + outt[1] + str(numret))
                del outt[0:4]

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