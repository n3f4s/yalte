# yalte (Yet Another Linux Terminal Emulator)

## Intro

Yalte is a toy emulator. The purpose of this project is to learn how emulator works.
The end result should be a light terminal emulator with the the minimal required features and maybe a little more
(closer to [st](http://st.suckless.org/) than to xterm).

## Goals

**Ultimate optimistic best case goal:**
    to be a fully functionnal terminal on par with xterm and the like (but lighter).

**Realistic goal:**
    to have a terminal with most of the usual features and that work in the best case scenario.

## Features / TODO list

- [ ] Clean code and TODO list (regroup stray TODO, ...)
- [ ] Handle special key (WIP)
- [ ] Signal management 
    - [ ] freeze/kill the command instead of the shell
    - [ ] handle child process
- [ ] Make sure that the terminal does not consume too much processing power
- [ ] GUI
- [ ] Configuration file
