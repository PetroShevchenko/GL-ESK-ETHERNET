# GL-ESK-ETHERNET

## Introduction
This repository contains several examples to demonstrate how to work with Ethernet interface of GL Embedded Starter Kit using
STM32CubeMX code generator and built-in TCP/IP stack LwIP

## Requirements
To build the examples you need to install STM32CubeIDE.
If you want to use command line you should first install all required utilities(see 1.2).

## License
This repository is provided under Apache license version 2.0.
LICENSE file contains license terms.
All third-party libraries are distributed under their own licenses.

## Build
### 1.1.Build using STM32CubeIDE
Launch STM32CubeIDE then select the tcp_socket_client project and click the menu item 'Project->Build All'

### 1.2.Build using Makefile
Launch system terminal by pressing 'Ctrl+T' and change current directory to ~/GL-ESK-ETHERNET.
Copy Makefile to the project directory:

**$ cp scripts/Makefile tcp_socket_client/**

Change current directory:

**$ cd tcp_socket_client**

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
Open the terminal by pressing 'Ctrl+T', type the command:

**$ openocd -f interface/stlink-v2-1.cfg -f target/stm32f4x.cfg**

Open a new tab of the terminal and launch gdb-multiarch:

**<p>$ gdb-multiarch build/tcp_socket_client.elf</p>**
**<p>(gdb) target remote localhost:3333</p>**
**<p>(gdb) load</p>**
<p>Loading section .isr_vector, size 0x188 lma 0x8000000</p>
<p>Loading section .text, size 0x1689c lma 0x80001c0</p>
<p>Loading section .rodata, size 0x5804 lma 0x8016a5c</p>
<p>Loading section .ARM, size 0x8 lma 0x801c260</p>
<p>Loading section .init_array, size 0x4 lma 0x801c268</p>
<p>Loading section .fini_array, size 0x4 lma 0x801c26c</p>
<p>Loading section .data, size 0xa4 lma 0x801c270</p>
<p>Start address 0x080169f0, load size 115420</p>
<p>Transfer rate: 20 KB/sec, 7694 bytes/write.</p>

**<p>(gdb)</p>**

Read <a href="https://www.gnu.org/software/gdb/documentation/">GDB documentation</a> to know more about debugging


