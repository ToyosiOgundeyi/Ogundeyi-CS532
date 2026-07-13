/*
Name: Toyosi Ogundeyi
BlazerId: Ogundeyi
Lab #: 6
To compile: gcc -o listing listing.c
To run: ./listing
*/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINESIZE 2048
#define MAX_LISTINGS 22555

struct listing
{
    int id;
    int host_id;
    int minimum_nights;
    int number_of_reviews;
    int calculated_host_listings_count;
    int availability_365;

    char *host_name;
    char *neighbourhood_group;
    char *neighbourhood;
    char *room_type;

    float latitude;
    float longitude;
    float price;
};

char *duplicateString(const char *text)
{
    char *copy;

    copy = strdup(text);

    if (copy == NULL)
    {
        fprintf(stderr, "Error: unable to allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    return copy;
}

struct listing getfields(char *line)
{
    struct listing item;
    char *token;

    token = strtok(line, ",");
    item.id = atoi(token);

    token = strtok(NULL, ",");
    item.host_id = atoi(token);

    token = strtok(NULL, ",");
    item.host_name = duplicateString(token);

    token = strtok(NULL, ",");
    item.neighbourhood_group = duplicateString(token);

    token = strtok(NULL, ",");
    item.neighbourhood = duplicateString(token);

    token = strtok(NULL, ",");
    item.latitude = (float)atof(token);

    token = strtok(NULL, ",");
    item.longitude = (float)atof(token);

    token = strtok(NULL, ",");
    item.room_type = duplicateString(token);

    token = strtok(NULL, ",");
    item.price = (float)atof(token);

    token = strtok(NULL, ",");
    item.minimum_nights = atoi(token);

    token = strtok(NULL, ",");
    item.number_of_reviews = atoi(token);

    token = strtok(NULL, ",");
    item.calculated_host_listings_count = atoi(token);

    token = strtok(NULL, ",\r\n");
    item.availability_365 = atoi(token);

    return item;
}

void displayStruct(struct listing item)
{
    printf("ID : %d\n", item.id);
    printf("Host ID : %d\n", item.host_id);
    printf("Host Name : %s\n", item.host_name);
    printf("Neighbourhood Group : %s\n", item.neighbourhood_group);
    printf("Neighbourhood : %s\n", item.neighbourhood);
    printf("Latitude : %f\n", item.latitude);
    printf("Longitude : %f\n", item.longitude);
    printf("Room Type : %s\n", item.room_type);
    printf("Price : %.2f\n", item.price);
    printf("Minimum Nights : %d\n", item.minimum_nights);
    printf("Number of Reviews : %d\n", item.number_of_reviews);
    printf("Calculated Host Listings Count : %d\n",
           item.calculated_host_listings_count);
    printf("Availability 365 : %d\n\n", item.availability_365);
}

int compareHostName(const void *first, const void *second)
{
    const struct listing *item1;
    const struct listing *item2;
    int comparison;

    item1 = (const struct listing *)first;
    item2 = (const struct listing *)second;

    comparison = strcmp(item1->host_name, item2->host_name);

    if (comparison == 0)
    {
        if (item1->id < item2->id)
        {
            return -1;
        }

        if (item1->id > item2->id)
        {
            return 1;
        }
    }

    return comparison;
}

int comparePrice(const void *first, const void *second)
{
    const struct listing *item1;
    const struct listing *item2;

    item1 = (const struct listing *)first;
    item2 = (const struct listing *)second;

    if (item1->price < item2->price)
    {
        return -1;
    }

    if (item1->price > item2->price)
    {
        return 1;
    }

    if (item1->id < item2->id)
    {
        return -1;
    }

    if (item1->id > item2->id)
    {
        return 1;
    }

    return 0;
}

int writeListings(const char *filename,
                  const char *header,
                  struct listing list_items[],
                  int count)
{
    FILE *outputFile;
    int i;

    outputFile = fopen(filename, "w");

    if (outputFile == NULL)
    {
        fprintf(stderr, "Error opening output file %s\n", filename);
        return 0;
    }

    fputs(header, outputFile);

    if (header[0] != '\0' &&
        header[strlen(header) - 1] != '\n')
    {
        fputc('\n', outputFile);
    }

    for (i = 0; i < count; i++)
    {
        fprintf(outputFile,
                "%d,%d,%s,%s,%s,%.7g,%.7g,%s,%.2f,%d,%d,%d,%d\n",
                list_items[i].id,
                list_items[i].host_id,
                list_items[i].host_name,
                list_items[i].neighbourhood_group,
                list_items[i].neighbourhood,
                list_items[i].latitude,
                list_items[i].longitude,
                list_items[i].room_type,
                list_items[i].price,
                list_items[i].minimum_nights,
                list_items[i].number_of_reviews,
                list_items[i].calculated_host_listings_count,
                list_items[i].availability_365);
    }

    fclose(outputFile);
    return 1;
}

void freeListings(struct listing list_items[], int count)
{
    int i;

    for (i = 0; i < count; i++)
    {
        free(list_items[i].host_name);
        free(list_items[i].neighbourhood_group);
        free(list_items[i].neighbourhood);
        free(list_items[i].room_type);
    }
}

int main(void)
{
    struct listing list_items[MAX_LISTINGS];
    char line[LINESIZE];
    char header[LINESIZE];
    int i;
    int count;
    FILE *fptr;

    fptr = fopen("listings.csv", "r");

    if (fptr == NULL)
    {
        fprintf(stderr, "Error reading input file listings.csv\n");
        return EXIT_FAILURE;
    }

    if (fgets(header, LINESIZE, fptr) == NULL)
    {
        fprintf(stderr, "Error: listings.csv is empty.\n");
        fclose(fptr);
        return EXIT_FAILURE;
    }

    count = 0;

    while (fgets(line, LINESIZE, fptr) != NULL)
    {
        if (count >= MAX_LISTINGS)
        {
            fprintf(stderr,
                    "Error: listings.csv contains more than %d records.\n",
                    MAX_LISTINGS);
            fclose(fptr);
            freeListings(list_items, count);
            return EXIT_FAILURE;
        }

        list_items[count] = getfields(line);
        count++;
    }

    fclose(fptr);

    for (i = 0; i < count; i++)
    {
        displayStruct(list_items[i]);
    }

    qsort(list_items,
          (size_t)count,
          sizeof(struct listing),
          compareHostName);

    if (!writeListings("listings_sorted_by_host_name.csv",
                       header,
                       list_items,
                       count))
    {
        freeListings(list_items, count);
        return EXIT_FAILURE;
    }

    qsort(list_items,
          (size_t)count,
          sizeof(struct listing),
          comparePrice);

    if (!writeListings("listings_sorted_by_price.csv",
                       header,
                       list_items,
                       count))
    {
        freeListings(list_items, count);
        return EXIT_FAILURE;
    }

    printf("Successfully processed %d listings.\n", count);
    printf("Created listings_sorted_by_host_name.csv\n");
    printf("Created listings_sorted_by_price.csv\n");

    freeListings(list_items, count);

    return EXIT_SUCCESS;
}