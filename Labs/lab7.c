/*
Name: Toyosi Ogundeyi
BlazerId: Ogundeyi
Project: Lab 7

To compile:
    gcc -Wall -Wextra -pedantic -std=c11 -o lab7 lab7.c

To run:
    ./lab7 commands.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>

#define MAX_LINE_LENGTH 1024
#define MAX_ARGUMENTS 64

/*
Removes the newline character placed in a string by fgets()
or returned by ctime().
*/
void removeNewline(char *text)
{
    if (text != NULL)
    {
        text[strcspn(text, "\n")] = '\0';
    }
}

/*
Separates a command line into individual command-line arguments.

For example:

    uname -a

becomes:

    arguments[0] = "uname"
    arguments[1] = "-a"
    arguments[2] = NULL

The function returns the number of arguments found.
*/
int parseCommand(char *commandLine, char *arguments[])
{
    char *token;
    int argumentCount;

    argumentCount = 0;

    token = strtok(commandLine, " \t");

    while (token != NULL && argumentCount < MAX_ARGUMENTS - 1)
    {
        arguments[argumentCount] = token;
        argumentCount++;

        token = strtok(NULL, " \t");
    }

    /*
    execvp() requires the argument array to end with NULL.
    */
    arguments[argumentCount] = NULL;

    return argumentCount;
}

int main(int argc, char *argv[])
{
    FILE *inputFile;
    FILE *logFile;

    char line[MAX_LINE_LENGTH];
    char commandCopy[MAX_LINE_LENGTH];
    char parseBuffer[MAX_LINE_LENGTH];

    char *arguments[MAX_ARGUMENTS];

    pid_t childPid;
    pid_t waitResult;
    int status;
    int argumentCount;

    time_t startTime;
    time_t endTime;

    char startTimeString[64];
    char endTimeString[64];

    char *timeText;

    /*
    The program requires exactly one input filename.
    */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <input-file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /*
    Open the command input file for reading.
    */
    inputFile = fopen(argv[1], "r");

    if (inputFile == NULL)
    {
        fprintf(stderr,
                "Error opening input file '%s': %s\n",
                argv[1],
                strerror(errno));

        return EXIT_FAILURE;
    }

    /*
    Open output.log for writing.

    The "w" mode creates the file if it does not exist and clears
    its previous contents if it already exists.
    */
    logFile = fopen("output.log", "w");

    if (logFile == NULL)
    {
        fprintf(stderr,
                "Error opening output.log: %s\n",
                strerror(errno));

        fclose(inputFile);
        return EXIT_FAILURE;
    }

    /*
    Read and process the input file one line at a time.
    */
    while (fgets(line, sizeof(line), inputFile) != NULL)
    {
        /*
        Remove the newline read by fgets().
        */
        removeNewline(line);

        /*
        Keep one copy for logging and another copy for strtok().
        strtok() changes the string that it processes.
        */
        snprintf(commandCopy, sizeof(commandCopy), "%s", line);
        snprintf(parseBuffer, sizeof(parseBuffer), "%s", line);

        argumentCount = parseCommand(parseBuffer, arguments);

        /*
        Ignore empty or whitespace-only lines.
        */
        if (argumentCount == 0)
        {
            continue;
        }

        /*
        Record the time immediately before fork().
        */
        startTime = time(NULL);

        if (startTime == (time_t)-1)
        {
            perror("time");
            continue;
        }

        /*
        ctime() returns a string containing a newline.
        Copy it immediately because later calls to ctime() reuse
        the same internal storage.
        */
        timeText = ctime(&startTime);

        if (timeText == NULL)
        {
            fprintf(stderr, "Unable to convert start time.\n");
            continue;
        }

        snprintf(startTimeString,
                 sizeof(startTimeString),
                 "%s",
                 timeText);

        removeNewline(startTimeString);

        /*
        Create the child process.
        */
        childPid = fork();

        if (childPid == -1)
        {
            perror("fork");
            continue;
        }

        if (childPid == 0)
        {
            /*
            Child process:

            execvp() searches the PATH when the command does not
            contain a slash. It also supports full paths such as
            /sbin/ifconfig.
            */
            execvp(arguments[0], arguments);

            /*
            This section executes only if execvp() fails.
            */
            fprintf(stderr,
                    "Unable to execute '%s': %s\n",
                    arguments[0],
                    strerror(errno));

            _exit(127);
        }

        /*
        Parent process:

        Wait specifically for the child created above.
        */
        do
        {
            waitResult = waitpid(childPid, &status, 0);
        }
        while (waitResult == -1 && errno == EINTR);

        if (waitResult == -1)
        {
            perror("waitpid");
            continue;
        }

        /*
        Record the time after the child process finishes.
        */
        endTime = time(NULL);

        if (endTime == (time_t)-1)
        {
            perror("time");
            continue;
        }

        timeText = ctime(&endTime);

        if (timeText == NULL)
        {
            fprintf(stderr, "Unable to convert end time.\n");
            continue;
        }

        snprintf(endTimeString,
                 sizeof(endTimeString),
                 "%s",
                 timeText);

        removeNewline(endTimeString);

        /*
        Report an unsuccessful command without stopping the rest
        of the input file from being processed.
        */
        if (WIFEXITED(status))
        {
            if (WEXITSTATUS(status) != 0)
            {
                fprintf(stderr,
                        "Command '%s' exited with status %d.\n",
                        commandCopy,
                        WEXITSTATUS(status));
            }
        }
        else if (WIFSIGNALED(status))
        {
            fprintf(stderr,
                    "Command '%s' was terminated by signal %d.\n",
                    commandCopy,
                    WTERMSIG(status));
        }

        /*
        Write the command, start time, and end time separated
        by tabs, as required.
        */
        fprintf(logFile,
                "%s\t%s\t%s\n",
                commandCopy,
                startTimeString,
                endTimeString);

        /*
        Ensure each completed record is written immediately.
        */
        fflush(logFile);
    }

    /*
    Check whether fgets() stopped because of a reading error.
    */
    if (ferror(inputFile))
    {
        fprintf(stderr,
                "Error while reading input file '%s'.\n",
                argv[1]);

        fclose(inputFile);
        fclose(logFile);

        return EXIT_FAILURE;
    }

    fclose(inputFile);
    fclose(logFile);

    printf("All commands have been processed.\n");
    printf("Execution information was written to output.log.\n");

    return EXIT_SUCCESS;
}