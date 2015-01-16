#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>

#include "../lib/pugixml/pugixml.hpp"

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

class image {
	public:
	image(const string& name);

	vector<vector<RGBA>> pixels() { return pixels_; }

	string& name() { return name_; }
	Size& size() { return size_; }
	Size& o_size() { return o_size_; }
	Rect& bounds() { return bounds_; }
	Pos& offset() { return offset_; }

	// Raw pixel input/output
	void set_pixels(vector<unsigned char>& raw, 
			unsigned int raw_width,
			unsigned int raw_height);

	vector<unsigned char> get_pixels(); 

	// Image manipulation
	void append_width(unsigned int new_width);
	void append_height(unsigned int new_height);

	void append_image_bot(image& image_to_append);
	void append_image_right(image& image_to_append);

	void trim();
	void draw_bounds();

	// reading/writing 
	void read_file(const string& filename);
	void write_file(const string& filename);

	private:
	string name_;
	vector<vector<RGBA>> pixels_;
	Size size_;
	Size o_size_;
	Rect bounds_;
	Pos offset_;
};
