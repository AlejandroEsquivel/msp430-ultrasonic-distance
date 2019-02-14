#!/usr/bin/python2.7
import serial # for serial port
import numpy as np # for arrays, numerical processing
from time import sleep,time
import gtk #the gui toolkit we'll use:
# graph plotting library:
from matplotlib.figure import Figure
from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas

#define the serial port. Pick one:
port = "/dev/ttyACM1"  #for Linux
#port = "COM5" #For Windows?
#port = "/dev/tty.uart-XXXX" #For Mac?

#function that gets called when a key is pressed:
def press(event):
    print('press', event.key)
    if event.key == 'q':
        print ('got q!')
        quit_app(None)
    return True
    
def quit_app(event):
    outFile.close()
    ser.close()
    quit()

#start our program proper:
#open the serial port
try:
    ser = serial.Serial(port,2400,timeout = 0.050)
    ser.baudrate=9600
# with timeout=0, read returns immediately, even if no data
except:
    print ("Opening serial port",port,"failed")
    print ("Edit program to point to the correct port.")
    print ("Hit enter to exit")
    raw_input()
    quit()

#create a window to put the plot in
win = gtk.Window()
#connect the destroy signal (clicking the x in the corner)
win.connect("destroy", quit_app)
win.set_default_size(400,300)

yvals = np.zeros(50) #array to hold last 50 measurements
times=np.arange(0,50,1.0) # 50 from 0 to 49.

#create a plot:
fig = Figure()
ax = fig.add_subplot(111,xlabel='Time Step',ylabel='Distance [cm]')
ax.set_ylim(2,200) # set limits of y axis.

canvas = FigureCanvas(fig) #put the plot onto a canvas
win.add(canvas) #put the canvas in the window

# define a callback for when a key is pressed
fig.canvas.mpl_connect('key_press_event',press)

#show the window
win.show_all()
win.set_title("ready to receive data");

line, = ax.plot(times,yvals)
#open a data file for the output
outFile = open("time_and_temp.txt","w")
start_time = time()
ser.flushInput()

while(1): #loop forever
    data = ser.readline() # look for a character from serial port, will wait up to timeout above.
    if (len(data) > 0): #was there a byte to read? should always be true.
        val = float(data)/1000;
        print val;
        yvals = np.roll(yvals,-1) # shift the values in the array
        yvals[49] = val # take the value of the byte
        line.set_ydata(yvals) # draw the line
        fig.canvas.draw() # update the canvas
        win.set_title("Distance")
    while gtk.events_pending():	#makes sure the GUI updates
        gtk.main_iteration()
#    sleep(.05) # don't eat the cpu. This delay limits the data rate to ~ 200 samples/s
