/*
Compile: gcc -o string_sort string_sort.c

Run: ./string_sort
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* This function reads strings from the user and stores them in the array. */
void readStrings(char **arr, int size)
{
    int i;
    char temp[100];

    for (i = 0; i < size; i++)
    {
        printf("Enter string %d of array: ", i + 1);
        scanf("%99s", temp);

        arr[i] = (char *)malloc((strlen(temp) + 1) * sizeof(char));
        strcpy(arr[i], temp);
    }
}

/* This function displays all strings in the array. */
void displayStrings(char **arr, int size)
{
    int i;

    printf("[");

    for (i = 0; i < size; i++)
    {
        printf("%s", arr[i]);

        if (i < size - 1)
        {
            printf(", ");
        }
    }

    printf("]\n");
}

/* This function sorts the strings. */
void sortStrings(char **arr, int size)
{
    int i;
    int currLoc;
    char *temp;

    for (i = 1; i < size; i++)
    {
        currLoc = i;

        while (currLoc > 0 && strcmp(arr[currLoc - 1], arr[currLoc]) > 0)
        {
            temp = arr[currLoc];
            arr[currLoc] = arr[currLoc - 1];
            arr[currLoc - 1] = temp;

            currLoc--;
        }
    }
}

/* The main function reads, sorts, and displays the strings. */
int main()
{
    int N;
    int i;
    char **arr;

    printf("Enter number of elements in array: ");
    scanf("%d", &N);

    arr = (char **)malloc(N * sizeof(char *));

    readStrings(arr, N);

    printf("Given array is: ");
    displayStrings(arr, N);

    sortStrings(arr, N);

    printf("Sorted array is: ");
    displayStrings(arr, N);

    for (i = 0; i < N; i++)
    {
        free(arr[i]);
    }

    free(arr);

    return 0;
}