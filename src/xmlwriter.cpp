#include "xmlwriter.h"

#include <iostream>

xmlwriter::xmlwriter(doctype type) {
	type_ = type;
	if (type_ == DOCTYPE_PLIST) {
		root_ = document_.append_child("plist");
		root_.append_attribute("version") = "1.0";
	
		// root plist dict
		root_.append_child("dict");
	
		// frames start
		root_.child("dict").append_child("key").append_child(pugi::node_pcdata).set_value("frames");
		frames_ = root_.child("dict").append_child("dict");
	}
	else if (type_ == DOCTYPE_SPRITE) {
		root_ = document_.append_child("sprite");
		frames_ = root_.append_child("frames");
	}
}

xmlwriter::~xmlwriter() {
}

void xmlwriter::append_image(const image& img) {
	std::stringstream buffer;

	if (type_ == DOCTYPE_PLIST) {
		// new frame with key(filename)
		frames_.append_child("key").append_child(pugi::node_pcdata).set_value(img.name().c_str());

		// dict containing frame data
		pugi::xml_node framedict = frames_.append_child("dict");

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
		buffer << "{" << img.o_size().width << "," << img.o_size().height << "}";
		framedict.append_child("key").append_child(pugi::node_pcdata).set_value("sourceSize");
		framedict.append_child("string").append_child(pugi::node_pcdata).set_value(buffer.str().c_str());
		buffer.str(string());
	}
	else if (type_ == DOCTYPE_SPRITE) {
		// new frame node
		pugi::xml_node newFrame = frames_.append_child("frame");

		// name node
		newFrame.append_child("name").append_child(pugi::node_pcdata).set_value(img.name().c_str());	

		// size node with (actual) width and height attributes
		pugi::xml_node sizeNode = newFrame.append_child("size");
		buffer << img.size().width;
		sizeNode.append_attribute("width") = buffer.str().c_str();
		buffer.str(string());
		buffer << img.size().height;
		sizeNode.append_attribute("height") = buffer.str().c_str();
		buffer.str(string());

		// position node with x and y
		pugi::xml_node posNode = newFrame.append_child("position");
		buffer << img.position().x;
		posNode.append_attribute("x") = buffer.str().c_str();
		buffer.str(string());
		buffer << img.position().y;
		posNode.append_attribute("y") = buffer.str().c_str();
		buffer.str(string());

		// offset node with x and y
		pugi::xml_node offsetNode = newFrame.append_child("offset");
		buffer << img.offset().x;
		offsetNode.append_attribute("x") = buffer.str().c_str();
		buffer.str(string());
		buffer << img.offset().y;
		offsetNode.append_attribute("y") = buffer.str().c_str();
		buffer.str(string());

		// source node with width and height
		pugi::xml_node sourceNode = newFrame.append_child("source");
		buffer << img.o_size().width;
		sourceNode.append_attribute("width") = buffer.str().c_str();
		buffer.str(string());
		buffer << img.o_size().height;
		sourceNode.append_attribute("height") = buffer.str().c_str();
		buffer.str(string());
	}
}

void xmlwriter::set_meta(const image& output_img) {
	std::stringstream buffer;

	if (type_ == DOCTYPE_PLIST) {
		// metadata start
		root_.child("dict").append_child("key").append_child(pugi::node_pcdata).set_value("metadata");
		pugi::xml_node metadata = root_.child("dict").append_child("dict");

		// format as integer
		metadata.append_child("key").append_child(pugi::node_pcdata).set_value("format");
		metadata.append_child("integer").append_child(pugi::node_pcdata).set_value("1");

		// size as string {width,height}
		buffer << "{" << output_img.size().width << "," << output_img.size().height << "}";
		metadata.append_child("key").append_child(pugi::node_pcdata).set_value("size");
		metadata.append_child("integer").append_child(pugi::node_pcdata).set_value(buffer.str().c_str());
		buffer.str(string());

		// textureFileName as string
		metadata.append_child("key").append_child(pugi::node_pcdata).set_value("textureFileName");
		metadata.append_child("string").append_child(pugi::node_pcdata).set_value("output.png");
	}
	else if (type_ == DOCTYPE_SPRITE) {
		// metadata node
		pugi::xml_node metaNode = root_.append_child("metadata");

		// texture file name
		metaNode.append_child("name").append_child(pugi::node_pcdata).set_value(output_img.name().c_str());

		// size node
		pugi::xml_node sizeNode = metaNode.append_child("size");
		buffer << output_img.size().width;
		sizeNode.append_attribute("width") = buffer.str().c_str();
		buffer.str(string());
		buffer << output_img.size().height;
		sizeNode.append_attribute("height") = buffer.str().c_str();
		buffer.str(string());
	}
}

void xmlwriter::set_data(const std::string& data) {
	root_.append_child("data").append_child(pugi::node_pcdata).set_value(data.c_str());
}

void xmlwriter::write_xml(const std::string& filename) {
	document_.save_file(filename.c_str());
	std::cout << filename << " written." << std::endl;	
}	
