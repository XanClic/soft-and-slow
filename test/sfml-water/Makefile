CXX = g++
LINK = g++
RM = rm -f
CXXFLAGS = -O3 -g2 -std=c++0x -Wall -Wextra -pedantic -I../../include
LDSASFLAGS = -g2 -rdynamic -L../../lib -lsfml-graphics -lSDL -lSASGL -lm -ldl
LDOGLFLAGS = -g2 -rdynamic -L../../lib -lsfml-window -lsfml-graphics -lGL -lGLU -lm -ldl

SASOBJECTS = $(patsubst %.cpp,%-sas.o,$(wildcard *.cpp))
OGLOBJECTS = $(patsubst %.cpp,%-ogl.o,$(wildcard *.cpp))

.PHONY: all clean

all: test-sasgl test-opengl

test-sasgl: $(SASOBJECTS)
	$(LINK) $^ -o $@ $(LDSASFLAGS)

test-opengl: $(OGLOBJECTS)
	$(LINK) $^ -o $@ $(LDOGLFLAGS)

%-sas.o: %.cpp
	$(CXX) $(CXXFLAGS) -DSAS -c $< -o $@

%-ogl.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(SASOBJECTS) $(OGLOBJECTS) test-sasgl test-opengl color.ppm depth.pgm
