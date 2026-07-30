.PHONY: clean all

CC ?= $(CROSS_COMPILE)gcc
# CFLAGS ?= -Wall -Wextra -Wconversion -Wsign-conversion -Werror -pthread
CFLAGS ?= -g -O0 -Wall -Wextra -Wconversion -Wsign-conversion -Werror -pthread
LDFLAGS ?=

TARGET = env_monitor

override CFLAGS += 

all: $(TARGET)

$(TARGET): env_monitor.c
	$(CC) $(CFLAGS) $(LDFLAGS) ./env_monitor.c -o $(TARGET)
clean:
	rm -f $(TARGET) ./*.o



    