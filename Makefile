.PHONY: clean all

CC ?= $(CROSS_COMPILE)gcc

CFLAGS ?= -g -O0 -Wall -Wextra -Wconversion -Wsign-conversion -Werror -pthread
LDFLAGS ?= -lcurl

TARGET = bin/env_monitor
SRC = src/drivers/d_uart.c

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p $(dir $(TARGET))
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -rf bin
	rm -f ./*.o