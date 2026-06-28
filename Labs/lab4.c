/*
Compile: gcc -Wall -o lab4 lab4.c

Run: ./lab4 file1 file2
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUFFSIZE 4096

int main(int argc, char *argv[])
{
    int readFileDescriptor;
    int writeFileDescriptor;
    long int n;
    char buf[BUFFSIZE];

    if (argc != 3)
    {
        printf("Usage: %s <file1> <file2>\n", argv[0]);
        exit(-1);
    }

    if (strcmp(argv[1], argv[2]) == 0)
    {
        printf("Error: The two filenames must not be the same.\n");
        exit(-1);
    }

    writeFileDescriptor = open(argv[1], O_WRONLY | O_APPEND);
    readFileDescriptor = open(argv[2], O_RDONLY);

    if (readFileDescriptor == -1 || writeFileDescriptor == -1)
    {
        printf("Error opening file.\n");
        exit(-1);
    }

    while ((n = read(readFileDescriptor, buf, BUFFSIZE)) > 0)
    {
        if (write(writeFileDescriptor, buf, n) != n)
        {
            printf("Error writing to file.\n");
            exit(-1);
        }
    }

    if (n < 0)
    {
        printf("Error reading file.\n");
        exit(-1);
    }

    close(readFileDescriptor);
    close(writeFileDescriptor);

    return 0;
}