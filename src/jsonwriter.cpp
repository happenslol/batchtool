#include "jsonwriter.h"

jsonwriter::jsonwriter() {
    document_.SetObject();

    // For testing
    rapidjson::Value frames;
    frames.SetArray();

    document_.AddMember("frames", frames, document_.GetAllocator());
}

jsonwriter::~jsonwriter() {
}

void jsonwriter::append_image(const image& img) {
    rapidjson::Value frame("frame");
    frame.SetObject();

    // name value
    rapidjson::Value name(rapidjson::StringRef(img.name().c_str()));
    frame.AddMember("name", name, document_.GetAllocator());

    // size value which contains actual width and height
    rapidjson::Value size;
    size.SetObject();
    size.AddMember("width",
                   img.size().width,
                   document_.GetAllocator());
    size.AddMember("height",
                   img.size().height,
                   document_.GetAllocator());
    frame.AddMember("size", size, document_.GetAllocator());
    
    // position value on spritesheet
    rapidjson::Value position;
    position.SetObject();
    position.AddMember("x",
                   img.position().x,
                   document_.GetAllocator());
    position.AddMember("y",
                   img.position().y,
                   document_.GetAllocator());
    frame.AddMember("position", position, document_.GetAllocator());

    // offset value that defines how much space was trimmed
    rapidjson::Value offset;
    offset.SetObject();
    offset.AddMember("x",
                   img.offset().x,
                   document_.GetAllocator());
    offset.AddMember("y",
                   img.offset().y,
                   document_.GetAllocator());
    frame.AddMember("offset", offset, document_.GetAllocator());

    // source value which contains the original image size
    rapidjson::Value source;
    source.SetObject();
    source.AddMember("width",
                   img.o_size().width,
                   document_.GetAllocator());
    source.AddMember("height",
                   img.o_size().height,
                   document_.GetAllocator());
    frame.AddMember("source", source, document_.GetAllocator());

    // add finished frame to frames array
    document_["frames"].PushBack(frame, document_.GetAllocator());
}

void jsonwriter::set_meta(const image& output_img) {
    // Create meta object
    rapidjson::Value meta;
    meta.SetObject();

    // name value which contains the spritesheet filename
    rapidjson::Value name(
            rapidjson::StringRef(output_img.name().c_str()));
    meta.AddMember("name", name, document_.GetAllocator());

    // size value which contains size of spritesheet
    rapidjson::Value size;
    size.SetObject();
    size.AddMember("width",
                   output_img.size().width,
                   document_.GetAllocator());
    size.AddMember("height",
                   output_img.size().height,
                   document_.GetAllocator());
    meta.AddMember("size", size, document_.GetAllocator());

    // add everything to the document
    document_.AddMember("meta", meta, document_.GetAllocator());
}

void jsonwriter::write_file(const std::string& filename) {
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    document_.Accept(writer);

#if DEBUG 
    // Output json string for debug purposes
    std::cout << "\n" << buffer.GetString() << std::endl;
#endif

    // Write output to file
    std::ofstream out("output/output.json");
    out << buffer.GetString();
    out.close();
}
