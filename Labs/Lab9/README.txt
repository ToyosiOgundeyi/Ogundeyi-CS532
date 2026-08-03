CS 532 Lab 9
Signal Handling with Fork, Exec, and Wait

Name: Toyosi Ogundeyi
BlazerId: Ogundeyi

Files Submitted:
forkexecvp.c
README.txt

Compilation:
gcc -Wall -Wextra -pedantic -std=c11 -o forkexecvp forkexecvp.c

Execution:
./forkexecvp <command> [arguments]

Example:
./forkexecvp sleep 100

Program Description:
This program demonstrates process creation, program execution, process management, and signal handling in UNIX.

The program accepts a command and optional arguments from the command line. It creates a child process using fork(). The child process executes the requested command using execvp().

The parent process installs signal handlers for SIGINT, SIGTSTP, SIGQUIT, and SIGCHLD.

Signal Behavior:
Control-C generates SIGINT and interrupts the child process. The parent remains active.

Control-Z generates SIGTSTP and suspends the child process. The parent remains active.

Control-\ generates SIGQUIT. The parent ends its waiting loop, terminates any remaining child process, and exits cleanly.

SIGCHLD notifies the parent when the child process exits, is interrupted, is suspended, or continues.

The child process uses the default actions for SIGINT, SIGTSTP, and SIGQUIT. The parent replaces the default actions so it can continue waiting until SIGQUIT is received.

Testing:
The program was tested by compiling with the required compiler flags and running:

./forkexecvp sleep 100

The following keyboard signals were tested:

Control-C to interrupt the child process.

Control-Z to suspend the child process.

Control-\ to terminate the parent process.

The program was also tested with no command-line argument to confirm that it prints the correct usage instructions.

Notes:
The parent remains active after the child is interrupted or suspended.

If the child is still running or suspended when the parent receives SIGQUIT, the parent continues the child if necessary, terminates it, and waits for it before exiting.

No known issues were observed during testing.
