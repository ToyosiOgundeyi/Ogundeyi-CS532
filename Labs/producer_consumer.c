#define _POSIX_C_SOURCE 200809L

/*
Name: Toyosi Ogundeyi
BlazerId: Ogundeyi
Project: Producer-Consumer Bonus Homework

To compile:
    gcc -Wall -Wextra -pedantic -std=c11 -pthread \
    -o producer_consumer producer_consumer.c

To run:
    ./producer_consumer
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>

#define PRODUCER_COUNT 10
#define NUMBERS_PER_PRODUCER 500

#define CONSUMER_COUNT 20
#define NUMBERS_PER_CONSUMER 250

#define MIN_NUMBER 0
#define MAX_NUMBER 1000

/*
The two pipe file descriptors:

pipeFileDescriptors[0] is used for reading.
pipeFileDescriptors[1] is used for writing.
*/
int pipeFileDescriptors[2];

/*
The parent stores the child process ID so it can send SIGUSR1
after all producers have generated their numbers.
*/
pid_t childProcessId;

/*
The write mutex prevents multiple producer threads from writing
to the pipe at the same time.
*/
pthread_mutex_t pipeWriteMutex = PTHREAD_MUTEX_INITIALIZER;

/*
The read mutex prevents multiple consumer threads from reading
partial or overlapping integer data from the pipe.
*/
pthread_mutex_t pipeReadMutex = PTHREAD_MUTEX_INITIALIZER;

/*
The print mutex prevents messages from different threads from
appearing on top of each other.
*/
pthread_mutex_t printMutex = PTHREAD_MUTEX_INITIALIZER;

/*
Each producer posts to producersFinishedGenerating after it has
generated its 500 unique numbers.

The parent waits for ten posts before notifying the child.
*/
sem_t producersFinishedGenerating;

/*
Each producer waits on producersMayWrite after generating its
numbers.

The parent releases the producers only after notifying the child.
*/
sem_t producersMayWrite;

/*
This structure stores the information used by one producer thread.
*/
typedef struct
{
    int producerId;
    unsigned int seed;
    int numbers[NUMBERS_PER_PRODUCER];
} ProducerData;

/*
This structure stores the information produced by one consumer
thread.
*/
typedef struct
{
    int consumerId;
    long sum;
    int numbersRead;
} ConsumerData;

/*
Waits on a semaphore.

If sem_wait() is interrupted by a signal, the function tries again.
*/
int waitForSemaphore(sem_t *semaphore)
{
    int result;

    do
    {
        result = sem_wait(semaphore);
    }
    while (result == -1 && errno == EINTR);

    return result;
}

/*
Writes one complete integer to the pipe.

The loop ensures that all bytes belonging to the integer are
written, even if write() writes only part of the value.
*/
ssize_t writeInteger(int fileDescriptor, int number)
{
    const char *buffer;
    size_t bytesRemaining;
    ssize_t bytesWritten;

    buffer = (const char *)&number;
    bytesRemaining = sizeof(number);

    while (bytesRemaining > 0)
    {
        bytesWritten = write(fileDescriptor,
                             buffer,
                             bytesRemaining);

        if (bytesWritten == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }

            return -1;
        }

        buffer += bytesWritten;
        bytesRemaining -= (size_t)bytesWritten;
    }

    return (ssize_t)sizeof(number);
}

/*
Reads one complete integer from the pipe.

The function returns:

sizeof(int) if one integer was read successfully
0 if the end of the pipe was reached
-1 if an error occurred
*/
ssize_t readInteger(int fileDescriptor, int *number)
{
    char *buffer;
    size_t bytesRemaining;
    ssize_t bytesRead;

    buffer = (char *)number;
    bytesRemaining = sizeof(*number);

    while (bytesRemaining > 0)
    {
        bytesRead = read(fileDescriptor,
                         buffer,
                         bytesRemaining);

        if (bytesRead == 0)
        {
            return 0;
        }

        if (bytesRead == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }

            return -1;
        }

        buffer += bytesRead;
        bytesRemaining -= (size_t)bytesRead;
    }

    return (ssize_t)sizeof(*number);
}

