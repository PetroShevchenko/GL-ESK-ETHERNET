# GL-ESK-ETHERNET

## Introduction
This repository contains several examples to demonstrate how to work with Ethernet interface of GL Embedded Starter Kit using
STM32CubeMX code generator and built-in TCP/IP stack LwIP

## Requirements
To build the examples you need to install STM32CubeIDE.
If you want to use command line you should install all required utilities firstly(see 1.2).

## License
This repository is provided under Apache license version 2.0.
LICENSE file contains license terms.
All third-party libraries are distributed under their own licenses.

## Build
### 1.1.Build using STM32CubeIDE
Launch STM32CubeIDE then select the tcp_socket_client project and click the menu item 'Project->Build All'

### 1.2.Build using Makefile
Launch system terminal by pressing 'Ctrl+T' and 
change the current directory to ~/GL-ESK-ETHERNET/tcp_socket_client.
Run the command:
**$ make**

Make sure you have installed all required utilies such as:
* GNU Make
* arm-none-eabi-gcc
* binutils
* openocd
* gdb-multiarch

If not than type the following commands:
**$ sudo apt update**

**$ sudo apt install make gcc-arm-none-eabi openocd gdb-multiarch -y**

## Flash \& Debug
### 2.1 Flash \& Debug in STM32CubeIDE
Launch STM32CubeIDE then select the tcp_socket_client project and click the picture with a bug(Debug tcp_socket_client)

### 2.2. Flash \& Debug using Linux terminal

