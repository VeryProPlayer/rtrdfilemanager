# Compiler
CC = gcc
CFLAGS = -Wall -lncurses

# the name of the target lmao
TARGET = rtrdfm

# Source file
SRC = file.c

# Default target
all: $(TARGET)

# Build rule
$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS)

# Clean compiled files
clean:
	rm -f $(TARGET)
