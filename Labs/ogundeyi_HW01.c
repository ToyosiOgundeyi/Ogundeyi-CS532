/*
Compile: gcc -o ogundeyi_HW01 ogundeyi_HW01.c

Run: ./ogundeyi_HW01
*/

#include <stdio.h>

/* This function returns the sum of the digits of a positive integer and returns -1 if the number is less than or equal to 0. */
int sumOfDigits(int n)
{
    int sum = 0;

    if (n <= 0)
    {
        return -1;
    }

    while (n > 0)
    {
        sum = sum + (n % 10);
        n = n / 10;
    }

    return sum;
}

/* This function returns the difference between the maximum and minimum values in an integer array. */
int UABMaxMinDiff(int arr[], int size)
{
    int i;
    int max = arr[0];
    int min = arr[0];

    for (i = 1; i < size; i++)
    {
        if (arr[i] > max)
        {
            max = arr[i];
        }

        if (arr[i] < min)
        {
            min = arr[i];
        }
    }

    return max - min;
}

/* This function creates a new array where all even numbers are replaced with 0. */
void replaceEvenWithZero(int arr[], int newArr[], int size)
{
    int i;

    for (i = 0; i < size; i++)
    {
        if (arr[i] % 2 == 0)
        {
            newArr[i] = 0;
        }
        else
        {
            newArr[i] = arr[i];
        }
    }
}

/* This function returns 1 if the number is a perfect square and returns 0 otherwise. */
int perfectSquare(int n)
{
    int i;

    if (n < 0)
    {
        return 0;
    }

    for (i = 0; i * i <= n; i++)
    {
        if (i * i == n)
        {
            return 1;
        }
    }

    return 0;
}

/* This function counts and returns the number of vowels in a string, ignoring letter case. */
int countVowels(char s[])
{
    int i = 0;
    int count = 0;

    while (s[i] != '\0')
    {
        if (s[i] == 'a' || s[i] == 'A' ||
            s[i] == 'e' || s[i] == 'E' ||
            s[i] == 'i' || s[i] == 'I' ||
            s[i] == 'o' || s[i] == 'O' ||
            s[i] == 'u' || s[i] == 'U')
        {
            count++;
        }

        i++;
    }

    return count;
}

/* This helper function prints the contents of an integer array in bracket format. */
void printArray(int arr[], int size)
{
    int i;

    printf("[");

    for (i = 0; i < size; i++)
    {
        printf("%d", arr[i]);

        if (i < size - 1)
        {
            printf(", ");
        }
    }

    printf("]");
}

/* The main function tests all five functions using the sample inputs provided. */
int main()
{
    int arr1[] = {3, 7, 2, 9};
    int arr2[] = {5, 5, 5, 5, 5, 5};
    int arr3[] = {-2, 4, -1, 6, 5};

    int evenArr1[] = {1, 2, 3, 4};
    int evenArr2[] = {2, 4, 6};
    int evenArr3[] = {1, 3, 5};

    int result1[4];
    int result2[3];
    int result3[3];

    printf("sumOfDigits(123) = %d\n", sumOfDigits(123));
    printf("sumOfDigits(405) = %d\n", sumOfDigits(405));
    printf("sumOfDigits(0) = %d\n", sumOfDigits(0));
    printf("sumOfDigits(7) = %d\n", sumOfDigits(7));
    printf("sumOfDigits(-308) = %d\n\n", sumOfDigits(-308));

    printf("UABMaxMinDiff([3, 7, 2, 9]) = %d\n", UABMaxMinDiff(arr1, 4));
    printf("UABMaxMinDiff([5, 5, 5, 5, 5, 5]) = %d\n", UABMaxMinDiff(arr2, 6));
    printf("UABMaxMinDiff([-2, 4, -1, 6, 5]) = %d\n\n", UABMaxMinDiff(arr3, 5));

    replaceEvenWithZero(evenArr1, result1, 4);
    replaceEvenWithZero(evenArr2, result2, 3);
    replaceEvenWithZero(evenArr3, result3, 3);

    printf("replaceEvenWithZero([1, 2, 3, 4]) = ");
    printArray(result1, 4);
    printf("\n");

    printf("replaceEvenWithZero([2, 4, 6]) = ");
    printArray(result2, 3);
    printf("\n");

    printf("replaceEvenWithZero([1, 3, 5]) = ");
    printArray(result3, 3);
    printf("\n\n");

    printf("perfectSquare(16) = %s\n", perfectSquare(16) ? "True" : "False");
    printf("perfectSquare(15) = %s\n", perfectSquare(15) ? "True" : "False");
    printf("perfectSquare(25) = %s\n", perfectSquare(25) ? "True" : "False");
    printf("perfectSquare(36) = %s\n\n", perfectSquare(36) ? "True" : "False");

    printf("countVowels(\"Hello World\") = %d\n", countVowels("Hello World"));
    printf("countVowels(\"UAB CS\") = %d\n", countVowels("UAB CS"));
    printf("countVowels(\"Python\") = %d\n", countVowels("Python"));
    printf("countVowels(\"aeiou\") = %d\n", countVowels("aeiou"));

    return 0;
}
