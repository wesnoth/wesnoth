/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "help/topic.hpp"

#include "config.hpp"
#include "formatter.hpp"
#include "gettext.hpp"
#include "help/utils.hpp"
#include "serialization/parser.hpp"

#include <iostream>

namespace help
{
namespace
{
std::string convert_to_wml(const std::string& element_name, const std::string& contents)
{
	std::stringstream ss;

	bool in_quotes = false;
	bool last_char_escape = false;

	const char escape_char = '\\';

	std::vector<std::string> attributes;

	// Find the different attributes.
	// No checks are made for the equal sign or something like that.
	// Attributes are just separated by spaces or newlines.
	// Attributes that contain spaces must be in single quotes.
	for(std::size_t pos = 0; pos < contents.size(); ++pos) {
		const char c = contents[pos];

		if(c == escape_char && !last_char_escape) {
			last_char_escape = true;
		} else {
			if(c == '\'' && !last_char_escape) {
				ss << '"';
				in_quotes = !in_quotes;
			} else if((c == ' ' || c == '\n') && !last_char_escape && !in_quotes) {
				// Space or newline, end of attribute.
				attributes.push_back(ss.str());
				ss.str("");
			} else {
				ss << c;
			}

			last_char_escape = false;
		}
	}

	if(in_quotes) {
		throw parse_error(formatter() << "Unterminated single quote after: '" << ss.str() << "'");
	}

	if(!ss.str().empty()) {
		attributes.push_back(ss.str());
	}

	ss.str("");

	// Create the WML.
	ss << "[" << element_name << "]\n";

	for(const std::string& attr : attributes) {
		ss << attr << "\n";
	}

	ss << "[/" << element_name << "]\n";

	return ss.str();
}

config parse_text(const std::string& text)
{
	config res;

	const auto add_text_child =
		[&res](const std::string& text) { res.add_child("text", config {"text", text}); };

	bool last_char_escape = false;
	const char escape_char = '\\';

	std::stringstream ss;
	std::size_t pos;

	enum { ELEMENT_NAME, OTHER } state = OTHER;

	for(pos = 0; pos < text.size(); ++pos) {
		const char c = text[pos];

		if(c == escape_char && !last_char_escape) {
			last_char_escape = true;
		} else {
			if(state == OTHER) {
				if(c == '<') {
					if(last_char_escape) {
						ss << c;
					} else {
						add_text_child(ss.str());
						ss.str("");
						state = ELEMENT_NAME;
					}
				} else {
					ss << c;
				}
			} else if(state == ELEMENT_NAME) {
				if(c == '/') {
					throw parse_error("Erroneous / in element name.");
				} else if(c == '>') {
					const std::string element_name = ss.str();
					ss.str("");

					// End of this name.
					// For markup such as "<span color=''>", we only want the first word. If the name
					// has no spaces, the entirety will be used.
					std::stringstream s;
					s << "</" << element_name.substr(0, element_name.find_first_of(' ')) << ">";

					const std::string end_element_name = s.str();
					std::size_t end_pos = text.find(end_element_name, pos);

					if(end_pos == std::string::npos) {
						throw parse_error(formatter() << "Unterminated element: " << element_name);
					}

					const std::string contents = text.substr(pos + 1, end_pos - pos - 1);

					// If we find no '=' character, we assume we're dealing with Pango markup.
					if(contents.find('=') != std::string::npos) {
						s.str(convert_to_wml(element_name, contents));
						s.seekg(0);

						try {
							config cfg;
							read(cfg, s);

							res.append_children(cfg);
						} catch(const config::error& e) {
							throw parse_error(formatter() << "Error when parsing help markup as WML: " << e.message);
						}
					} else {
						add_text_child(formatter() << "<" << element_name << ">" << contents << end_element_name);
					}

					pos = end_pos + end_element_name.size() - 1;
					state = OTHER;
				} else {
					ss << c;
				}
			}

			last_char_escape = false;
		}
	}

	if(state == ELEMENT_NAME) {
		throw parse_error(formatter() << "Element '" << ss.str() << "' continues through end of string.");
	}

	// Add the last string.
	if(!ss.str().empty()) {
		res.add_child("text", config{"text", ss.str()});
	}

	return res;
}

} // end anon namespace

const config& topic::parsed_text() const
{
	if(text_generator_) {
		try {
			parsed_text_ = parse_text(text_generator_->generate());
		} catch(const parse_error& e) {
			std::cerr << e.message << std::endl;
		}

		text_generator_.reset(nullptr);
	}

	return parsed_text_;
}

bool topic::operator<(const topic& t) const
{
	return translation::compare(title, t.title) < 0;
}

} // namespace help
