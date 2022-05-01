# CMPSCI-4760 P5

## Project Goal:

* Designing and implementing a resource management module for our Operating System
Simulator.
* Using the deadlock detection and recovery strategy to manage resources.

## How to run
Commands to run project from hoare server:
```
$ make
$ ./oss -v 
```
where all other parameters are default, number of process 18, number of resource 10, termination time 5.
OR:
```
$ make
$ ./oss -v -t [termination time] -n [number of procrss] -r [number of resource]
```

## Git

https://github.com/SanzidaH/CMPSCI4760-p5.git

## Task Summary

* OSS will fork multiple children or user processes at random times.
* OSS will allocate shared memory for system data structures, including resource descriptors for each resource.
* User processes will ask/release resource or terminate process at random times.
* OSS will grant resources when asked if it can find sufficient quantity to allocate or it put the process in waiting queue.
* OSS will run deadlock detection algortihm periodically. 
* OSS will try recover deadlock situtation by killing if algorithm detects deadlock.

## Experience

* I used shared memory for system clock and other data stuctures. 
* I spent a lot of time trying to solve "Resource unavailable" issue and stuck there.
