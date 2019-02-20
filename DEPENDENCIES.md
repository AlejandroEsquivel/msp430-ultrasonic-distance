# Installation of Dependencies

## Mac

Important: If you are using Mac, and you want to plot the measurements with Python, you may need to edit the `port` variable in either `python-serial-plot.py` to `port = "/dev/tty.uart-XXXX"`. 

To find out which port it is, you can type the following in terminal
```
ls /dev/tty.uart-
```
And press [TAB] to autocomplete. The result of the autocomplete will need to go into the `port` variable.



### MacPorts
We require macports to install packages on Mac (https://guide.macports.org/)

### MSP430 Assembler & Driver

Install mspdebug
```
sudo port install mspdebug
```

Download http://www.mikekohn.net/downloads/naken430asm/naken430asm-2011-10-30-macosx10.6-x86_64.tar.gz

Extract package
```
tar -zxvf ~/Downloads/naken430asm-2011-10-30-macosx10.6-x86_64.tar.gz
```
Build & Install it
```
cd naken430asm-2011-10-30
./configure
make
sudo make install
```

Download MSP430 Driver at https://github.com/energia/Energia/raw/gh-pages/files/MSP430LPCDC-1.0.3b.zip
and install.

### MSP430 C Compiler

Install the following (you may need to sign up with TI first). Install at `$HOME/ti` or anywhere that is consistent with the `Makefile` on this repository and the PATH (below)
http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSPGCC/4_02_00_00/index_FDS.html

After installing, add to your PATH
```
export PATH="$PATH:$HOME/ti/bin"
```
### Python Plotting Data (optional)

We require python 2.7 (https://www.python.org/downloads/mac-osx/)

```
sudo port install xorg-server py27-serial py27-pygtk py27-numpy
sudo port install py27-matplotlib +gtk2
```
