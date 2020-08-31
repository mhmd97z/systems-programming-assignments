# Kernel Module Programming
 This repo contains several kernel modules developed as assignments in Operating Systems and System Programming course.

## Modules Description
### Interface warm-up
- module1: establishes two interfaces; one can write in the dev interface and the proc interface keeps a log of what is written in the dev interface.
- module2: this module comprises a pair of proc interfaces in a way that what is written in each interface is readable only from the other interface.

NOTE: These two modules are compiled using a simple Makefile but installed with different insmod commands.
### Data structures in Kernel programming 
- module3: This module consists of the following interfaces:
	- dev: dmod (Writing  in this interface and reading from it is possible in 3 ways: stack, fifo, both.)
	- proc: plog, pfifo, pstack (Keeps a log from what is written in dev interface in each possible method)
	- sys: two attributes: sstack, sfifo (This interface sets the writing/reading style in dev interface)
	- ioctl: This interface gives a possibility to reset the written content and also the log.

NOTE: There is also a userspace program to work with ioctl interface

### Work differ 
- Implementation and running time comparison of every possible combination of tasklet, work queue, and shared queue.
 
### Keyboard notifier
 - A module that keeps a log of pressed keyboard buttons in a proc interface named kb_history

## How to run
Module compilation and installation:
```
make
insmod MODULE_NAME.ko
```
Module remove:
```
rmmod MODULE_NAME
```
