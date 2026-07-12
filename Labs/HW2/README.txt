CS 532 Homework 2
Search Program
This program recursively traverses a directory hierarchy and supports filtering files and directories using command-line options.

Name: Toyosi Ogundeyi
Blazer ID: Ogundeyi

Files Submitted:
search.c
Makefile
README.txt

Compilation:
make

Execution:
    ./search projects
    ./search -S projects
    ./search -s 1024 projects
    ./search -f docx 2 projects
    ./search -t f projects
    ./search -t d projects
    ./search -S -s 1024 -f docx 2 projects

Options:

-S
Print size, permissions, and last access time.

-s size
Print entries whose sizes are less than or equal to the specified size in bytes.
Directories are treated as size 0.

-f pattern depth
Print entries whose names contain the specified pattern and whose depth is less than or equal to the specified depth.
Both conditions must be satisfied.

-t f
Print regular files only.

-t d
Print directories only.

Testing:
The program was tested using the sample projects directory supplied with the homework.
The following features were tested:
- Recursive directory traversal
- Symbolic link handling
- -S option
- -s option
- -f option
- -t option
- Multiple option combinations
- Invalid command-line arguments

Notes:
- Directory entries may appear in a different order because readdir() does not guarantee alphabetical ordering.
- The program uses function pointers to separate directory traversal from entry processing.