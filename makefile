# compiler to use
COMP = g++
FLAGS = -std=gnu++11
BIN = bin/batchtool

all: batchtool.o image.o pugixml.o lodepng.o xmlwriter.o jsonwriter.o base64.o
	$(COMP) $(FLAGS) batchtool.o xmlwriter.o jsonwriter.o base64.o image.o pugixml.o lodepng.o -o $(BIN)

batchtool.o: src/batchtool.cpp
	$(COMP) $(FLAGS) -c src/batchtool.cpp 

xmlwriter.o: src/xmlwriter.cpp
	$(COMP) $(FLAGS) -c src/xmlwriter.cpp

jsonwriter.o: src/jsonwriter.cpp
	$(COMP) $(FLAGS) -c src/jsonwriter.cpp

base64.o: src/base64.cpp
	$(COMP) $(FLAGS) -c src/base64.cpp

image.o: src/image.cpp
	$(COMP) $(FLAGS) -c src/image.cpp 

pugixml.o: ../../lib/pugixml/src/pugixml.cpp
	$(COMP) $(FLAGS) -c ../../lib/pugixml/src/pugixml.cpp

lodepng.o: ../../lib/lodepng/lodepng.cpp
	$(COMP) $(FLAGS) -c ../../lib/lodepng/lodepng.cpp

clean:
	rm -rf *o batchtool
	rm output/*

