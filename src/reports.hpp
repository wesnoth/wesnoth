/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef REPORTS_HPP_INCLUDED
#define REPORTS_HPP_INCLUDED

#include "image.hpp"

class team;

//this module is responsible for outputting textual reports of
//various game and unit statistics
namespace reports {
	struct element {
		explicit element(const std::string& text) :
				image(),
				text(text),
				tooltip(),
				action()
				{}

		// Invariant: either text or image should be empty
		// It would be okay to create a class for this, but it's a pretty simple
		// invariant so I left it like the original report class.
		image::locator image;
		std::string text;

		std::string tooltip;
		std::string action;
		element(const std::string& text, const std::string& image,
				const std::string& tooltip, const std::string& action="") :
			image(image), text(text), tooltip(tooltip), action(action) {}

		element(const std::string& text, const image::locator& image,
				const std::string& tooltip,	const std::string& action="") :
			image(image), text(text), tooltip(tooltip), action(action) {}
		element(const std::string& text, const char* image,
				const std::string& tooltip, const std::string& action="") :
			image(image), text(text), tooltip(tooltip), action(action) {}

		bool operator==(const element& o) const {
			return o.text == text && o.image == image && o.tooltip == tooltip && o.action == action;
		}
		bool operator!=(const element& o) const { return !(o == *this); }
	};
	struct report : public std::vector<element> {
		report() {}
		explicit report(const std::string& text) { this->push_back(element(text)); }
		report(const std::string& text, const std::string& image, const std::string& tooltip, const std::string& action="") {
			this->push_back(element(text, image, tooltip, action));
		}
		report(const std::string& text, const char* image, const std::string& tooltip, const std::string& action="") {
			this->push_back(element(text, image, tooltip, action));
		}
		report(const std::string& text, const image::locator& image, const std::string& tooltip, const std::string& action="") {
			this->push_back(element(text, image, tooltip, action));
		}

		// Convenience functions
		void add_text(const std::string& text,
				const std::string& tooltip, const std::string& action="");
		void add_image(const std::string& image,
				const std::string& tooltip, const std::string& action="");
	};

struct report_data
{
	int viewing_side;
	int current_side;
	int active_side;
	map_location selected_hex;
	map_location mouseover_hex;
	map_location displayed_unit_hex;
	const std::set<std::string> &observers;
	const config &level;
	bool show_everything;
};

report generate_report(const std::string &name, const report_data &data);

const std::set<std::string> &report_list(bool for_units);
}

#endif
