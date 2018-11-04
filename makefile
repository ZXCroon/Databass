CC = g++
SRCS = $(wildcard **/*.cpp)
OBJS = $(SRCS:.cpp=.o)

all: $(OBJS)

%.o: %.c
	$(CC) -o $@ $^

clean:
	rm -f $(OBJS)
