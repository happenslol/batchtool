//
//
//
#include <iostream>
#include <cstdlib>
#include <sstream>

#include "lib/pugixml/pugixml.hpp"
#include "lib/lodepng/lodepng.h"


using namespace std;

struct Pos {
	unsigned int x, y;
	Pos() : x(0), y(0) {}
	Pos(unsigned int x, unsigned int y) 
		: x(x), y(y) {}
};

struct Size {
	unsigned int width, height;
	Size() : width(0), height(0) {}
	Size(unsigned int width, unsigned int height) 
		: width(width), height(height) {}
};

struct Rect {
	Pos position;
	Size size;
	Rect() {}
	Rect(unsigned int x,
	     unsigned int y,
	     unsigned int width,
	     unsigned int height) 
		: position(Pos(x, y)), size(Size(width, height)) {}
};


struct RGBA {
	unsigned int R, G, B, A;
	RGBA() : R(255), G(0), B(0), A(255) {}
	RGBA(unsigned int R, 
	     unsigned int G, 
	     unsigned int B, 
	     unsigned int A) 
		: R(R), G(G), B(B), A(A) {}
};

struct image {
	string name;
	vector<vector<RGBA>> pixels;
	Size size;	
	Size o_size;
	Rect bounds;
	Pos offset;

	image(const string& name) : name(name), size(Size()), o_size(Size()), bounds(Rect()) {}

	void set_pixels(vector<unsigned char>& raw, 
			unsigned int raw_width,
		        unsigned int raw_height) {
		size.width = raw_width;
		size.height = raw_height;

		o_size.width = raw_width;
		o_size.height = raw_height;

		for (unsigned int y = 0; y < size.height; ++y) {
			vector<RGBA> row;
			unsigned int row_start = y * 4 * size.width;

			for (unsigned int x = 0; x < size.width; ++x) {
				RGBA new_pixel(raw[row_start + (x * 4)],		// R 
					       raw[row_start + (x * 4) + 1], 	// G
					       raw[row_start + (x * 4) + 2], 	// B
					       raw[row_start + (x * 4) + 3]);	// A
				row.push_back(new_pixel);
			}
			pixels.push_back(row);
		}
	}

	vector<unsigned char> get_pixels() {
		vector<unsigned char> raw;
		for (vector<RGBA> current_row : pixels)
			for (RGBA current_pixel : current_row) {
				raw.push_back(current_pixel.R);
				raw.push_back(current_pixel.G);
				raw.push_back(current_pixel.B);
				raw.push_back(current_pixel.A);
			}

		return raw;
	}
};

void append_width(image& img, unsigned int new_width) {
	if (new_width <= img.size.width) 
		cout << "append width failed: new width too small" << endl;
	else {
		unsigned int w_diff = new_width - img.size.width;

		for (vector<RGBA>& current_row : img.pixels)
			for (unsigned int i = 0; i < w_diff; ++i)
				current_row.push_back(RGBA());
		
		img.size.width = new_width;
	}
}

void append_height(image& img, unsigned int new_height) {
	if (new_height <= img.size.height) 
		cout << "append height failed: new height too small" << endl;
	else {
		unsigned int h_diff = new_height - img.size.height;

		for (unsigned int y = 0; y < h_diff; ++y) {
			vector<RGBA> new_row;

			for (unsigned int x = 0; x < img.size.width; ++x)
				new_row.push_back(RGBA());
			
			img.pixels.push_back(new_row);
		}

		img.size.height = new_height;
	}
}

void append_image_bot(image& base_image, image& image_to_append) {
	
	// bring both images to the same width
	if (base_image.size.width < image_to_append.size.width)
		append_width(base_image, image_to_append.size.width);
	else if (base_image.size.width > image_to_append.size.width)
		append_width(image_to_append, base_image.size.width);

	// append new rows from other image at bottom
	for (vector<RGBA> row : image_to_append.pixels)
		base_image.pixels.push_back(row);

	// update height
	base_image.size.height += image_to_append.size.height;
}

void append_image_right(image& base_image, image& image_to_append) {

	// bring both images to the same height
	if (base_image.size.height < image_to_append.size.height)
		append_height(base_image, image_to_append.size.height);
	else if (base_image.size.height > image_to_append.size.height)
		append_height(image_to_append, base_image.size.height);

	// append new rows from other image at bottom
	for (unsigned int y = 0; y < base_image.size.height; ++y)
		base_image.pixels.at(y).insert(base_image.pixels.at(y).end(), 
					       image_to_append.pixels.at(y).begin(), 
					       image_to_append.pixels.at(y).end());

	// update width
	base_image.size.width += image_to_append.size.width;

}

