CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wno-unused-value
TARGET = example
SRC = example.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
