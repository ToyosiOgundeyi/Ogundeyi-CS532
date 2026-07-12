/*
Name: Toyosi Ogundeyi
BlazerId: Ogundeyi
Project #: Homework 2
To compile: make
To run: 
    ./search projects
    ./search -S projects
    ./search -s 1024 projects
    ./search -f docx 2 projects
    ./search -t f projects
    ./search -t d projects
    ./search -S -s 1024 -f docx 2 projects
*/

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <time.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define PATHSIZE PATH_MAX
#define LINKSIZE PATH_MAX

/*
This function-pointer type represents a function that prints
information about one file or directory.
*/
typedef void PRINTFUNC(char *name, char *fullPath, struct stat *statbuf, int depth);

typedef struct
{
    int showDetails;       /* -S */

    int useSizeFilter;     /* -s */
    off_t maximumSize;

    int useNameFilter;     /* -f */
    char *pattern;
    int maximumDepth;

    int useTypeFilter;     /* -t */
    char requestedType;    /* f or d */
} SearchOptions;

SearchOptions options = {0};

/* Prints tabs based on the current depth. */
void printIndentation(int depth)
{
    int i;

    for (i = 0; i < depth; i++)
    {
        printf("\t");
    }
}

/*
Checks whether a file or directory satisfies all active filters.
Returns 1 when the entry matches and 0 when it does not match.
*/
int matchesFilters(char *name, struct stat *statbuf, int depth)
{
    off_t entrySize;

    /*
    Directories are treated as size 0 according to the assignment.
    */
    if (S_ISDIR(statbuf->st_mode))
    {
        entrySize = 0;
    }
    else
    {
        entrySize = statbuf->st_size;
    }

    /*
    Apply the -s size filter.
    */
    if (options.useSizeFilter)
    {
        if (entrySize > options.maximumSize)
        {
            return 0;
        }
    }

    /*
    Apply the -f pattern and depth filters.
    */
    if (options.useNameFilter)
    {
        if (strstr(name, options.pattern) == NULL)
        {
            return 0;
        }

        if (depth > options.maximumDepth)
        {
            return 0;
        }
    }

    /*
    Apply the -t type filter.
    */
    if (options.useTypeFilter)
    {
        if (options.requestedType == 'f' && !S_ISREG(statbuf->st_mode))
        {
            return 0;
        }

        if (options.requestedType == 'd' && !S_ISDIR(statbuf->st_mode))
        {
            return 0;
        }
    }

    return 1;
}

/*
Prints a file or directory name.

If the entry is a symbolic link, the function also prints
the file or directory that the link points to.
*/
void printBasic(char *name, char *fullPath, struct stat *statbuf, int depth)
{
    char linkTarget[LINKSIZE];
    ssize_t linkLength;

    if (!matchesFilters(name, statbuf, depth))
    {
        return;
    }

    printIndentation(depth);

    if (S_ISLNK(statbuf->st_mode))
    {
        linkLength = readlink(fullPath, linkTarget, LINKSIZE - 1);

        if (linkLength == -1)
        {
            printf("%s (unable to read symbolic link)\n", name);
        }
        else
        {
            linkTarget[linkLength] = '\0';
            printf("%s (%s)\n", name, linkTarget);
        }
    }
    else
    {
        printf("%s\n", name);
    }
}

void getPermissions(mode_t mode, char permissions[10])
{
    permissions[0] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[1] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[2] = (mode & S_IXUSR) ? 'x' : '-';

    permissions[3] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[4] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[5] = (mode & S_IXGRP) ? 'x' : '-';

    permissions[6] = (mode & S_IROTH) ? 'r' : '-';
    permissions[7] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[8] = (mode & S_IXOTH) ? 'x' : '-';

    permissions[9] = '\0';
}

void printDetailed(char *name, char *fullPath, struct stat *statbuf, int depth)
{
    char permissions[10];
    char accessTime[100];
    char linkTarget[LINKSIZE];
    ssize_t linkLength;
    off_t entrySize;
    struct tm *timeInfo;

    if (!matchesFilters(name, statbuf, depth))
    {
        return;
    }

    if (S_ISDIR(statbuf->st_mode))
    {
        entrySize = 0;
    }
    else
    {
        entrySize = statbuf->st_size;
    }

    getPermissions(statbuf->st_mode, permissions);

    timeInfo = localtime(&statbuf->st_atime);

    if (timeInfo != NULL)
    {
        strftime(accessTime, sizeof(accessTime), "%Y-%m-%d %H:%M:%S", timeInfo);
    }
    else
    {
        strcpy(accessTime, "unknown");
    }

    printIndentation(depth);

    if (S_ISLNK(statbuf->st_mode))
    {
        linkLength = readlink(fullPath, linkTarget, LINKSIZE - 1);

        if (linkLength == -1)
        {
            printf("%s (unable to read symbolic link)",
                   name);
        }
        else
        {
            linkTarget[linkLength] = '\0';
            printf("%s (%s)", name, linkTarget);
        }
    }
    else
    {
        printf("%s", name);
    }

    printf(" (size: %lld bytes, permissions: %s, last access: %s)\n",
           (long long)entrySize,
           permissions,
           accessTime);
}

