/*
Compile: gcc -o prime prime.c

Run: ./prime
*/

#include <stdio.h>

int main(){
    int given_number;
    int i;

    printf("Enter a number: ");
    scanf("%d", &given_number);

    if (given_number <= 1)
    {
        printf("The number is not prime\n");
    }
    else
    {
        for (i = 2; i < given_number; i++)
        {
            if (given_number % i == 0)
            {
                printf("The number is not prime\n");
                return 0;
            }
        }

        printf("The number is prime\n");
    }

    return 0;
}