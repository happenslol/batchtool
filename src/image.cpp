#include "image.h"

#include "../lib/lodepng/lodepng.h"

using namespace std;

image::image(const string& name) : name_(name), 
		   		   size_(Size()), 
		   		   o_size_(Size()), 
		   		   bounds_(Rect()),
				   offset_(Pos(0, 0)) {}


void image::set_pixels(vector<unsigned char>& raw, 
		       unsigned int raw_width,
		       unsigned int raw_height) {
	size_.width = raw_width;
	size_.height = raw_height;

	o_size_.width = raw_width;
	o_size_.height = raw_height;

	for (unsigned int y = 0; y < size_.height; ++y) {
		vector<RGBA> row;
		unsigned int row_start = y * 4 * size_.width;

		for (unsigned int x = 0; x < size_.width; ++x) {
			RGBA new_pixel(raw[row_start + (x * 4)],     // R
				       raw[row_start + (x * 4) + 1], // G
				       raw[row_start + (x * 4) + 2], // B
				       raw[row_start + (x * 4) + 3]);// A
			row.push_back(new_pixel);
		}
		pixels_.push_back(row);
	}
}

vector<unsigned char> image::get_pixels() {
	vector<unsigned char> raw;
	for (vector<RGBA> current_row : pixels_)
		for (RGBA current_pixel : current_row) {
			raw.push_back(current_pixel.R);
			raw.push_back(current_pixel.G);
			raw.push_back(current_pixel.B);
			raw.push_back(current_pixel.A);
		}

	return raw;
}

void image::set_size(Size size) {
	size_ = size;
	pixels_.resize(size_.height);
	for (vector<RGBA>& current_row : pixels_)
		current_row.resize(size_.width, RGBA());
}


void image::append_width(unsigned int new_width) {
	if (new_width <= size_.width) 
		cout << "append width failed: new width too small" << endl;
	else {
		unsigned int w_diff = new_width - size_.width;

		for (vector<RGBA>& current_row : pixels_)
			for (unsigned int i = 0; i < w_diff; ++i)
				current_row.push_back(RGBA());
		
		size_.width = new_width;
	}
}

void image::append_height(unsigned int new_height) {
	if (new_height <= size_.height) 
		cout << "append height failed: new height too small" << endl;
	else {
		unsigned int h_diff = new_height - size_.height;

		for (unsigned int y = 0; y < h_diff; ++y) {
			vector<RGBA> new_row;

			for (unsigned int x = 0; x < size_.width; ++x)
				new_row.push_back(RGBA());
			
			pixels_.push_back(new_row);
		}

		size_.height = new_height;
	}
}

void image::append_image_bot(image& image_to_append) {
	
	// bring both images to the same width
	if (size_.width < image_to_append.size().width)
		append_width(image_to_append.size().width);
	else if (size_.width > image_to_append.size().width)
		image_to_append.append_width(size_.width);

	// append new rows from other image at bottom
	for (vector<RGBA> row : image_to_append.pixels())
		pixels_.push_back(row);

	// update height
	size_.height += image_to_append.size().height;
}

void image::append_image_right(image& image_to_append) {

	// bring both images to the same height
	if (size_.height < image_to_append.size().height)
		append_height(image_to_append.size().height);
	else if (size_.height > image_to_append.size().height)
		image_to_append.append_height(size_.height);

	// append new rows from other image to the right of each row
	for (int y = 0; 
	     y < (int) image_to_append.size().height; 
	     ++y) 
		for (int x = 0;
		     x < (int) image_to_append.size().width;
		     ++x) 
		     	pixels_.at(y).push_back(image_to_append.pixels().at(y).at(x));


	size_.width += image_to_append.size().width;

	cout << "images merged. array sizes: "
	     << pixels_.at(1).size()
	     << " width, "
	     << pixels_.size()
	     << " height. image size: "
	     << size_.width
	     << ", "
	     << size_.height
	     << endl;
	// update width

}

void image::insert_image(Pos& pos, image& image_to_insert) {
	// check if it fits
	if (pos.x + image_to_insert.size().width > size_.width ||
	    pos.y + image_to_insert.size().height > size_.height) {
		cout << "image doesnt fit!" << endl;
		return;
	}
	for (int y = 0; y < image_to_insert.size().height; ++y)
		for (int x = 0; x < image_to_insert.size().width; ++x) 
			pixels_[y + pos.y][x + pos.x] = image_to_insert.pixels().at(y).at(x);
		
}

void image::trim() {
	// Trim rows from top down
	bool is_empty = true;
	unsigned int i = 0;
	do {
		for (RGBA& current_pixel : pixels_.front()) 
			if (current_pixel.A != 0) {
				is_empty = false;
				break;
			}
		if (is_empty) {
			pixels_.erase(pixels_.begin());
			size_.height -= 1;
			offset_.y += 1;
			++i;
		}
	} while (is_empty && i < size_.height);
	
	// Trim rows from bottom up
	is_empty = true;
	i = size_.height;
	do {
		for (RGBA& current_pixel : pixels_.back()) 
			if (current_pixel.A != 0) {
				is_empty = false;
				break;
			}
		if (is_empty) {
			pixels_.pop_back();
			size_.height -= 1;
			--i;
		}
	} while (is_empty && i > 0);
	
	// Trim from left to right
	is_empty = true;
	i = 0;
	do {
		for (vector<RGBA>& current_row : pixels_) 
			if (current_row.front().A != 0) {
				is_empty = false;
				break;
			}
		if (is_empty) {
			for (vector<RGBA>& current_row : pixels_)
				current_row.erase(current_row.begin());
			size_.width -= 1;
			offset_.x += 1;	
			++i;
		}	
	} while (is_empty && i < size_.width);

	// Trim from right to left
	is_empty = true;
	i = size_.width;
	do {

		for (vector<RGBA>& current_row : pixels_) 
			if (current_row.back().A != 0) {
				is_empty = false;
				break;
			}
		if (is_empty) {
			for (vector<RGBA>& current_row : pixels_)
				current_row.pop_back();
			size_.width -= 1;
			--i;
		}
	} while (is_empty && i > 0);
}

void image::draw_bounds() {
	// first and last row
	for (RGBA& current_pixel : pixels_.front()) {
		current_pixel.R = 0;
		current_pixel.G = 0;
		current_pixel.B = 0;
		current_pixel.A = 255;
	}
	for (RGBA& current_pixel : pixels_.back()) {
		current_pixel.R = 0;
		current_pixel.G = 0;
		current_pixel.B = 0;
		current_pixel.A = 255;
	}

	// sides
	for (vector<RGBA>& current_row : pixels_) {
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

void image::read_file(const string& filename) {
	vector<unsigned char> raw;
	unsigned int raw_width, raw_height;

	unsigned error = lodepng::decode(raw,
					 raw_width,
					 raw_height,
					 filename);

	if (error) cout << "error reading file: " 
			<< lodepng_error_text(error)
			<< endl;
	else set_pixels(raw, raw_width, raw_height);
}

void image::write_file(const string& filename) {
	// encode image and write to file
	unsigned error = lodepng::encode(filename,
					 get_pixels(),
					 size_.width,
					 size_.height);
	cout << "trying to write " << filename 
		<< " with " 
		<< size_.width << " width and "
		<< size_.height << " height."
		<< endl;

	if (error) cout << "error writing file: " 
			<< lodepng_error_text(error) 
			<< endl;
	else cout << filename << " written." << endl;
	
}

