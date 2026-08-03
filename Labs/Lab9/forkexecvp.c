#define _POSIX_C_SOURCE 200809L

/*
Name: Toyosi Ogundeyi
BlazerId: Ogundeyi
Project #: Lab 9
To compile: gcc -Wall -Wextra -pedantic -std=c11 -o forkexecvp forkexecvp.c
To run: ./forkexecvp <command> [arguments]
*/

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t quitRequested = 0;
static pid_t childProcessId = -1;

/*
Handles signals received by the parent.

SIGINT and SIGTSTP are forwarded to the child.
SIGQUIT tells the parent to end its waiting loop.
SIGCHLD wakes the parent when the child's status changes.
*/
static void signalHandler(int signalNumber)
{
    if (signalNumber == SIGQUIT)
    {
        quitRequested = 1;
    }
    else if ((signalNumber == SIGINT ||
              signalNumber == SIGTSTP) &&
             childProcessId > 0)
    {
        kill(childProcessId, signalNumber);
    }
}

/*
Installs the parent signal handler using sigaction().
*/
static int installSignalHandler(int signalNumber)
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = signalHandler;
    sigemptyset(&action.sa_mask);

    return sigaction(signalNumber, &action, NULL);
}

/*
Restores a signal's default behavior in the child.
*/
static int restoreDefaultSignal(int signalNumber)
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = SIG_DFL;
    sigemptyset(&action.sa_mask);

    return sigaction(signalNumber, &action, NULL);
}

/*
Reports changes in the child process status.
*/
static void reportChildStatus(int status)
{
    if (WIFEXITED(status))
    {
        printf("Child process exited with status = %d\n",
               WEXITSTATUS(status));
    }
    else if (WIFSIGNALED(status))
    {
        printf("Child process was interrupted by signal %d\n",
               WTERMSIG(status));
    }
    else if (WIFSTOPPED(status))
    {
        printf("Child process was suspended by signal %d\n",
               WSTOPSIG(status));
    }
    else if (WIFCONTINUED(status))
    {
        printf("Child process continued.\n");
    }

    fflush(stdout);
}

int main(int argc, char *argv[])
{
    sigset_t blockedSignals;
    sigset_t previousMask;
    pid_t waitResult;
    int childStatus;
    int childIsRunning;

    if (argc < 2)
    {
        fprintf(stderr,
                "Usage: %s <command> [arguments]\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    /*
    Block the signals before fork() to prevent a signal from
    arriving before the parent is ready to handle it.
    */
    sigemptyset(&blockedSignals);
    sigaddset(&blockedSignals, SIGINT);
    sigaddset(&blockedSignals, SIGTSTP);
    sigaddset(&blockedSignals, SIGQUIT);
    sigaddset(&blockedSignals, SIGCHLD);

    if (sigprocmask(SIG_BLOCK,
                    &blockedSignals,
                    &previousMask) == -1)
    {
        perror("sigprocmask");
        return EXIT_FAILURE;
    }

    if (installSignalHandler(SIGINT) == -1 ||
        installSignalHandler(SIGTSTP) == -1 ||
        installSignalHandler(SIGQUIT) == -1 ||
        installSignalHandler(SIGCHLD) == -1)
    {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    childProcessId = fork();

    if (childProcessId == -1)
    {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (childProcessId == 0)
    {
        /*
        Restore the default signal actions in the child before
        replacing it with the requested program.
        */
        if (restoreDefaultSignal(SIGINT) == -1 ||
            restoreDefaultSignal(SIGTSTP) == -1 ||
            restoreDefaultSignal(SIGQUIT) == -1 ||
            restoreDefaultSignal(SIGCHLD) == -1)
        {
            perror("sigaction");
            _exit(EXIT_FAILURE);
        }

        if (sigprocmask(SIG_SETMASK,
                        &previousMask,
                        NULL) == -1)
        {
            perror("sigprocmask");
            _exit(EXIT_FAILURE);
        }

        execvp(argv[1], &argv[1]);

        perror("execvp");
        _exit(127);
    }

    childIsRunning = 1;

    printf("Parent process %ld created child process %ld.\n",
           (long)getpid(),
           (long)childProcessId);
    printf("Press Control-C to interrupt the child.\n");
    printf("Press Control-Z to suspend the child.\n");
    printf("Press Control-\\ to quit the parent.\n");
    fflush(stdout);

    /*
    The parent continues waiting until it receives SIGQUIT.
    */
    while (!quitRequested)
    {
        sigsuspend(&previousMask);

        do
        {
            waitResult = waitpid(childProcessId,
                                 &childStatus,
                                 WNOHANG |
                                 WUNTRACED |
                                 WCONTINUED);

            if (waitResult == childProcessId)
            {
                reportChildStatus(childStatus);

                if (WIFEXITED(childStatus) ||
                    WIFSIGNALED(childStatus))
                {
                    childIsRunning = 0;
                }
            }
        }
        while (waitResult == childProcessId &&
               childIsRunning);
    }

    printf("Parent received SIGQUIT and will exit.\n");

    /*
    If the child is still running or suspended, continue it,
    terminate it, and collect its exit status.
    */
    if (childIsRunning)
    {
        kill(childProcessId, SIGCONT);
        kill(childProcessId, SIGTERM);

        do
        {
            waitResult = waitpid(childProcessId,
                                 &childStatus,
                                 0);
        }
        while (waitResult == -1 && errno == EINTR);
    }

    if (sigprocmask(SIG_SETMASK,
                    &previousMask,
                    NULL) == -1)
    {
        perror("sigprocmask");
        return EXIT_FAILURE;
    }

    printf("[%ld]: Exiting program .....\n",
           (long)getpid());

    return EXIT_SUCCESS;
}