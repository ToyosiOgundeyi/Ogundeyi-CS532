/*
Compile: gcc -o insertionsort insertionsort.c

Run: ./insertionsort
*/

#include <stdio.h>

int main()
{
    int N;
    int i;
    int temp;
    int currLoc;

    printf("Please enter number of elements in array: ");
    scanf("%d", &N);

    int arr[N];

    for (i = 0; i < N; i++)
    {
        printf("Please enter element %d of array: ", i + 1);
        scanf("%d", &arr[i]);
    }

    /* Display the original array */
    printf("Given array is: [");

    for (i = 0; i < N - 1; i++)
    {
        printf("%d, ", arr[i]);
    }

    printf("%d]\n", arr[N - 1]);

    /* Insertion Sort */
    for (i = 1; i < N; i++)
    {
        currLoc = i;

        while (currLoc > 0 && arr[currLoc - 1] > arr[currLoc])
        {
            temp = arr[currLoc];
            arr[currLoc] = arr[currLoc - 1];
            arr[currLoc - 1] = temp;

            currLoc--;
        }
    }

    /* Display the sorted array */
    printf("Sorted array is: [");

    for (i = 0; i < N - 1; i++)
    {
        printf("%d, ", arr[i]);
    }

    printf("%d]\n", arr[N - 1]);

    return 0;
}