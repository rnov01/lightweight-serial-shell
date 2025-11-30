# AVR USART Drivers

Semi hardware-abstracted USART drivers for AVR devices.

---
##Overview

A modular USART driver that is hardware agnostic within the AVR chip family. The files usart.h and usart.cpp define 
    a 'USART' class used to create hardware-backed USART objects. Member functions include basic USART capabilities like
    read and write. 

    The file pin.h defines a 'pin' struct for hardware-abstracted GPIO access. The 'PIN_LIST' macro expands all
        ATMEGA2560 pins automatically. This list can be adjusted for other AVR chips as needed.

##Compilation Info

    This software was compiled using:
    - **avr-gcc**
    - **avrdude**
    - **GNU make**

##Documentation

    More information can be found [here](https://rnov01.github.io/avr-hal-usart/index.html)
---


