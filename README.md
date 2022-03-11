# CUBA (CAN-UART Bridge Analyzer)
CUBA is a library that creates a bridge between two communication peripherals of the microcontroller, CAN, and UART. This library allows the user to analyze the CAN bus data on a computer terminal like puTTY or other serial port terminals.
___
## Table of Contents

1. [About the Project](#about-the-project)
1. [Getting Started](#getting-started)
    1. [Dependencies](#dependencies)
    1. [Getting the Source](#getting-the-source)
    1. [Installation](#installation)
    1. [Building](#building)
    1. [Usage](#usage)
1. [Further Reading](#further-reading)
1. [Authors](#authors)
___
## About the Project
The library is for the STM32G0B1RE Nucleo Board and uses the STM32 HAL drivers. It uses one of the two CAN instances in the Nucleo Board, and one UART to simulate a CAN bus analyzer. The user can use the other CAN instance to transmit data and view it on the computer's serial port terminal. In addition, the user can also transmit data from the CUBA's CAN instance through the computer's serial port terminal using an AT command.

Command:
``
ATSMCAN=< ID msg >,< 8 bytes of data in hex >\r
``

Example:
``
ATSMCAN=1FF,484F4C41484F4C41
``

The example transmits through the CUBA's CAN instance the message "HOLAHOLA" with the message identifier 0x1FF.

**[Back to top](#table-of-contents)**

## Getting started
These instructions will get you a copy of the library project up and running on your local machine for development and testing purposes.

### Dependencies
In order to build, debug or use this project you'll need the following dependencies to be install in your computer.

- GCC compiler.
- GDB debugger.
- JLink.
- Make
- Git.
- VS Code (or another code editor).
- PuTTY (or another serial port terminal).

**[Back to top](#table-of-contents)**

### Getting the Source
This project is [hosted on GitHub](https://github.com/LuisGoC/CUBAlib). You can clone this project directly using this command:

```
git clone https://github.com/LuisGoC/CUBAlib.git
```
**[Back to top](#table-of-contents)**

### Installation
For download and install Visual Studio Code for Linux (Manjaro) you can click [here](https://snapcraft.io/install/code/manjaro).
You can also install GCC, the GNU compiler in the official [website](https://gcc.gnu.org/). Or using the following Linux command:
```
pacman -Sy gcc
```
For Git installation on Linux:
```
sudo pacman -S git
```
Putty Serial Port Terminal on Linux:
```
sudo pacman -S putty
```
Make installation on Linux:
```
sudo pacman -S make
```

**[Back to top](#table-of-contents)**

### Building
A makefile is included in this project to automate the compilation process and make easier some other features. You can use the following make commands through the VS Code terminal.
To build the project:
```
make all
```
To delete the built files:
```
make clean
```
To connect the interface and target micro to the debug host:
```
make open
```
To debug with GDB:
```
make debug
``` 
**[Back to top](#table-of-contents)**

### Usage
There are specific steps to follow to use the library correctly:
- The user must configure the clock frequency at 48MHz.
- The CAN bus speed is 100 Kbps (for easy hardware configuration purposes).
- The CUBA library uses FDCAN2 instance, so the user should use the FDCAN1 instance.
- The CUBA library sets MSP configuration for FDCAN2 as PB0(RX) and PB1(TX).
- The CUBA library uses Rx Handle FIFO1 to store received messages, so the user should use Rx Handle FIFO0.
- The CUBA library uses IRQn 22 (TIM17_FDCAN_IT1_IRQn), so the user should use IRQn 21 (TIM16_FDCAN_IT0_IRQn) in case of need interruptions in the FDCAN1 instance.
- The user must initialize the CUBA library using a CUBA_HandleTypeDef structure type variable. 

#### Hardware Scheme
Single wire CAN bus without transceivers connection.

![Schematic Diagram](https://github.com/LuisGoC/CUBAlib/blob/main/CUBA_draw.png)

**[Back to top](#table-of-contents)**

## Authors
* **[Luis Gonzalez](https://github.com/LuisGoC)**
