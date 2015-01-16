# compiler to use
COMP = clang++
FLAGS = -std=c++11 -stdlib=libc++ -Wall
BIN = batchtool

all: batchtool.o image.o pugixml.o lodepng.o
	$(COMP) $(FLAGS) batchtool.o image.o pugixml.o lodepng.o -o $(BIN)

batchtool.o: src/batchtool.cpp
	$(COMP) $(FLAGS) -c src/batchtool.cpp 

image.o: src/image.cpp
	$(COMP) $(FLAGS) -c src/image.cpp 

pugixml.o: lib/pugixml/pugixml.cpp
	$(COMP) $(FLAGS) -c lib/pugixml/pugixml.cpp

lodepng.o: lib/lodepng/lodepng.cpp
	$(COMP) $(FLAGS) -c lib/lodepng/lodepng.cpp

clean:
	rm -rf *o batchtool
