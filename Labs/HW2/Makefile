CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11
TARGET = search
OBJECTS = search.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

search.o: search.c
	$(CC) $(CFLAGS) -c search.c

clean:
	rm -f $(TARGET) $(OBJECTS)