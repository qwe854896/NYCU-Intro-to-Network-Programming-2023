CXX		= g++
PROGS	= server client
UID		= $(shell stat -c "%u:%g" /build/Makefile 2>/dev/null || stat -c "%u:%g" ./Makefile 2> /dev/null)

all: $(PROGS)

%.o: %.cpp
	$(CXX) -c -g -Wall $<

server: server.o
	$(CXX) -o $@ -static $^
	chown -R "$(UID)" *.o $@

client: client.o
	$(CXX) -o $@ -static $^
	chown -R "$(UID)" *.o $@

clean:
	rm -f *.o $(PROGS)
