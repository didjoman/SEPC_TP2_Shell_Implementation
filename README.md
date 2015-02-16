Ensimag-shell
=============

These files implement a mini-shell.
This project was an assignment for the course of "Operating System & Concurrent computing" at Ensimag ("Système d'Exploitation et Programmation Concurrente" in french).

All the files are covered by the licence GPLV3+

Introduction
----------

The goal of this assignment was to implement a mini-shell capable of launching commands.
Different features were implemented : 
- Launching of a command in foreground
- Launching of a command in background (with &)
- Listing of all the process launched (implementation of the command jobs).
- Multiple Pipes ( | in shell)
- I/O Redirections ( > in shell)
- Calculation time (given at the termination of a process).

How to build the project
----------

> `cd SEPC_TP2_Shell_Implementation`

> `cd build`

> `cmake ..`

> `make`

Usage
----------
We can test the mini-shell (after having compilled it) by launching the following commands : 


> `cd SEPC_TP2_Shell_Implementation`

> `cd build`

> `./ensishell`

Tests
----------
Unit-tests (Google tests) have been provided by the professors to check the validity of the shell.
In order to launch it : 

> `cd SEPC_TP2_Shell_Implementation`

> `cd build`

> `make test`

> `make check`



