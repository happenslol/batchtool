#include <image.h>

#include <lodepng/lodepng.h>

using namespace std;

image::image(const string& name) : 
                   name_(name), 
		   		   size_(Size()), 
		   		   o_size_(Size()), 
		   		   bounds_(Rect()),
				   offset_(Pos(0, 0)) {}

image::~image() {
}

void image::set_pixels(vector<unsigned char>& raw, 
		       unsigned int raw_width,
		       unsigned int raw_height) {
	this->size_.width = raw_width;
	this->size_.height = raw_height;

	this->o_size_.width = raw_width;
	this->o_size_.height = raw_height;

	for (unsigned int y = 0; y < this->size_.height; ++y) {
		vector<RGBA> row;
		unsigned int row_start = y * 4 * this->size_.width;

		for (unsigned int x = 0; x < this->size_.width; ++x) {
			RGBA new_pixel(raw[row_start + (x * 4)],     // R
				       raw[row_start + (x * 4) + 1], // G
				       raw[row_start + (x * 4) + 2], // B
				       raw[row_start + (x * 4) + 3]);// A
			row.push_back(new_pixel);
		}
		this->pixels_.push_back(row);
	}
}

vector<unsigned char> image::get_pixels() {
	vector<unsigned char> raw;
	for (vector<RGBA> current_row : this->pixels_)
		for (RGBA current_pixel : current_row) {
			raw.push_back(current_pixel.R);
			raw.push_back(current_pixel.G);
			raw.push_back(current_pixel.B);
			raw.push_back(current_pixel.A);
		}
	return raw;
}

void image::set_size(Size size) {
	this->size_ = size;
	this->pixels_.resize(this->size_.height);
	for (vector<RGBA>& current_row : this->pixels_)
		current_row.resize(this->size_.width, RGBA());
}


void image::append_width(unsigned int new_width) {
	if (new_width <= this->size_.width) {
		cout << "Append width failed: new width too small" << endl;
        return;
    }

    unsigned int w_diff = new_width - this->size_.width;

    for (vector<RGBA>& current_row : this->pixels_)
        for (unsigned int i = 0; i < w_diff; ++i)
            current_row.push_back(RGBA());
    
    this->size_.width = new_width;
}

void image::append_height(unsigned int new_height) {
	if (new_height <= this->size_.height) {
		cout << "Append height failed: new height too small" << endl;
        return;
    }

    unsigned int h_diff = new_height - this->size_.height;

    for (unsigned int y = 0; y < h_diff; ++y) {
        vector<RGBA> new_row;

        for (unsigned int x = 0; x < this->size_.width; ++x)
            new_row.push_back(RGBA());
        
        this->pixels_.push_back(new_row);
    }

    this->size_.height = new_height;
}

void image::append_image_bot(image& image_to_append) {
	
	// Bring both images to the same width
	if (this->size_.width < image_to_append.size().width)
		append_width(image_to_append.size().width);
	else if (this->size_.width > image_to_append.size().width)
		image_to_append.append_width(this->size_.width);

	// Append new rows from other image at bottom
	for (vector<RGBA> row : image_to_append.pixels())
		this->pixels_.push_back(row);

	// Update height
	this->size_.height += image_to_append.size().height;
}

void image::append_image_right(image& image_to_append) {

	// Bring both images to the same height
	if (this->size_.height < image_to_append.size().height)
		append_height(image_to_append.size().height);
	else if (this->size_.height > image_to_append.size().height)
		image_to_append.append_height(this->size_.height);

	// Append new rows from other image to the right of each row
	for (int y = 0; y < (int) image_to_append.size().height; ++y) 
		for (int x = 0; x < (int) image_to_append.size().width; ++x) 
		     	this->pixels_.at(y).push_back(image_to_append.pixels().at(y).at(x));


	this->size_.width += image_to_append.size().width;

#if DEBUG
	cout << "images merged. array sizes: "
	     << this->pixels_.at(1).size()
	     << " width, "
	     << this->pixels_.size()
	     << " height. image size: "
	     << this->size_.width
	     << ", "
	     << this->size_.height
	     << endl;
#endif
}

void image::insert_image(Pos& pos, image& image_to_insert) {
	// Check dimensions
	if (pos.x + image_to_insert.size().width > this->size_.width ||
	    pos.y + image_to_insert.size().height > this->size_.height) {
		cout << "Cant insert image: Size mismatch!" << endl;
		return;
	}

	for (int y = 0; y < image_to_insert.size().height; ++y)
		for (int x = 0; x < image_to_insert.size().width; ++x) 
			this->pixels_[y + pos.y][x + pos.x] = image_to_insert.pixels().at(y).at(x);
		
}

void image::trim() {
	bool is_empty = true;
	unsigned int i = 0;

	// Trim rows from top down
	do {
		for (RGBA& current_pixel : this->pixels_.front()) 
			if (current_pixel.A != 0) {
				is_empty = false;
				break;
			}
		if (is_empty) {
			this->pixels_.erase(pixels_.begin());
			this->size_.height -= 1;
			this->offset_.y += 1;
			++i;
		}
	} while (is_empty && i < this->size_.height);
	
	// Trim rows from bottom up
	is_empty = true;
	i = this->size_.height;
	do {
		for (RGBA& current_pixel : this->pixels_.back()) 
			if (current_pixel.A != 0) {
				is_empty = false;
				break;
			}
		if (is_empty) {
			this->pixels_.pop_back();
			this->size_.height -= 1;
			--i;
		}
	} while (is_empty && i > 0);
	
	// Trim from left to right
	is_empty = true;
	i = 0;
	do {
		for (vector<RGBA>& current_row : this->pixels_) 
			if (current_row.front().A != 0) {
				is_empty = false;
				break;
			}
		if (is_empty) {
			for (vector<RGBA>& current_row : this->pixels_)
				current_row.erase(current_row.begin());
			this->size_.width -= 1;
			this->offset_.x += 1;	
			++i;
		}	
	} while (is_empty && i < this->size_.width);

	// Trim from right to left
	is_empty = true;
	i = this->size_.width;
	do {

		for (vector<RGBA>& current_row : this->pixels_) 
			if (current_row.back().A != 0) {
				is_empty = false;
				break;
			}
		if (is_empty) {
			for (vector<RGBA>& current_row : this->pixels_)
				current_row.pop_back();
			this->size_.width -= 1;
			--i;
		}
	} while (is_empty && i > 0);
}

void image::draw_bounds() {
	// First and last row
	for (RGBA& current_pixel : this->pixels_.front()) {
		current_pixel.R = 0;
		current_pixel.G = 0;
		current_pixel.B = 0;
		current_pixel.A = 255;
	}
	for (RGBA& current_pixel : this->pixels_.back()) {
		current_pixel.R = 0;
		current_pixel.G = 0;
		current_pixel.B = 0;
		current_pixel.A = 255;
	}

	// Sides
	for (vector<RGBA>& current_row : this->pixels_) {
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

	if (error) cout << "Error reading file: " 
			        << lodepng_error_text(error)
			        << endl;
	else set_pixels(raw, raw_width, raw_height);
}

void image::write_file() {
	// encode image and write to file
	unsigned error = lodepng::encode(name_,
					 get_pixels(),
					 size_.width,
					 size_.height);
	cout << "Writing file \"" 
         << this->name_ 
		 << "\" with " 
		 << this->size_.width << " width and "
		 << this->size_.height << " height."
		 << endl;

	if (error) cout << "Error writing file: " 
			        << lodepng_error_text(error) 
			        << endl;
	else cout << "\""
              << this->name_
              << "\" written."
              << endl;	
}

