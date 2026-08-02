CS 532 Bonus Homework
Producer-Consumer Using Processes, Threads, Pipes, and Signals

Name: Toyosi Ogundeyi
BlazerId: Ogundeyi

Files Submitted:
producer_consumer.c
Makefile
README.txt
Bonus Homework.pdf

Compilation:
make

Execution:
./producer_consumer

Program Description:
This program demonstrates process management, multi-threading, inter-process communication (IPC), synchronization, file operations, and signals in C.

The parent process creates ten producer threads. Each producer thread generates 500 random integers between 0 and 1000. The generated numbers are unique within each producer thread. After generating the numbers, the producer threads write them to a shared pipe using mutexes to prevent data corruption.

After all producer threads finish generating their numbers, the parent process sends a SIGUSR1 signal to the child process. The child waits for this signal before creating twenty consumer threads.

Each consumer thread reads 250 integers from the shared pipe and calculates the sum of the numbers it reads. After all consumer threads complete, the child process calculates the average of the twenty sums and writes the result to average.txt using standard output redirection.

Synchronization:
- Mutexes are used to protect pipe read operations.
- Mutexes are used to protect pipe write operations.
- Mutexes are used to prevent overlapping console output.
- Semaphores coordinate the producer threads by allowing the parent to detect when generation is complete and then release the producers to begin writing to the pipe.
- SIGUSR1 is used to notify the child process when all producer threads have finished generating their numbers.

Input:
No user input is required.

Output:
The program displays producer and consumer progress on the terminal.

The program creates:
average.txt

which contains the average of the twenty consumer thread sums.

Testing:
The program was tested by:
- Compiling with make.
- Executing ./producer_consumer.
- Verifying that:
  - 10 producer threads generated 500 numbers each.
  - 20 consumer threads each read 250 numbers.
  - The child process waited for SIGUSR1 before starting the consumer threads.
  - average.txt was created successfully.
  - The average of the consumer sums was written correctly to average.txt.

Notes:
- Each producer thread generates 500 unique random numbers within that producer thread.
- Duplicate numbers may exist across different producer threads.
- The program uses POSIX threads, pipes, semaphores, mutexes, and signals.