CXX     = g++
PROGS   = server
UID     = $(shell stat -c "%u:%g" /build/Makefile 2>/dev/null || stat -c "%u:%g" ./Makefile 2> /dev/null)

CXXFLAGS = -std=c++17

all: server

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -g -Wall $<

server: server.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -static -lssl -lcrypto
	chown -R "$(UID)" *.o server

clean:
	rm -f *.o $(PROGS)
