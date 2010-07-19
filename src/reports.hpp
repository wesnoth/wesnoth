/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef REPORTS_HPP_INCLUDED
#define REPORTS_HPP_INCLUDED

#include <set>
#include <string>
#include <vector>

#include "image.hpp"

class team;

//this module is responsible for outputting textual reports of
//various game and unit statistics
namespace reports {
	enum TYPE { UNIT_NAME, UNIT_TYPE, UNIT_RACE, UNIT_LEVEL,
		    UNIT_SIDE, UNIT_AMLA, UNIT_TRAITS, UNIT_STATUS,
		    UNIT_ALIGNMENT, UNIT_ABILITIES, UNIT_HP, UNIT_XP,
		    UNIT_ADVANCEMENT_OPTIONS, UNIT_DEFENSE, UNIT_MOVES, UNIT_WEAPONS,
		    UNIT_IMAGE, UNIT_PROFILE, TIME_OF_DAY,
		    TURN, GOLD, VILLAGES, NUM_UNITS, UPKEEP, EXPENSES,
		    INCOME, TERRAIN, POSITION, SIDE_PLAYING, OBSERVERS,
		REPORT_COUNTDOWN, REPORT_CLOCK, EDITOR_SELECTED_TERRAIN,
		EDITOR_LEFT_BUTTON_FUNCTION, EDITOR_TOOL_HINT, NUM_REPORTS
	};

	enum { UNIT_REPORTS_BEGIN=UNIT_NAME, UNIT_REPORTS_END=UNIT_PROFILE+1 };
	enum { STATUS_REPORTS_BEGIN=TIME_OF_DAY, STATUS_REPORTS_END=EDITOR_TOOL_HINT };

	const std::string& report_name(TYPE type);

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

	report generate_report(TYPE type,
		const team &current_team,
			       int current_side, int active_side,
			       const map_location& loc, const map_location& mouseover, const map_location& displayed_unit_hex,
		const std::set<std::string> &observers,
			       const config& level, bool show_everything = false);
}

#endif
