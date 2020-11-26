CXX             := g++
DEBUG_LEVEL     := -g
EXTRA_CCFLAGS   := -Wall -Werror=return-type -Wextra -pedantic
OPT             := -O0
CXXFLAGS        := $(DEBUG_LEVEL) $(EXTRA_CCFLAGS) $(OPT)
LDLIBS          := -lm -lstdc++ -pthread

INC             := include
SRC             := src
PROC            := proc
TESTDIR         := tests
TESTS           := test
PARSER          := state_machine.o token.o tokenizer.o parser.o
DISK            := disk.o
FS              := $(DISK) fat.o
SOCKET          := socket.o
BASIC_SERVER    := basic_client basic_server
DIR_LISTING     := dir_listing_client dir_listing_server
DISK_SERVER     := $(PARSER) $(SOCKET) $(DISK)\
                   disk_client disk_client_rand disk_server
FS_BASIC        := $(PARSER) $(SOCKET) $(FS) fs_basic_client\
                   fs_basic_server
FS_FULL         := $(PARSER) $(SOCKET) $(FS) fs_full_client\
                   fs_full_server
ALL             := $(BASIC_SERVER) $(DIR_LISTING) $(DISK_SERVER) $(FS_BASIC)\
                   $(FS_FULL)
                   

# $@ targt name
# $< first prerequisite
# $^ all prerequisites

all: $(ALL)
#	rm *.o

# BASIC SERVER

basic-server: $(BASIC_SERVER)

basic_server: basic_server.o
	$(CXX) -o $@ $^ $(LDLIBS)

basic_server.o: $(PROC)/basic_server.cpp
	$(CXX) $(CXXFLAGS) -c $<

basic_client: basic_client.o
	$(CXX) -o $@ $^

basic_client.o: $(PROC)/basic_client.cpp
	$(CXX) $(CXXFLAGS) -c $<

dir-listing: $(DIR_LISTING)

dir_listing_server: dir_listing_server.o
	$(CXX) -o $@ $^ $(LDLIBS)

dir_listing_server.o: $(PROC)/dir_listing_server.cpp
	$(CXX) $(CXXFLAGS) -c $<

dir_listing_client: dir_listing_client.o
	$(CXX) -o $@ $^

dir_listing_client.o: $(PROC)/dir_listing_client.cpp
	$(CXX) $(CXXFLAGS) -c $<

# DISK CLIENT/SERVER

disk-server: $(DISK_SERVER)

disk_client: disk_client.o $(SOCKET)
	$(CXX) -o $@ $^ $(LDLIBS)

disk_client.o: $(PROC)/disk_client.cpp
	$(CXX) $(CXXFLAGS) -c $<

disk_client_rand: disk_client_rand.o $(SOCKET)
	$(CXX) -o $@ $^ $(LDLIBS)

disk_client_rand.o: $(PROC)/disk_client_rand.cpp
	$(CXX) $(CXXFLAGS) -c $<

disk_server: disk_server.o $(PARSER) $(DISK) $(SOCKET)
	$(CXX) -o $@ $^ $(LDLIBS)

disk_server.o: $(PROC)/disk_server.cpp
	$(CXX) $(CXXFLAGS) -c $<

# BASIC FILESYSTEM CLIENT/SERVER

fs-basic: $(FS_BASIC)

fs_basic_client: fs_basic_client.o $(SOCKET)
	$(CXX) -o $@ $^ $(LDLIBS)

fs_basic_client.o: $(PROC)/fs_basic_client.cpp
	$(CXX) $(CXXFLAGS) -c $<

fs_basic_server: fs_basic_server.o  $(PARSER) $(FS) $(SOCKET)
	$(CXX) -o $@ $^ $(LDLIBS)

fs_basic_server.o: $(PROC)/fs_basic_server.cpp
	$(CXX) $(CXXFLAGS) -c $<

# FULL FILESYSTEM CLIENT/SERVER

fs-full: $(FS_FULL)

fs_full_client: fs_full_client.o $(SOCKET)
	$(CXX) -o $@ $^ $(LDLIBS)

fs_full_client.o: $(PROC)/fs_full_client.cpp\
	${INC}/ansi_style.h
	$(CXX) $(CXXFLAGS) -c $<

fs_full_server: fs_full_server.o  $(PARSER) $(FS) $(SOCKET)
	$(CXX) -o $@ $^ $(LDLIBS)

fs_full_server.o: $(PROC)/fs_full_server.cpp
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

# DISK
disk.o: ${SRC}/disk.cpp\
	${INC}/disk.h
	$(CXX) $(CXXFLAGS) -c $<

# FILESYSTEM
fat.o: ${SRC}/fat.cpp\
	${INC}/fat.h\
	${INC}/ansi_style.h
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
