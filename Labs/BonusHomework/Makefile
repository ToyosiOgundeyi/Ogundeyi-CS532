CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -pthread
TARGET = producer_consumer

all: $(TARGET)

$(TARGET): producer_consumer.c
	$(CC) $(CFLAGS) -o $(TARGET) producer_consumer.c

clean:
	rm -f $(TARGET) average.txt