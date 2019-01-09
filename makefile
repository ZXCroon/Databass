LEX = flex
YACC = bison
CC = g++
CXX = g++
CFLAGS = -std=c++11
CXXFLAGS = -std=c++11
SRCS = $(wildcard **/*.cpp)
SRCS_FOR_TEST = $(wildcard ??/*.cpp) $(wildcard utils/*.cpp) test.cpp
#FS_HEADERS = $(wildcard fs/**/*.h)
FS_HEADERS = fs/fileio/FileManager.h
OBJS = $(SRCS:.cpp=.o)
OBJS_FOR_TEST = $(SRCS_FOR_TEST:.cpp=.o)

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
	$(CXX) -o $@ $^
	rm -rf $(TEST_DIR)
	mkdir $(TEST_DIR)

test: $(OBJS_FOR_TEST)
	$(CXX) -o test $^
	rm -rf $(TEST_DIR)
	mkdir $(TEST_DIR)
	./test

clean:
	rm -f test
	rm -f test.o $(OBJS) $(wildcard parser/*.o)
	rm -f parser/parser.tab.c parser/parser.tab.h parser/lex.yy.c $(TARGET)
