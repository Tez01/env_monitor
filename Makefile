.PHONY: clean all

CC ?= $(CROSS_COMPILE)gcc
# CFLAGS ?= -Wall -Wextra -Wconversion -Wsign-conversion -Werror -pthread
CFLAGS ?= -g -O0 -Wall -Wextra -Wconversion -Wsign-conversion -Werror -pthread
LDFLAGS ?=

override CFLAGS += 

TARGET = env_monitor
SRC = src/drivers/d_uart.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) ./*.o



    