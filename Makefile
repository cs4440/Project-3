CXX             := g++
DEBUG_LEVEL     := -g
EXTRA_CCFLAGS   := -Wall -Werror=return-type -Wextra -pedantic
CXXFLAGS        := $(DEBUG_LEVEL) $(EXTRA_CCFLAGS)
LDFLAGS         := -lm -lstdc++ -pthread

INC             := include
SRC             := src
PROC            := proc
TESTDIR         := tests
TESTS           := test
UTILS           := utils.o
PARSER          := state_machine.o token.o tokenizer.o parser.o
FS              := entry.o file.o directory.o filesystem.o disk.o
SOCKET          := socket.o
ALL             := basic_client basic_server\
                   dir_listing_client dir_listing_server\
                   disk_server disk_client

# $@ targt name
# $< first prerequisite
# $^ all prerequisites

all: $(ALL)
#	rm *.o

basic_server: basic_server.o
	$(CXX) -o $@ $^ $(LDFLAGS)

basic_server.o: $(PROC)/basic_server.cpp
	$(CXX) $(CXXFLAGS) -c $<

basic_client: basic_client.o
	$(CXX) -o $@ $^

basic_client.o: $(PROC)/basic_client.cpp
	$(CXX) $(CXXFLAGS) -c $<

dir_listing_server: dir_listing_server.o
	$(CXX) -o $@ $^ $(LDFLAGS)

dir_listing_server.o: $(PROC)/dir_listing_server.cpp
	$(CXX) $(CXXFLAGS) -c $<

dir_listing_client: dir_listing_client.o
	$(CXX) -o $@ $^

dir_listing_client.o: $(PROC)/dir_listing_client.cpp
	$(CXX) $(CXXFLAGS) -c $<

# DISK SERVER

disk_server: disk_server.o $(SOCKET) $(FS) $(PARSER) 
	$(CXX) -o $@ $^ $(LDFLAGS)

disk_server.o: $(PROC)/disk_server.cpp
	$(CXX) $(CXXFLAGS) -c $<

disk_client: disk_client.o $(SOCKET)
	$(CXX) -o $@ $^ $(LDFLAGS)

disk_client.o: $(PROC)/disk_client.cpp
	$(CXX) $(CXXFLAGS) -c $<

# UTILS
utils.o: ${SRC}/utils.cpp\
	${INC}/utils.h
	$(CXX) $(CXXFLAGS) -c $<

# SOCKET
socket.o: ${SRC}/socket.cpp\
	${INC}/socket.h
	$(CXX) $(CXXFLAGS) -c $<

# PARSER
state_machine.o: ${SRC}/state_machine.cpp\
	${INC}/state_machine.h
	$(CXX) $(CXXFLAGS) -c $<

token.o: ${SRC}/token.cpp\
	${INC}/token.h
	$(CXX) $(CXXFLAGS) -c $<

tokenizer.o: ${SRC}/tokenizer.cpp\
	${INC}/tokenizer.h
	$(CXX) $(CXXFLAGS) -c $<

parser.o: ${SRC}/parser.cpp\
	${INC}/parser.h
	$(CXX) $(CXXFLAGS) -c $<

# FILE SYSTEM
disk.o: ${SRC}/disk.cpp\
	${INC}/disk.h
	$(CXX) $(CXXFLAGS) -c $<

entry.o: ${SRC}/entry.cpp\
	${INC}/entry.h
	$(CXX) $(CXXFLAGS) -c $<

file.o: ${SRC}/file.cpp\
	${INC}/file.h
	$(CXX) $(CXXFLAGS) -c $<

directory.o: ${SRC}/directory.cpp\
	${INC}//directory.h
	$(CXX) $(CXXFLAGS) -c $<

filesystem.o: ${SRC}/filesystem.cpp\
	${INC}/filesystem.h
	$(CXX) $(CXXFLAGS) -c $<

# TESTS
tests: $(TESTS)

test: test.o $(FS)
	$(CXX) -o $@ $^

test.o: $(TESTDIR)/test.cpp
	$(CXX) $(CXXFLAGS) -c $<

.PHONY: clean

clean:
	rm -f *.o *.out $(ALL) $(TESTS)
