CC      = gcc
CFLAGS  = -g -std=c17 -D_POSIX_C_SOURCE=200809L -pthread \
          -Wall -Werror -Wextra -Wcast-align -Wconversion -Wfloat-equal \
          -Wformat=2 -Wnull-dereference -Wshadow -Wsign-conversion \
          -Wswitch-enum -Wundef -Wunreachable-code -Wunused \
          -fsanitize=undefined -fmax-errors=5
LDFLAGS = -pthread -fsanitize=undefined
LDLIBS  = -lm

TARGET  = cloudIST
OBJS    = main.o parser.o datacenter.o datacenter_utils.o resources.o \
          filesystem.o auxiliate_conf.o
HEADERS = auxiliate_conf.h constants.h datacenter.h datacenter_utils.h \
          filesystem.h parser.h resources.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean