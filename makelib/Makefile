.PHONY:clean test
CC=g++
CFLAGS=-Wall -g
BINALL=makepagelib.exe page_unique.exe make_inverted_index.exe query.exe run_query.exe
STAND= -std=c++0x
INC=-I ~/mycode/muduo/build/release-install/include
LINKS=-lmuduo_net -lmuduo_base -lpthread -ljsoncpp -L ~/mycode/muduo/build/release-install/lib
BIN=$@
all:$(BINALL)
makepagelib.exe:makelib.o MakePageLib.o RobustIO.o ReadFile.o TruncFile.o \
	gbk2utf_8.o
	$(CC) $(CFLAGS) $^ -o $(BIN) $(LINKS)
page_unique.exe:unique_page.o RobustIO.o ReadFile.o TruncFile.o \
	Document.o PageUnique.o SegmentSingleton.o InvertedIndex.o
	$(CC) $(CFLAGS) $^ -o $(BIN) $(LINKS)
make_inverted_index.exe:make_inverted_index.o RobustIO.o ReadFile.o \
	TruncFile.o Document.o SegmentSingleton.o InvertedIndex.o \
	MakeInvertedIndex.o
	$(CC) $(CFLAGS) $^ -o $(BIN) $(LINKS)
query.exe:make_query.o RobustIO.o ReadFile.o TruncFile.o \
	Document.o SegmentSingleton.o InvertedIndex.o PageQuery.o
	$(CC) $(CFLAGS) $^ -o $(BIN) $(LINKS)
run_query.exe:run_server.o RobustIO.o ReadFile.o TruncFile.o \
	Document.o SegmentSingleton.o InvertedIndex.o PageQuery.o \
	QueryServer.o
	$(CC) $(CFLAGS) $^ -o $(BIN) $(LINKS)
%.o:%.cpp
	$(CC) $(CFLAGS) -c $< -o $@ $(STAND) $(INC)
clean:
	rm -f *.o $(BIN) $(TEST) core
