# compiler to use
COMP = clang++
FLAGS = -std=c++11 -stdlib=libc++ -Wall
BIN = batchtool


#all:
#	clang++ -std=c++11 -stdlib=libc++ batchtool.cpp lib/lodepng/lodepng.cpp lib/pugixml/pugixml.cpp -o batchtool

all: batchtool.o pugixml.o lodepng.o
	$(COMP) $(FLAGS) batchtool.o pugixml.o lodepng.o -o $(BIN)

batchtool.o: batchtool.cpp
	$(COMP) $(FLAGS) -c batchtool.cpp 

pugixml.o: lib/pugixml/pugixml.cpp
	$(COMP) $(FLAGS) -c lib/pugixml/pugixml.cpp

lodepng.o: lib/lodepng/lodepng.cpp
	$(COMP) $(FLAGS) -c lib/lodepng/lodepng.cpp

clean:
	rm -rf *o batchtool
