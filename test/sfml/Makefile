CXX = g++
LINK = g++
RM = rm -f
CXXFLAGS = -O3 -g2 -std=c++0x -Wall -Wextra -pedantic -I../../include
LDFLAGS = -g2 -rdynamic -L../../lib -lsfml-window -lsfml-graphics -lSASGL -lm -ldl

OBJECTS = $(patsubst %.cpp,%.o,$(wildcard *.cpp))

.PHONY: all clean

all: test

test: $(OBJECTS)
	$(LINK) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJECTS) test
