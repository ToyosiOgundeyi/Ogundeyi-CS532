CS 532 Homework 3
Directory File Processing Using Child Processes

Name: Toyosi Ogundeyi
BlazerId: Ogundeyi

Files Submitted:
hw3.c
Makefile
README.txt

Compilation:
make

Execution:
./hw3 <directory_name>

User Ownership Filter:
./hw3 -u <username> <directory_name>

Program Description:
This program demonstrates directory traversal, file processing, process creation, and user ownership filtering in C.

The program accepts a directory name as a command line argument. It opens the directory, examines its entries, and processes only regular files. Subdirectories and other nonregular entries are ignored.

For each eligible regular file, the parent process creates a separate child process using fork(). Each child process displays the file name and size. If the file has a .txt extension, the child also counts and displays the number of words in the file. For files without a .txt extension, the program displays Words: N/A.

The parent process waits for all child processes to complete before terminating.

User Ownership Filter:
The optional -u argument allows the program to process only files owned by a specified user.

When this option is used, the program verifies that the username exists, filters files using the user ID, and displays the owner of each processed file.

Input:
The program requires a directory name as a command line argument.

An optional username may be supplied with the -u option.

Output:
For each eligible regular file, the program displays:
File name
File size in bytes
Word count for .txt files
Words: N/A for other file types
File owner when the -u option is used

Error Handling:
The program handles:
Missing or incorrect command line arguments
Invalid usernames
Directories that do not exist or cannot be opened
Memory allocation failures
File access errors
File reading errors
Child process creation failures

Testing:
The program was tested by:
Compiling with make
Running the program with a valid directory
Running the program with the -u option
Processing regular .txt files
Processing nontext files
Processing an empty .txt file
Verifying that subdirectories were ignored
Testing a nonexistent directory
Testing an invalid username
Testing missing command line arguments

Notes:
The order of the file output may change between executions because each file is processed by a separate child process.

Only files with names ending in .txt are included in the word count calculation.