/*
Producer thread function.

Each producer:

1. Generates 500 random numbers.
2. Ensures the 500 numbers are unique within that producer.
3. Reports generation progress.
4. Notifies the parent after generation is complete.
5. Waits for permission to begin writing.
6. Writes its 500 numbers to the shared pipe.
7. Prints its completion message and thread ID.
*/
void *producerThread(void *argument)
{
    ProducerData *producerData;
    int used[MAX_NUMBER + 1];
    int number;
    int index;

    producerData = (ProducerData *)argument;

    /*
    A value of 0 means that a number has not yet been used by
    this producer.

    A value of 1 means that it has already been selected.
    */
    memset(used, 0, sizeof(used));

    for (index = 0;
         index < NUMBERS_PER_PRODUCER;
         index++)
    {
        do
        {
            number =
                (int)(rand_r(&producerData->seed)
                % (MAX_NUMBER + 1));
        }
        while (used[number] != 0);

        used[number] = 1;
        producerData->numbers[index] = number;

        /*
        Print a progress message after every 100 numbers.
        */
        if ((index + 1) % 100 == 0)
        {
            pthread_mutex_lock(&printMutex);

            printf("Producer %d generated %d of %d numbers (%d%%).\n",
                   producerData->producerId,
                   index + 1,
                   NUMBERS_PER_PRODUCER,
                   ((index + 1) * 100)
                   / NUMBERS_PER_PRODUCER);

            pthread_mutex_unlock(&printMutex);
        }
    }

    /*
    Notify the parent that this producer has finished generating
    its numbers.
    */
    if (sem_post(&producersFinishedGenerating) == -1)
    {
        perror("sem_post");
        return NULL;
    }

    /*
    Wait until the parent has notified the child process and gives
    the producers permission to begin writing.
    */
    if (waitForSemaphore(&producersMayWrite) == -1)
    {
        perror("sem_wait");
        return NULL;
    }

    for (index = 0;
         index < NUMBERS_PER_PRODUCER;
         index++)
    {
        /*
        Only one producer may write to the pipe at a time.
        */
        pthread_mutex_lock(&pipeWriteMutex);

        if (writeInteger(pipeFileDescriptors[1],
                         producerData->numbers[index]) == -1)
        {
            pthread_mutex_unlock(&pipeWriteMutex);

            perror("write");
            return NULL;
        }

        pthread_mutex_unlock(&pipeWriteMutex);

        /*
        Bonus progress message.
        */
        if ((index + 1) % 100 == 0)
        {
            pthread_mutex_lock(&printMutex);

            printf("Producer %d wrote %d of %d numbers "
                   "to the pipe.\n",
                   producerData->producerId,
                   index + 1,
                   NUMBERS_PER_PRODUCER);

            pthread_mutex_unlock(&printMutex);
        }
    }

    pthread_mutex_lock(&printMutex);

    printf("Producer %d completed. Thread ID: %lu\n",
           producerData->producerId,
           (unsigned long)pthread_self());

    pthread_mutex_unlock(&printMutex);

    return NULL;
}

/*
Consumer thread function.

Each consumer:

1. Reads 250 integers from the pipe.
2. Calculates the sum of those integers.
3. Stores the sum in its ConsumerData structure.
4. Prints a completion message.
*/
void *consumerThread(void *argument)
{
    ConsumerData *consumerData;
    int number;
    int index;
    ssize_t readResult;

    consumerData = (ConsumerData *)argument;

    consumerData->sum = 0;
    consumerData->numbersRead = 0;

    for (index = 0;
         index < NUMBERS_PER_CONSUMER;
         index++)
    {
        /*
        Only one consumer may perform a pipe read at a time.
        */
        pthread_mutex_lock(&pipeReadMutex);

        readResult =
            readInteger(pipeFileDescriptors[0], &number);

        pthread_mutex_unlock(&pipeReadMutex);

        if (readResult == 0)
        {
            break;
        }

        if (readResult == -1)
        {
            perror("read");
            break;
        }

        consumerData->sum += number;
        consumerData->numbersRead++;
    }

    pthread_mutex_lock(&printMutex);

    printf("Consumer %d completed. Thread ID: %lu, "
           "numbers read: %d, sum: %ld\n",
           consumerData->consumerId,
           (unsigned long)pthread_self(),
           consumerData->numbersRead,
           consumerData->sum);

    pthread_mutex_unlock(&printMutex);

    return NULL;
}

