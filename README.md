# **Matching-Dice-Command-Line-Game-**

## **Brief Overview**
### A very simple game that uses POSIX threads. The game includes one dealer, two teams of two players, and two dice. Each team competes by rolling dice one at a time. Before players start rolling the dice, the dealer randomly selects a player to go first. After that first roll, players continue to take turns rolling the dice in order A, B, C and D. If the current player’s rolls sum up to the amount of their last teammates’, that team wins. If not, players continue to take turns in order.

## **Implementation**
###This project was implemented in C using POSIX threads.
For this project, I used the libraries stdio.h, stdlib.h, stdbool.h, and pthreads.h.

## **Instructions**
###Compiling the main.c file creates the compiled executable a.out. To run a.out, type the following: • ./a.out [seed]
• i.e. ./a.out 1445 or ./a.out 3