void trim_image(image& img) {
	// Trim rows from top down
	bool is_empty = true;
	unsigned int i = 0;
	do {
		for (RGBA& current_pixel : img.pixels.front()) 
			if (current_pixel.A != 0) {
				is_empty = false;
				break;
			}
		if (is_empty) {
			img.pixels.erase(img.pixels.begin());
			img.size.height -= 1;
			img.offset.y += 1;
			++i;
		}
	} while (is_empty && i < img.size.height);
	
	// Trim rows from bottom up
	is_empty = true;
	i = img.size.height;
	do {
		for (RGBA& current_pixel : img.pixels.back()) 
			if (current_pixel.A != 0) {
				is_empty = false;
				break;
			}
		if (is_empty) {
			img.pixels.pop_back();
			img.size.height -= 1;
			--i;
		}
	} while (is_empty && i > 0);
	
	// Trim from left to right
	is_empty = true;
	i = 0;
	do {
		for (vector<RGBA>& current_row : img.pixels) 
			if (current_row.front().A != 0) {
				is_empty = false;
				break;
			}
		if (is_empty) {
			for (vector<RGBA>& current_row : img.pixels)
				current_row.erase(current_row.begin());
			img.size.width -= 1;
			img.offset.x += 1;	
			++i;
		}	
	} while (is_empty && i < img.size.width);

	// Trim from right to left
	is_empty = true;
	i = img.size.width;
	do {

		for (vector<RGBA>& current_row : img.pixels) 
			if (current_row.back().A != 0) {
				is_empty = false;
				break;
			}
		if (is_empty) {
			for (vector<RGBA>& current_row : img.pixels)
				current_row.pop_back();
			img.size.width -= 1;
			--i;
		}
	} while (is_empty && i > 0);

}

void draw_bounds(image& img) {
	// first and last row
	for (RGBA& current_pixel : img.pixels.front()) {
		current_pixel.R = 0;
		current_pixel.G = 0;
		current_pixel.B = 0;
		current_pixel.A = 255;
	}
	for (RGBA& current_pixel : img.pixels.back()) {
		current_pixel.R = 0;
		current_pixel.G = 0;
		current_pixel.B = 0;
		current_pixel.A = 255;
	}

	// sides
	for (vector<RGBA>& current_row : img.pixels) {
		current_row.front().R = 0;		
		current_row.front().G = 0;		
		current_row.front().B = 0;		
		current_row.front().A = 255;
		current_row.back().R = 0;		
		current_row.back().G = 0;		
		current_row.back().B = 0;		
		current_row.back().A = 255;
	}	
}

void read_image(image& img, const string& filename) {
	vector<unsigned char> raw;
	unsigned int raw_width, raw_height;

	unsigned error = lodepng::decode(raw, raw_width, raw_height, filename);

	if (error) cout << "error reading file: " << lodepng_error_text(error) << endl;
	else img.set_pixels(raw, raw_width, raw_height);
}

void write_image(image& img, const string& filename) {
	// encode image and write to file
	unsigned error = lodepng::encode(filename, img.get_pixels(), img.size.width, img.size.height);
	if (error) cout << "error writing file: " << lodepng_error_text(error) << endl;
	else cout << filename << " written." << endl;
	
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
			read_image(new_image, argv[i]);


			if (new_image.size.width != 0 && new_image.size.height != 0) input.push_back(new_image);
		}
		

		/*stringstream trimbuffer; 
		
		for (image& current_image : input) {
			trimbuffer.str(string());
			trim_image(current_image);
			//draw_bounds(current_image);
			trimbuffer << "output/" << current_image.name;
			write_image(current_image, trimbuffer.str());
		}
		*/
		// Trim images
		cout << "trimming..." << endl;
		for (image& current_image : input)
			trim_image(current_image);
		// Draw bounding boxes (for debug purposes)
		cout << "drawing bounds..." << endl;
		for (image& current_image : input)
			draw_bounds(current_image);

		// Set first image as base
		image output = input.at(0);

		// add all images to the right
		// TODO: implement some sprite packing algorithm
		cout << "merging..." << endl;
		for (unsigned int i = 1; i < input.size(); ++i)
			append_image_right(output, input.at(i));

		if (input.size() == 1)
			cout << "only 1 image has been input!";
		else {
			remove("output.png");
			write_image(output, "output.png");
			
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
				frames.append_child("key").append_child(pugi::node_pcdata).set_value(img.name.c_str());

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
				buffer << "{" << img.o_size.width << "," << img.o_size.height << "}";
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
			buffer << "{" << output.size.width << "," << output.size.height << "}";
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

	// cin.get();

	return 0;

}

