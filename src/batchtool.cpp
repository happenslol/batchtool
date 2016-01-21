#define DEBUG 0

#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>
#include <string>
#include <algorithm>

#include <unistd.h>

#include <image.h>
#include <xmlwriter.h>
#include <jsonwriter.h>

#include <base64.h>

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

static void usage(const string& bin) {
    cerr << "Usage: " << bin << " <option(s)> SOURCES" << endl
         << "Options:" << endl
         << "\t-v\t\tShow verbose output" << endl
         << "\t-t\t\tTrim images" << endl
         << "\t-p\t\tMake output square and sidelength pow2" << endl
         << "\t-b\t\tDraw bounding boxes" << endl
         << "\t-o FILENAME\tOutput filename" << endl
         << "\t-f FORMAT\tOutput format" << endl;
}

int main(int argc, char **argv)
{
    if (argc <= 1) {
        usage(argv[0]);
        return 0;
    }

    bool VERBOSE = false;
    bool TRIM = false;
    bool POW2 = false;
    bool BOUNDS = false;
    string OUTPUT_NAME = string("output");
    int OUTPUT_FORMAT = 0;

    // Command line parsing
    opterr = 1;
    int c;

    while ((c = getopt(argc, argv, "vtbo:f:")) != -1) {
        switch(c) {
            case 'v':
                VERBOSE = true;
                break;
            case 't':
                TRIM = true;
                break;
            case 'b':
                BOUNDS = true;
                break;
            case 'o':
                OUTPUT_NAME = string(optarg);
                break;
            case 'f':
                cout << "Optarg at -f: " << optarg << endl;
                if (!strcmp("xml", optarg))
                    OUTPUT_FORMAT = 0;
                else if (!strcmp("plist", optarg))
                    OUTPUT_FORMAT = 1;
                else if (!strcmp("sprite", optarg))
                    OUTPUT_FORMAT = 2;
                else if (!strcmp("json", optarg))
                    OUTPUT_FORMAT = 3; 
            default:
                break;
        }
    }

#if DEBUG
    cout << "Options:" << endl
         << "\tverbose: " << VERBOSE << endl
         << "\ttrim: " << TRIM << endl
         << "\toutput_name: " << OUTPUT_NAME << endl
         << "\toutput_format: " << OUTPUT_FORMAT << endl;
#endif

    // Iterate through arguments
    vector<image> input;
    for (int i = optind; i < argc; ++i) {
        string path(argv[i]);
        string filename = path.substr(path.find_last_of("/\\") + 1);

        if (VERBOSE) {
            cout << "Loading image: " << filename << endl;
        }
        
        image new_image(filename);
        new_image.read_file(argv[i]);

        if (new_image.size().width != 0
            && new_image.size().height != 0)
               input.push_back(new_image);
    }
    
    if (TRIM) {
        // Trim images
        cout << "Trimming..." << endl;
        for (image& current_image : input)
            current_image.trim();
    }

    if (BOUNDS) {
        // Draw bounding boxes
        cout << "Drawing bounds..." << endl;
        for (image& current_image : input)
            current_image.draw_bounds();
    }

    if (input.size() <= 1)
        cout << "More than 1 image required.";
    else {
        cout << "Merging..." << endl;

        sort(input.begin(), input.end(), image::compare_area);

        vector<Pos> free_anchors;
        vector<Pos> abs_anchors;

        free_anchors.push_back(Pos());

        for (unsigned int i = 0; i < input.size(); ++i) {
            // set next image anchor to current 
            // smallest anchor point
            abs_anchors.push_back(free_anchors.front());
            
            // find new anchors
            Pos new_anchor_right(free_anchors.front().x
                                 + input.at(i).size().width,
                                 free_anchors.front().y);   
            Pos new_anchor_bot(free_anchors.front().x,
                               free_anchors.front().y
                               + input.at(i).size().height); 

            // still finding new anchors
            for (int j = 1; j < free_anchors.size(); ++j) {
                if ((free_anchors.at(j).x
                     >= free_anchors.front().x) &&
                    (free_anchors.at(j).x <= new_anchor_right.x)) {
                    new_anchor_right.y = min(new_anchor_right.y,
                                             free_anchors.at(j).y);
                    free_anchors.erase(free_anchors.begin() + j);
                    continue;
                }
                if ((free_anchors.at(j).y
                     >= free_anchors.front().y) &&
                    (free_anchors.at(j).y <= new_anchor_bot.y)) {
                    new_anchor_bot.x = min(new_anchor_bot.x,
                                           free_anchors.at(j).x);
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
        
#if DEBUG
        for (Pos& current_anchor : abs_anchors) 
            cout << ++x
                 << ": absolute anchor x: "
                 << current_anchor.x
                 << " y: "
                 << current_anchor.y
                 << " sum: "
                 << current_anchor.x + current_anchor.y
                 << endl;
#endif

        unsigned int side_length_x = 0;
        unsigned int side_length_y = 0;

        for (Pos& current_anchor : free_anchors) {
            side_length_x = max(side_length_x, current_anchor.x);
            side_length_y = max(side_length_y, current_anchor.y);
        }

        cout << "biggest side length x is " 
             << side_length_x 
             << endl;
        cout << "biggest side length y is " 
             << side_length_y
             << endl;

        if (POW2) {
            unsigned int side_length = max(nextPow2(side_length_x), nextPow2(side_length_x));
            side_length_x = side_length_y = side_length;
            cout << "Pow2 side length is " << side_length << endl;
        }

        image output_image(OUTPUT_NAME + ".png");
        output_image.set_size(Size(side_length_x, side_length_y));

        if (VERBOSE)
            cout << "output image size is now "
                 << output_image.size().width << ", "
                 << output_image.size().height << endl;

        if (abs_anchors.size() != input.size()) {
            cout << "Count mismatch!" << endl;
            return -1;
        }

        for (int i = 0; i < abs_anchors.size(); ++i) {
            output_image.insert_image(abs_anchors.at(i), input.at(i));
            input.at(i).position(abs_anchors.at(i));
        }

        output_image.write_file();

        // encode output file to b64
        /*
        ifstream pngInput("output/output.png", ios::binary);
        string pngString((istreambuf_iterator<char>(pngInput)), istreambuf_iterator<char>());
        cout << "read output png into string. length is " << pngString.length() << endl;
        string encoded = base64_encode(reinterpret_cast<const unsigned char*>(pngString.c_str()), pngString.length());
        */

        /*
        string decoded = base64_decode(encoded);
        ofstream pngOutput("output/decodedout.png", ios::binary);
        pngOutput << decoded;
        pngOutput.close();
        */

        // TODO: option to choose format from command line
#if 0
        xmlwriter* plistWriter = new xmlwriter(DOCTYPE_PLIST);
        for (image& img : input) {
            plistWriter->append_image(img);
        }
        plistWriter->set_meta(output_image);
        plistWriter->write_xml("output/plistout.plist");

        xmlwriter* spriteWriter = new xmlwriter(DOCTYPE_JSON);
        for (image& img : input)
            spriteWriter->append_image(img);
        spriteWriter->set_meta(output_image);
        spriteWriter->set_data(encoded);
        spriteWriter->write_xml("output/spriteout.json");
#else
        jsonwriter* writer = new jsonwriter();
        for (image& img : input)
            writer->append_image(img);
        writer->set_meta(output_image);
        writer->write_file(OUTPUT_NAME + ".json");
#endif
    }

    return 0;
}

