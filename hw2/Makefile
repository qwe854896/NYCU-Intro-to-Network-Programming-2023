CXX       := g++
CXXFLAGS  := -g -Wall -std=c++17
RM        := rm -f
SRCDIR    := src
INCDIR    := include
BINDIR    := bin
PROGS     := hw2_chat_server
SRCS      := $(wildcard $(SRCDIR)/*.cpp)
OBJS      := $(patsubst $(SRCDIR)/%.cpp,$(BINDIR)/%.o,$(SRCS)) $(BINDIR)/hw2_chat_server.o

.PHONY: all test clean

all: $(PROGS)

$(BINDIR)/%.o: $(SRCDIR)/%.cpp $(INCDIR)/%.h
	$(CXX) -c $(CXXFLAGS) -I$(INCDIR) -o $@ $<

$(BINDIR)/hw2_chat_server.o: hw2_chat_server.cpp
	$(CXX) -c $(CXXFLAGS) -I$(INCDIR) -o $@ $<

hw2_chat_server: $(OBJS)
	$(CXX) -o $@ $^

test:
	./test/examples.py

clean:
	$(RM) $(BINDIR)/*.o $(PROGS)
