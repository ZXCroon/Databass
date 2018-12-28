LEX = flex
YACC = bison
CC = g++ -std=c++11
SRCS = $(wildcard **/*.cpp)
#SRCS = $(wildcard ??/*.cpp) $(wildcard utils/*.cpp)
#FS_HEADERS = $(wildcard fs/**/*.h)
FS_HEADERS = fs/fileio/FileManager.h
OBJS = $(SRCS:.cpp=.o)

TEST_DIR = test_dbfiles
LEXTARGET = parser/lex.yy.c
YACCTARGET = parser/parser.tab.c
TARGET = console

all: $(LEXTARGET) $(YACCTARGET) $(OBJS) parser/lex.yy.o parser/parser.tab.o $(TARGET)

$(LEXTARGET): parser/parser.lex
	$(LEX) -o $@ $^

$(YACCTARGET): parser/parser.y
	$(YACC) -d -o $@ $^

$(TARGET): $(LEXTARGET) $(YACCTARGET) $(OBJS)
	$(CC) -o $@ $^
	rm -rf $(TEST_DIR)
	mkdir $(TEST_DIR)

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
	rm -f parser/parser.tab.c parser/parser.tab.h parser/lex.yy.c $(TARGET)
