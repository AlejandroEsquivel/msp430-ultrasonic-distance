# MSP430 Ultrasonic Distance Fimware

The MSP430 (MSP430g2553) was used to create an ultrasonic (HC-SR04) distance meter, the distance measurements are sent via USB-Serial interface to the host computer using UART. 

# Connections

There were two different approaches of implementing the distance sensor as they have different connections, below indicates which branch contains the code for the respective connections.

## (Default) Capture Mode Approach [master branch]

The `P2.1` MSP430 pin was connected to the HC-SR04's TRIGGER pin.

The `P1.1` pin was connected in series with a 1kOhm resistor and to the HC-SR04's ECHO pin.

## Manual rising/falling edge detection & TAR counter [no-capture branch]

The `P2.1` MSP430 pin was connected to the HC-SR04's TRIGGER pin.

The `P2.0` pin was connected in series with a 1kOhm resistor and to the HC-SR04's ECHO pin.

# Build & Install firmware into MSP430

```
make
mspdebug rf2500
prog ultrasonic.elf
^D
```
Where `^D` is [CTRL-D].

The MSP430 should start sending distance measurements to the host computer.

If you don't have a msp430 C compiler, see [Installation of Dependencies](DEPENDENCIES.md).

# Plot Distance

See a real time plot of the distance measurements being transmitted via USB.
```
python python-serial-plot.py
``

Note: some dependencies required, see [Installation of Dependencies](DEPENDENCIES.md)