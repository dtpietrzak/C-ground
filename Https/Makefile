# Makefile

# Define the compiler and flags
CC = gcc
CFLAGS = -I/opt/homebrew/opt/openssl/include -Wall -Wextra -pedantic -std=c17
LDFLAGS = -L/opt/homebrew/opt/openssl/lib -lssl -lcrypto

# Define the target executable
TARGET = main
SRCS = main.c
OBJS = $(SRCS:.c=.o)

# Build rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)