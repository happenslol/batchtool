#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <algorithm>

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
		cout << "drawing bounds..." << endl;
		for (image& current_image : input)
			current_image.draw_bounds();

		// TODO: implement some sprite packing algorithm
		if (input.size() == 1)
			cout << "only 1 image has been input!";
		else {
			cout << "merging..." << endl;

			sort(input.begin(), input.end(), image::compare_area);

			vector<Pos> free_anchors;
			vector<Pos> abs_anchors;

			free_anchors.push_back(Pos());

			for (unsigned int i = 0; i < input.size(); ++i) {
				// set next image anchor to current smallest anchor point
				abs_anchors.push_back(free_anchors.front());
				
				// find new anchors
				Pos new_anchor_right(free_anchors.front().x + input.at(i).size().width,
						     free_anchors.front().y);	
				Pos new_anchor_bot(free_anchors.front().x,
						   free_anchors.front().y + input.at(i).size().height);	

				// still finding new anchors
				for (int j = 1; j < free_anchors.size(); ++j) {
					if ((free_anchors.at(j).x >= free_anchors.front().x) &&
					    (free_anchors.at(j).x <= new_anchor_right.x)) {
						new_anchor_right.y = min(new_anchor_right.y, free_anchors.at(j).y);
						free_anchors.erase(free_anchors.begin() + j);
						continue;
					}
					if ((free_anchors.at(j).y >= free_anchors.front().y) &&
					    (free_anchors.at(j).y <= new_anchor_bot.y)) {
						new_anchor_bot.x = min(new_anchor_bot.x, free_anchors.at(j).x);
						free_anchors.erase(free_anchors.begin() + j);
						continue;
					}
				}

				// remove first, add new anchors
				free_anchors.erase(free_anchors.begin());
				free_anchors.push_back(new_anchor_right);
				free_anchors.push_back(new_anchor_bot);

				// re-sort anchors
				sort(free_anchors.begin(), free_anchors.end(), Pos::compare);

			}

			int x = 0;
			for (Pos& current_anchor : abs_anchors) 
				cout << ++x
				     << ": absolute anchor x: "
				     << current_anchor.x
				     << " y: "
				     << current_anchor.y
				     << " sum: "
				     << current_anchor.x + current_anchor.y
				     << endl;

			unsigned int side_length = 0;

			for (Pos& current_anchor : free_anchors) {
				side_length = max(side_length, current_anchor.x);
				side_length = max(side_length, current_anchor.y);
			}

			cout << "biggest side length is " << side_length << endl;
			side_length = nextPow2(side_length);
			cout << "pow2 side length is " << side_length << endl;

			image output_image("output.png");
			output_image.set_size(Size(side_length, side_length));

			cout << "output image size is now " << output_image.size().width << ", " << output_image.size().height << endl;

			if (abs_anchors.size() != input.size()) {
				cout << "count mismatch!" << endl;
				return -1;
			}

			for (int i = 0; i < abs_anchors.size(); ++i)
				output_image.insert_image(abs_anchors.at(i), input.at(i));

			remove("output.png");
			output_image.write_file("output.png");
			
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
			buffer << "{" << output_image.size().width << "," << output_image.size().height << "}";
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
	
	cin.get();
	return 0;

}
