CC = g++
SRCS = $(wildcard **/*.cpp)
#FS_HEADERS = $(wildcard fs/**/*.h)
FS_HEADERS = fs/fileio/FileManager.h
OBJS = $(SRCS:.cpp=.o)

TEST_DIR = test_dbfiles

all: $(OBJS)

test: test.o $(OBJS)
	$(CC) -o test $^
	rm -rf $(TEST_DIR)
	mkdir $(TEST_DIR)
	./test

%.o: %.c %.h $(FS_HEADERS)
	$(CC) -c -o $@ $^

clean:
	rm -f test
	rm -f test.o $(OBJS)
