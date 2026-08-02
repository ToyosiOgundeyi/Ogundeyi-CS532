/*
Name: Toyosi Ogundeyi
BlazerId: Ogundeyi
Project #: Homework 3
To compile: make
To run: ./hw3 <directory_name>
        ./hw3 -u <username> <directory_name>
*/

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*
Prints the correct command format.
*/
void printUsage(const char *programName)
{
    fprintf(stderr, "Usage: %s <directory_name>\n", programName);
    fprintf(stderr, "       %s -u <username> <directory_name>\n",
            programName);
}

/*
Checks whether a file name ends with the .txt extension.
*/
int isTextFile(const char *fileName)
{
    const char *extension;

    extension = strrchr(fileName, '.');

    if (extension == NULL)
    {
        return 0;
    }

    return strcmp(extension, ".txt") == 0;
}

/*
Creates the complete path to a file by combining the
directory name and file name.

The caller is responsible for freeing the returned memory.
*/
char *createFilePath(const char *directoryName, const char *fileName)
{
    size_t directoryLength;
    size_t fileNameLength;
    size_t pathLength;
    int needsSlash;
    char *filePath;

    directoryLength = strlen(directoryName);
    fileNameLength = strlen(fileName);

    needsSlash = directoryLength > 0 &&
                 directoryName[directoryLength - 1] != '/';

    pathLength = directoryLength + fileNameLength + 2;

    filePath = malloc(pathLength);

    if (filePath == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate memory.\n");
        return NULL;
    }

    if (needsSlash)
    {
        snprintf(filePath, pathLength, "%s/%s",
                 directoryName, fileName);
    }
    else
    {
        snprintf(filePath, pathLength, "%s%s",
                 directoryName, fileName);
    }

    return filePath;
}

/*
Counts the number of words in a text file.

A word begins when a non-whitespace character is found after
whitespace or at the beginning of the file.

The file is processed one character at a time, so the function
can also handle large files without loading the entire file
into memory.

Returns:
    Number of words when successful
    -1 when the file cannot be opened or read
*/
long long countWords(const char *filePath)
{
    FILE *file;
    int currentCharacter;
    int insideWord;
    long long wordCount;

    file = fopen(filePath, "r");

    if (file == NULL)
    {
        fprintf(stderr, "Error: Cannot open file %s: %s\n",
                filePath, strerror(errno));
        return -1;
    }

    insideWord = 0;
    wordCount = 0;

    while ((currentCharacter = fgetc(file)) != EOF)
    {
        if (isspace((unsigned char)currentCharacter))
        {
            insideWord = 0;
        }
        else if (!insideWord)
        {
            insideWord = 1;
            wordCount++;
        }
    }

    if (ferror(file))
    {
        fprintf(stderr, "Error: Cannot read file %s.\n", filePath);
        fclose(file);
        return -1;
    }

    fclose(file);

    return wordCount;
}

/*
Processes one file in a child process.

The function obtains the file information using stat(),
counts the words when the file has a .txt extension, and
prints the required information.
*/
void processFile(const char *filePath,
                 const char *fileName,
                 int displayOwner)
{
    struct stat fileInformation;
    struct passwd *ownerInformation;
    const char *ownerName;
    long long wordCount;

    if (stat(filePath, &fileInformation) == -1)
    {
        fprintf(stderr, "Error: Cannot access file %s: %s\n",
                filePath, strerror(errno));
        return;
    }

    ownerInformation = getpwuid(fileInformation.st_uid);

    if (ownerInformation != NULL)
    {
        ownerName = ownerInformation->pw_name;
    }
    else
    {
        ownerName = "Unknown";
    }

    if (isTextFile(fileName))
    {
        wordCount = countWords(filePath);

        if (wordCount == -1)
        {
            return;
        }

        if (displayOwner)
        {
            printf("File: %s | Size: %lld bytes | Words: %lld | "
                   "Owner: %s\n",
                   fileName,
                   (long long)fileInformation.st_size,
                   wordCount,
                   ownerName);
        }
        else
        {
            printf("File: %s | Size: %lld bytes | Words: %lld\n",
                   fileName,
                   (long long)fileInformation.st_size,
                   wordCount);
        }
    }
    else
    {
        /*
        Word counting is required only for files ending in .txt.
        */
        if (displayOwner)
        {
            printf("File: %s | Size: %lld bytes | Words: N/A | "
                   "Owner: %s\n",
                   fileName,
                   (long long)fileInformation.st_size,
                   ownerName);
        }
        else
        {
            printf("File: %s | Size: %lld bytes | Words: N/A\n",
                   fileName,
                   (long long)fileInformation.st_size);
        }
    }

    fflush(stdout);
}

