.PHONY: clean all

CC  ?= $(CROSS_COMPILE)gcc
CXX ?= $(CROSS_COMPILE)g++

HEADER_PATHS = -Isrc/app -Isrc/drivers -Isrc/middleware -Isrc/utility

CFLAGS   ?= -g -O0 -Wall -Wextra -Wconversion -Wsign-conversion -Werror -pthread $(HEADER_PATHS)
CXXFLAGS ?= -std=c++23 -g -O0 -Wall -Wextra -Wconversion -Wsign-conversion -Werror -pthread $(HEADER_PATHS)

LDFLAGS ?= -lcurl

TARGET = bin/mold_detector

C_SRC = 
CPP_SRC =  	src/app/main.cpp \
			src/utility/u_syslog.cpp

C_OBJ = $(C_SRC:.c=.o)
CPP_OBJ = $(CPP_SRC:.cpp=.o)

OBJ = $(C_OBJ) $(CPP_OBJ)

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p $(dir $(TARGET))
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf bin
	rm -f $(OBJ)