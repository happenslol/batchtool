#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string>

#include "image.h"

using namespace std;

unsigned int nextPow2(unsigned int n) {
	--n;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;

	return ++n;
}


int main(int argc, const char* argv[])
{
	cout << "file root: " << argv[0] << endl;

	// Check if files have been input
	if (argc > 1) {
		cout << "loading images" << endl;

		vector<image> input;
		
		// Iterate through all input filepaths
		for (int i = 1; i < argc; i++) {
			string path(argv[i]);
			string filename = path.substr(path.find_last_of("/\\") + 1);
			cout << "loading image: " << filename << endl;
			
			image new_image(filename);
			new_image.read_file(argv[i]);

			if (new_image.size().width != 0 && new_image.size().height != 0)
			       input.push_back(new_image);
		}
		
		// Trim images
		cout << "trimming..." << endl;
		for (image& current_image : input)
			current_image.trim();

		// Draw bounding boxes (for debug purposes)
/*		cout << "drawing bounds..." << endl;
		for (image& current_image : input)
			current_image.draw_bounds();
*/
		// Set first image as base
		image output = input.at(0);

		// add all images to the right
		// TODO: implement some sprite packing algorithm
		if (input.size() == 1)
			cout << "only 1 image has been input!";
		else {
			cout << "merging..." << endl;
			for (unsigned int i = 1; i < input.size(); ++i)
				output.append_image_right(input.at(i));

			remove("output/output.png");
			output.write_file("output/output.png");
			
			// xml stuff goes here
			stringstream buffer;

			pugi::xml_document plist;
			pugi::xml_node root = plist.append_child("plist");
			root.append_attribute("version") = "1.0";
		
			// root plist dict
			root.append_child("dict");
		
			// frames start
			root.child("dict").append_child("key").append_child(pugi::node_pcdata).set_value("frames");
			pugi::xml_node frames = root.child("dict").append_child("dict");

			for (image& img : input) {
				// new frame with key(filename)
				frames.append_child("key").append_child(pugi::node_pcdata).set_value(img.name().c_str());

				// dict containing frame data
				pugi::xml_node framedict = frames.append_child("dict");

				// frame as string {{x,y},{width,height}}
				buffer << "{{" << 0 << "," << 0 << "},{" << 0 << "," << 0 << "}}";
				framedict.append_child("key").append_child(pugi::node_pcdata).set_value("frame");
				framedict.append_child("string").append_child(pugi::node_pcdata).set_value(buffer.str().c_str());
				buffer.str(string());

				// offset as string {x,y}
				buffer << "{" << 0 << "," << 0 << "}";
				framedict.append_child("key").append_child(pugi::node_pcdata).set_value("offset");
				framedict.append_child("string").append_child(pugi::node_pcdata).set_value(buffer.str().c_str());
				buffer.str(string());

				// rotated as bool
				framedict.append_child("key").append_child(pugi::node_pcdata).set_value("rotated");
				framedict.append_child("false");
	
				// sourceColorRect as string {{x,y},{width,height}}
				buffer << "{{" << 0 << "," << 0 << "},{" << 0 << "," << 0 << "}}";
				framedict.append_child("key").append_child(pugi::node_pcdata).set_value("sourceColorRect");
				framedict.append_child("string").append_child(pugi::node_pcdata).set_value(buffer.str().c_str());
				buffer.str(string());

				// sourceSize as string {width, height}
				buffer << "{" << img.o_size().width << "," << img.o_size().height << "}";
				framedict.append_child("key").append_child(pugi::node_pcdata).set_value("sourceSize");
				framedict.append_child("string").append_child(pugi::node_pcdata).set_value(buffer.str().c_str());
				buffer.str(string());
			}	

			// metadata start
			root.child("dict").append_child("key").append_child(pugi::node_pcdata).set_value("metadata");
			pugi::xml_node metadata = root.child("dict").append_child("dict");

			// format as integer
			metadata.append_child("key").append_child(pugi::node_pcdata).set_value("format");
			metadata.append_child("integer").append_child(pugi::node_pcdata).set_value("1");

			// size as string {width,height}
			buffer << "{" << output.size().width << "," << output.size().height << "}";
			metadata.append_child("key").append_child(pugi::node_pcdata).set_value("size");
			metadata.append_child("integer").append_child(pugi::node_pcdata).set_value(buffer.str().c_str());
			buffer.str(string());

			// textureFileName as string
			metadata.append_child("key").append_child(pugi::node_pcdata).set_value("textureFileName");
			metadata.append_child("string").append_child(pugi::node_pcdata).set_value("testout.png");

			plist.save_file("output.plist");
			cout << "output.plist written." << endl;	

		}
	}

	return 0;

}