/*
Waits for every child process created by the parent.
*/
void waitForChildren(int numberOfChildren)
{
    int childrenCompleted;
    pid_t waitResult;

    childrenCompleted = 0;

    while (childrenCompleted < numberOfChildren)
    {
        waitResult = wait(NULL);

        if (waitResult > 0)
        {
            childrenCompleted++;
        }
        else if (waitResult == -1 && errno == EINTR)
        {
            /*
            The wait operation was interrupted.
            Try waiting again.
            */
            continue;
        }
        else
        {
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    DIR *directory;
    struct dirent *directoryEntry;
    struct stat fileInformation;
    struct passwd *specifiedUser;
    const char *directoryName;
    const char *username;
    char *filePath;
    pid_t processId;
    uid_t specifiedUserId;
    int option;
    int filterByOwner;
    int childCount;
    int fileCount;
    int forkFailed;

    username = NULL;
    directoryName = NULL;
    filterByOwner = 0;
    childCount = 0;
    fileCount = 0;
    forkFailed = 0;

    /*
    Read the optional -u username command-line option.
    */
    while ((option = getopt(argc, argv, "u:")) != -1)
    {
        switch (option)
        {
            case 'u':
                filterByOwner = 1;
                username = optarg;
                break;

            default:
                printUsage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    /*
    Exactly one directory argument must remain after processing
    the command-line options.
    */
    if (optind >= argc || optind + 1 != argc)
    {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    directoryName = argv[optind];

    /*
    Confirm that the username supplied with -u exists.
    */
    if (filterByOwner)
    {
        specifiedUser = getpwnam(username);

        if (specifiedUser == NULL)
        {
            fprintf(stderr, "Error: User '%s' was not found.\n",
                    username);
            return EXIT_FAILURE;
        }

        specifiedUserId = specifiedUser->pw_uid;
    }

    /*
    Attempt to open the supplied directory.
    */
    directory = opendir(directoryName);

    if (directory == NULL)
    {
        fprintf(stderr, "Error: Directory not found or cannot be "
                "opened: %s\n",
                directoryName);
        return EXIT_FAILURE;
    }

    /*
    Read each entry in the directory.
    */
    while ((directoryEntry = readdir(directory)) != NULL)
    {
        /*
        Skip the current-directory and parent-directory entries.
        */
        if (strcmp(directoryEntry->d_name, ".") == 0 ||
            strcmp(directoryEntry->d_name, "..") == 0)
        {
            continue;
        }

        filePath = createFilePath(directoryName,
                                  directoryEntry->d_name);

        if (filePath == NULL)
        {
            forkFailed = 1;
            break;
        }

        /*
        Obtain file information to determine whether the entry
        is a regular file.
        */
        if (stat(filePath, &fileInformation) == -1)
        {
            fprintf(stderr, "Error: Cannot access %s: %s\n",
                    filePath, strerror(errno));
            free(filePath);
            continue;
        }

        /*
        Exclude directories and other non-regular entries.
        */
        if (!S_ISREG(fileInformation.st_mode))
        {
            free(filePath);
            continue;
        }

        /*
        When -u is used, process only files belonging to the
        specified user.
        */
        if (filterByOwner &&
            fileInformation.st_uid != specifiedUserId)
        {
            free(filePath);
            continue;
        }

        fileCount++;

        /*
        Create one child process for the current file.
        */
        processId = fork();

        if (processId == -1)
        {
            fprintf(stderr, "Error: Unable to create child "
                    "process: %s\n",
                    strerror(errno));

            free(filePath);
            forkFailed = 1;
            break;
        }

        if (processId == 0)
        {
            /*
            Child process:
            Print the file name, size, word count, and optional
            owner information.
            */
            processFile(filePath,
                        directoryEntry->d_name,
                        filterByOwner);

            free(filePath);
            closedir(directory);

            _exit(EXIT_SUCCESS);
        }

        /*
        Parent process:
        Keep track of the number of children and continue to the
        next file.
        */
        childCount++;
        free(filePath);
    }

    closedir(directory);

    /*
    The parent waits for every child process before terminating.
    */
    waitForChildren(childCount);

    if (forkFailed)
    {
        return EXIT_FAILURE;
    }

    if (fileCount == 0)
    {
        if (filterByOwner)
        {
            printf("No regular files owned by %s were found in "
                   "the directory.\n",
                   username);
        }
        else
        {
            printf("No regular files were found in the "
                   "directory.\n");
        }
    }

    return EXIT_SUCCESS;
}