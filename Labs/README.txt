CS 532 Lab 7
Process Creation Using fork() and exec()

Name: Toyosi Ogundeyi
BlazerId: Ogundeyi

Files Submitted:
lab7.c
README.txt

Compilation:
gcc -Wall -Wextra -pedantic -std=c11 -o lab7 lab7.c

Execution:
./lab7 commands.txt

Program Description:
This program reads commands from an input file one line at a time.
For each command, it creates a child process using fork().
The child process executes the command using execvp().
The parent process records the start time before creating the child process,
waits for the child process to finish using waitpid(), records the end time,
and writes the command, start time, and end time to output.log.

Input:
The input file contains one command per line with optional command-line
arguments.

Output:
The program creates output.log containing:
- command executed
- start time
- end time

Testing:
The program was tested using commands such as:
uname -a
echo Hello World
sleep 2
date
pwd
whoami
ls -l

Notes:
- execvp() is used to execute commands with variable numbers of arguments.
- waitpid() ensures the parent waits for each child process before processing the next command.
- The program uses the standard I/O library functions for file operations.