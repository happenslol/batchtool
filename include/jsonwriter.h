#include <string>
#include <fstream>
#include <iostream>

#include <rapidjson/include/rapidjson/document.h>
#include <rapidjson/include/rapidjson/writer.h>
#include <rapidjson/include/rapidjson/prettywriter.h>
#include <rapidjson/include/rapidjson/stringbuffer.h>
#include <rapidjson/include/rapidjson/filewritestream.h>

#include <image.h>

class jsonwriter {
    public:
        jsonwriter();
        ~jsonwriter();

        void append_image(const image& img);
        void set_meta(const image& output_img);
        void write_file(const std::string& filename);

    private:
        rapidjson::Document document_;
};
