CC = gcc
CFLAGS = -Wall -Wextra -std=c99

SRCS = shell.c
OBJS = $(SRCS:.c=.o)
TARGET = shell

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