/*
Runs the child process.

The child:

1. Closes the write side of the pipe.
2. Waits for SIGUSR1 from the parent.
3. Creates twenty consumer threads.
4. Waits for the consumers.
5. Calculates the average of their sums.
6. Redirects standard output to average.txt.
7. Writes the average to the file.
*/
int runChildProcess(const sigset_t *signalSet)
{
    pthread_t consumers[CONSUMER_COUNT];
    ConsumerData consumerData[CONSUMER_COUNT];

    int receivedSignal;
    int index;
    int createResult;

    long totalOfSums;
    double averageOfSums;

    /*
    The child does not write to the pipe.
    */
    close(pipeFileDescriptors[1]);

    /*
    SIGUSR1 was blocked before fork().

    sigwait() safely waits for the signal and avoids the possibility
    of the signal arriving just before pause() is called.
    */
    if (sigwait(signalSet, &receivedSignal) != 0)
    {
        fprintf(stderr,
                "Child could not wait for SIGUSR1.\n");

        close(pipeFileDescriptors[0]);

        return EXIT_FAILURE;
    }

    pthread_mutex_lock(&printMutex);

    printf("Child received SIGUSR1. "
           "Consumer threads are starting.\n");

    pthread_mutex_unlock(&printMutex);

    for (index = 0;
         index < CONSUMER_COUNT;
         index++)
    {
        consumerData[index].consumerId = index + 1;

        createResult =
            pthread_create(&consumers[index],
                           NULL,
                           consumerThread,
                           &consumerData[index]);

        if (createResult != 0)
        {
            fprintf(stderr,
                    "Unable to create consumer thread %d: %s\n",
                    index + 1,
                    strerror(createResult));

            close(pipeFileDescriptors[0]);

            return EXIT_FAILURE;
        }
    }

    for (index = 0;
         index < CONSUMER_COUNT;
         index++)
    {
        pthread_join(consumers[index], NULL);
    }

    close(pipeFileDescriptors[0]);

    totalOfSums = 0;

    for (index = 0;
         index < CONSUMER_COUNT;
         index++)
    {
        totalOfSums += consumerData[index].sum;
    }

    /*
    The homework asks for the average of the twenty consumer sums.
    */
    averageOfSums =
        (double)totalOfSums / CONSUMER_COUNT;

    /*
    Redirect standard output to average.txt.

    After freopen(), printf() writes to average.txt instead of
    the terminal.
    */
    if (freopen("average.txt", "w", stdout) == NULL)
    {
        perror("freopen");

        return EXIT_FAILURE;
    }

    printf("Average of the consumer thread sums: %.2f\n",
           averageOfSums);

    fflush(stdout);

    return EXIT_SUCCESS;
}

