CS 532 Lab 6
Standard I/O Streams and CSV Sorting

Name: Toyosi Ogundeyi
BlazerId: Ogundeyi

Files Submitted:
listing.c
README.txt

Required Input File:
listings.csv

Compilation:
gcc -o listing listing.c

Execution:
./listing

The files listing.c and listings.csv must be in the same directory before
the program is executed.

Program Description:
The program opens listings.csv in read mode and reads each record with
fgets(). It uses strtok(), atoi(), atof(), and strdup() to parse the
13 comma-separated fields and store each record in a struct listing.

The program displays the listing structures on standard output. It then
uses the C library qsort() function to sort the complete structures in
two different ways:

1. Alphabetically by host_name
2. Numerically by price, from lowest to highest

Output Files:
listings_sorted_by_host_name.csv
listings_sorted_by_price.csv

Both output files retain the original CSV heading row and contain all
13 fields from each listing.

Testing:
The program was tested by compiling with:
gcc -Wall -Wextra -pedantic -std=c11 -o listing listing.c

The following items were tested:
- Opening and reading listings.csv
- Parsing all 13 CSV fields
- Storing records in an array of structures
- Displaying the structures
- Sorting complete structures by host_name
- Sorting complete structures by price
- Writing both sorted CSV output files
- Closing all input and output streams
- Releasing dynamically allocated strings
- Handling an unavailable or empty input file

Notes:
The program reads the header row separately and processes all remaining
rows as listing data. The array size is sufficient to store all records
contained in the provided listings.csv file. When two listings have the
same host name or price, the listing ID is used to produce a consistent
ordering.