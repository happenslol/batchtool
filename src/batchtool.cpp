#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>
#include <string>
#include <algorithm>

#include "image.h"
#include "xmlwriter.h"

#include "base64.h"

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

			for (int i = 0; i < abs_anchors.size(); ++i) {
				output_image.insert_image(abs_anchors.at(i), input.at(i));
				input.at(i).position(abs_anchors.at(i));
			}

			remove("output/output.png");
			output_image.write_file("output/" + output_image.name());

			// encode output file to b64
			ifstream pngInput("output/output.png", ios::binary);
			string pngString((istreambuf_iterator<char>(pngInput)), istreambuf_iterator<char>());
			cout << "read output png into string. length is " << pngString.length() << endl;
			string encoded = base64_encode(reinterpret_cast<const unsigned char*>(pngString.c_str()), pngString.length());

			string decoded = base64_decode(encoded);
			ofstream pngOutput("output/decodedout.png", ios::binary);
			pngOutput << decoded;
			pngOutput.close();
#if 0
			xmlwriter* plistWriter = new xmlwriter(DOCTYPE_PLIST);
			for (image& img : input) {
				plistWriter->append_image(img);
			}
			plistWriter->set_meta(output_image);
			plistWriter->write_xml("output/plistout.plist");
#else
			xmlwriter* spriteWriter = new xmlwriter(DOCTYPE_SPRITE);
			for (image& img : input)
				spriteWriter->append_image(img);
			spriteWriter->set_meta(output_image);
			spriteWriter->set_data(encoded);
			spriteWriter->write_xml("output/spriteout.sprite");
#endif
		}
	}
	
	cin.get();
	return 0;

}

