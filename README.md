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
The library is for the STM32G0B1RE Nucleo Board and uses the STM32 HAL drivers. It uses one of the two CAN instances in the Nucleo Board to simulate a CAN bus analyzer. The user can use the other CAN instance to transmit data and view it on the computer's serial port terminal. In addition, the user can also transmit data from the CUBA's CAN instance through the computer's serial port terminal using an AT command.

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
*The clock frequency must be 48Mhz
*The CAN speed should be 100kbps
*The CAN instance for CUBA library is FDCAN2
*The CUBA library uses CAN interrupt signal IT1 TIM17_FDCAN_IT1_IRQn
*The user should configure in the CUBA MSP the GPIO AF PINS for FDCAN2

//Incomplete

**[Back to top](#table-of-contents)**

## Authors
* **[Luis Gonzalez](https://github.com/LuisGoC)**
