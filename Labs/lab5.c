/*
Compile: gcc -Wall -o lab5 lab5.c

Run: ./lab5 .
*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

/* This function returns the file type. */
char *filetype(unsigned char type)
{
    char *str;
    switch (type){
    case DT_BLK: str = "block device"; break;
    case DT_CHR: str = "character device"; break;
    case DT_DIR: str = "directory"; break;
    case DT_FIFO: str = "named pipe (FIFO)"; break;
    case DT_LNK: str = "symbolic link"; break;
    case DT_REG: str = "regular file"; break;
    case DT_SOCK: str = "UNIX domain socket"; break;
    case DT_UNKNOWN: str = "unknown file type"; break;
    default: str = "UNKNOWN";
    }
    return str;
}

/* This function recursively traverses directories. */
void traverseDirectory(char *path, int level)
{
    DIR *parentDir;
    struct dirent *dirent;
    struct stat statbuf;
    char fullPath[1024];
    int i;

    parentDir = opendir(path);

    if (parentDir == NULL)
    {
        printf("Error opening directory '%s'\n", path);
        return;
    }

    while ((dirent = readdir(parentDir)) != NULL)
    {
        if (strcmp(dirent->d_name, ".") == 0 ||
            strcmp(dirent->d_name, "..") == 0)
        {
            continue;
        }

        for (i = 0; i < level; i++)
        {
            printf("\t");
        }

        printf("%s (%s)\n", dirent->d_name, filetype(dirent->d_type));

        strcpy(fullPath, path);
        strcat(fullPath, "/");
        strcat(fullPath, dirent->d_name);

        if (stat(fullPath, &statbuf) == -1)
        {
            printf("stat error\n");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode))
        {
            traverseDirectory(fullPath, level + 1);
        }
    }

    closedir(parentDir);
}

/* The main function starts the recursive directory traversal. */
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <dirname>\n", argv[0]);
        exit(-1);
    }

    printf("%s\n", argv[1]);

    traverseDirectory(argv[1], 1);

    return 0;
}