/*
Recursively traverses a directory.

The printFunction parameter is a function pointer used to
print each file or directory found during the traversal.
*/
void traverseDirectory(char *path, int depth, PRINTFUNC *printFunction)
{
    DIR *parentDir;
    struct dirent *entry;
    struct stat statbuf;
    char fullPath[PATHSIZE];
    int pathLength;

    parentDir = opendir(path);

    if (parentDir == NULL)
    {
        fprintf(stderr, "Error opening directory '%s': %s\n", path, strerror(errno));
        return;
    }

    while ((entry = readdir(parentDir)) != NULL)
    {
        /*
        Skip the current-directory and parent-directory entries
        to prevent infinite recursion.
        */
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        pathLength = snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);

        if (pathLength < 0 ||
            pathLength >= (int)sizeof(fullPath))
        {
            fprintf(stderr, "Path is too long: %s/%s\n", path, entry->d_name);
            continue;
        }

        /*
        lstat is used instead of stat so that symbolic links
        are identified as links rather than followed.
        */
        if (lstat(fullPath, &statbuf) == -1)
        {
            printIndentation(depth);
            fprintf(stderr, "%s (lstat error: %s)\n", entry->d_name, strerror(errno));
            continue;
        }

        /*
        Call the printing function through the function pointer.
        */
        printFunction(entry->d_name, fullPath, &statbuf, depth);

        /*
        Recurse only when the entry is an actual directory.
        Symbolic links to directories are not followed.
        */
        if (S_ISDIR(statbuf.st_mode))
        {
            if (!options.useNameFilter || depth < options.maximumDepth)
            {
                traverseDirectory(fullPath, depth + 1, printFunction);
            }
        }
    }

    closedir(parentDir);
}

void printUsage(char *programName)
{
    fprintf(stderr,
            "Usage: %s [-S] [-s size] "
            "[-f pattern depth] [-t f|d] "
            "[directory]\n",
            programName);
}

int main(int argc, char *argv[])
{
    char *startingDirectory;
    struct stat statbuf;
    PRINTFUNC *printFunction;
    int commandOption;
    char *endPointer;

    startingDirectory = ".";

    /*
    Process options using getopt.
    */
    while ((commandOption =
            getopt(argc, argv, "Ss:f:t:")) != -1)
    {
        switch (commandOption)
        {
            case 'S':
                options.showDetails = 1;
                break;

            case 's':
                errno = 0;

                options.maximumSize =
                    strtol(optarg, &endPointer, 10);

                if (errno != 0 || *endPointer != '\0' || options.maximumSize < 0)
                {
                    fprintf(stderr, "Invalid size: %s\n", optarg);
                    printUsage(argv[0]);
                    return 1;
                }

                options.useSizeFilter = 1;
                break;

            case 'f':
                options.pattern = optarg;

                /*
                The next argument after the pattern is depth.
                */
                if (optind >= argc)
                {
                    fprintf(stderr, "-f requires a pattern and depth.\n");
                    printUsage(argv[0]);
                    return 1;
                }

                errno = 0;

                options.maximumDepth = strtol(argv[optind], &endPointer, 10);

                if (errno != 0 ||
                    *endPointer != '\0' ||
                    options.maximumDepth < 0)
                {
                    fprintf(stderr, "Invalid depth: %s\n", argv[optind]);
                    printUsage(argv[0]);
                    return 1;
                }

                options.useNameFilter = 1;

                /*
                Move past the depth argument.
                */
                optind++;
                break;

            case 't':
                if (strlen(optarg) != 1 ||
                    (optarg[0] != 'f' &&
                     optarg[0] != 'd'))
                {
                    fprintf(stderr, "-t must be followed by f or d.\n");
                    printUsage(argv[0]);
                    return 1;
                }

                options.useTypeFilter = 1;
                options.requestedType = optarg[0];
                break;

            default:
                printUsage(argv[0]);
                return 1;
        }
    }

    /*
    After processing options, one optional directory may remain.
    */
    if (optind < argc)
    {
        startingDirectory = argv[optind];
        optind++;
    }

    /*
    Reject extra directory arguments.
    */
    if (optind < argc)
    {
        fprintf(stderr, "Too many directory arguments.\n");
        printUsage(argv[0]);
        return 1;
    }

    /*
    Verify the starting path.
    */
    if (lstat(startingDirectory, &statbuf) == -1)
    {
        fprintf(stderr, "Error reading '%s': %s\n", startingDirectory, strerror(errno));
        return 1;
    }

    if (!S_ISDIR(statbuf.st_mode))
    {
        fprintf(stderr, "Error: '%s' is not a directory.\n", startingDirectory);
        return 1;
    }

    /*
    Use the function pointer to select basic or detailed output.
    */
    if (options.showDetails)
    {
        printFunction = printDetailed;
    }
    else
    {
        printFunction = printBasic;
    }

    /*
    The starting directory has depth 0.
    */
    printFunction(startingDirectory, startingDirectory, &statbuf, 0);

    traverseDirectory(startingDirectory, 1, printFunction);

    return 0;
}

