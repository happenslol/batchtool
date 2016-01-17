#ifndef _XMLWRITER_H_
#define _XMLWRITER_H_

#include <string>
#include <sstream>

#include "../../../lib/pugixml/src/pugixml.hpp"

#include "image.h"

enum doctype {
	DOCTYPE_XML,
	DOCTYPE_PLIST,
	DOCTYPE_SPRITE,
};

class xmlwriter {
	public:
		xmlwriter(doctype type);
		~xmlwriter();

		void append_image(const image& img);
		void set_meta(const image& output_img);
		void set_data(const std::string& data);
		void write_xml(const std::string& filename);

	private:
		pugi::xml_document document_;
		pugi::xml_node root_;
		pugi::xml_node frames_;
		doctype type_;
};

#endif