/*
Runs the parent process.

The parent:

1. Closes the read side of the pipe.
2. Creates ten producer threads.
3. Waits until all producers finish generating.
4. Sends SIGUSR1 to the child.
5. Allows the producers to begin writing.
6. Waits for all producer threads.
7. Closes the pipe.
8. Waits for the child process.
*/
int runParentProcess(void)
{
    pthread_t producers[PRODUCER_COUNT];
    ProducerData producerData[PRODUCER_COUNT];

    int index;
    int createResult;
    int childStatus;

    /*
    The parent does not read from the pipe.
    */
    close(pipeFileDescriptors[0]);

    if (sem_init(&producersFinishedGenerating,
                 0,
                 0) == -1)
    {
        perror("sem_init");

        close(pipeFileDescriptors[1]);

        return EXIT_FAILURE;
    }

    if (sem_init(&producersMayWrite,
                 0,
                 0) == -1)
    {
        perror("sem_init");

        sem_destroy(&producersFinishedGenerating);
        close(pipeFileDescriptors[1]);

        return EXIT_FAILURE;
    }

    for (index = 0;
         index < PRODUCER_COUNT;
         index++)
    {
        producerData[index].producerId = index + 1;

        /*
        Give each producer a different random seed.
        */
        producerData[index].seed =
            (unsigned int)time(NULL)
            ^ (unsigned int)(index * 7919)
            ^ (unsigned int)getpid();

        createResult =
            pthread_create(&producers[index],
                           NULL,
                           producerThread,
                           &producerData[index]);

        if (createResult != 0)
        {
            fprintf(stderr,
                    "Unable to create producer thread %d: %s\n",
                    index + 1,
                    strerror(createResult));

            close(pipeFileDescriptors[1]);

            return EXIT_FAILURE;
        }
    }

    /*
    Wait until all ten producers have generated their numbers.
    */
    for (index = 0;
         index < PRODUCER_COUNT;
         index++)
    {
        if (waitForSemaphore(
                &producersFinishedGenerating) == -1)
        {
            perror("sem_wait");

            close(pipeFileDescriptors[1]);

            return EXIT_FAILURE;
        }
    }

    pthread_mutex_lock(&printMutex);

    printf("All producer threads have generated "
           "their numbers.\n");

    pthread_mutex_unlock(&printMutex);

    /*
    Notify the child that generation is complete.
    */
    if (kill(childProcessId, SIGUSR1) == -1)
    {
        perror("kill");

        close(pipeFileDescriptors[1]);

        return EXIT_FAILURE;
    }

    /*
    Release all ten producer threads so they can write their
    numbers to the pipe.
    */
    for (index = 0;
         index < PRODUCER_COUNT;
         index++)
    {
        if (sem_post(&producersMayWrite) == -1)
        {
            perror("sem_post");

            close(pipeFileDescriptors[1]);

            return EXIT_FAILURE;
        }
    }

    for (index = 0;
         index < PRODUCER_COUNT;
         index++)
    {
        pthread_join(producers[index], NULL);
    }

    /*
    Closing the write side tells the child that no more data
    will be written.
    */
    close(pipeFileDescriptors[1]);

    if (waitpid(childProcessId,
                &childStatus,
                0) == -1)
    {
        perror("waitpid");

        sem_destroy(&producersFinishedGenerating);
        sem_destroy(&producersMayWrite);

        return EXIT_FAILURE;
    }

    sem_destroy(&producersFinishedGenerating);
    sem_destroy(&producersMayWrite);

    if (WIFEXITED(childStatus))
    {
        return WEXITSTATUS(childStatus);
    }

    return EXIT_FAILURE;
}

/*
The main function:

1. Blocks SIGUSR1.
2. Creates the pipe.
3. Creates the child process.
4. Runs the appropriate parent or child function.
*/
int main(void)
{
    sigset_t signalSet;

    sigemptyset(&signalSet);
    sigaddset(&signalSet, SIGUSR1);

    /*
    Block SIGUSR1 before fork().

    The child will later receive it using sigwait().
    */
    if (sigprocmask(SIG_BLOCK,
                    &signalSet,
                    NULL) == -1)
    {
        perror("sigprocmask");

        return EXIT_FAILURE;
    }

    if (pipe(pipeFileDescriptors) == -1)
    {
        perror("pipe");

        return EXIT_FAILURE;
    }

    childProcessId = fork();

    if (childProcessId == -1)
    {
        perror("fork");

        close(pipeFileDescriptors[0]);
        close(pipeFileDescriptors[1]);

        return EXIT_FAILURE;
    }

    if (childProcessId == 0)
    {
        return runChildProcess(&signalSet);
    }

    return runParentProcess();
}