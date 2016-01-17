#include <string>
#include <fstream>
#include <iostream>

#include "../../../lib/rapidjson/include/rapidjson/document.h"
#include "../../../lib/rapidjson/include/rapidjson/writer.h"
#include "../../../lib/rapidjson/include/rapidjson/prettywriter.h"
#include "../../../lib/rapidjson/include/rapidjson/stringbuffer.h"
#include "../../../lib/rapidjson/include/rapidjson/filewritestream.h"

#include "image.h"

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
