#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

struct Pos {
	unsigned int x, y;
	Pos() : x(0), y(0) {}
	Pos(unsigned int x, unsigned int y) 
		: x(x), y(y) {}
	static inline bool compare(const Pos& pos1, const Pos& pos2) {
		return (pow((double) pos1.x, 4) + pow((double) pos1.y, 4) < 
			pow((double) pos2.x, 4) + pow((double) pos2.y, 4));
	}
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
	RGBA() : R(0), G(0), B(0), A(0) {}
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

	const string& name() const { return name_; }
	const Size& size() const { return size_; }
	const Size& o_size() const { return o_size_; }
	const Rect& bounds() const { return bounds_; }
	const Pos& position() const { return position_; }
	void position(const Pos& position) { position_ = position; }
	const Pos& offset() const { return offset_; }

	// Raw pixel input/output
	void set_pixels(vector<unsigned char>& raw, 
			unsigned int raw_width,
			unsigned int raw_height);

	vector<unsigned char> get_pixels(); 

	void set_size(Size size);

	// comparing images
	static bool compare_area(const image& img1, const image& img2) {
		return (img1.get_area() > img2.get_area());	
	}

	inline int get_area() const {
		return (size_.width * size_.height);
	}

	// Image manipulation
	void append_width(unsigned int new_width);
	void append_height(unsigned int new_height);

	void append_image_bot(image& image_to_append);
	void append_image_right(image& image_to_append);

	void insert_image(Pos& pos, image& image_to_insert);

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
	Pos position_;
	Pos offset_;
};

#endif
