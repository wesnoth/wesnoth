#ifndef REPORTS_HPP_INCLUDED
#define REPORTS_HPP_INCLUDED

#include <set>
#include <string>
#include <vector>

class gamestatus;

#include "map.hpp"
#include "team.hpp"
#include "unit.hpp"

//this module is responsible for outputting textual reports of
//various game and unit statistics
namespace reports {

	enum TYPE { UNIT_DESCRIPTION, UNIT_TYPE, UNIT_LEVEL, UNIT_TRAITS, UNIT_STATUS,
	            UNIT_ALIGNMENT, UNIT_ABILITIES, UNIT_HP, UNIT_XP, UNIT_MOVES, UNIT_WEAPONS,
				UNIT_IMAGE, UNIT_PROFILE, TIME_OF_DAY,
				TURN, GOLD, VILLAGES, NUM_UNITS, UPKEEP, EXPENSES, INCOME, TERRAIN, POSITION,
				SIDE_PLAYING, OBSERVERS, SELECTED_TERRAIN, EDIT_LEFT_BUTTON_FUNCTION, 
				NUM_REPORTS};

	enum { UNIT_REPORTS_BEGIN=UNIT_DESCRIPTION, UNIT_REPORTS_END=UNIT_PROFILE+1 };
	enum { STATUS_REPORTS_BEGIN=TIME_OF_DAY, STATUS_REPORTS_END=NUM_REPORTS};

	const std::string& report_name(TYPE type);

	struct element {
		// Invariant: either text or image should be empty
		// It would be okay to create a class for this, but it's a pretty simple
		// invariant so I left it like the original report class.
		std::string image;
		std::string text;
		
		std::string tooltip;
		
		element() {}
		explicit element(const std::string& text) : text(text) {}
		element(const std::string& text, const std::string& image, const std::string& tooltip) :
			image(image), text(text), tooltip(tooltip) {}
		
		bool operator==(const element& o) const {
			return o.text == text && o.image == image && o.tooltip == tooltip;
		}
		bool operator!=(const element& o) const { return !(o == *this); }
	};
	struct report : public std::vector<element> {
		report() {}
		explicit report(const std::string& text) { this->push_back(element(text)); }
		report(const std::string& text, const std::string& image, const std::string& tooltip) {
			this->push_back(element(text, image, tooltip));
		}
		
		// Convenience functions
		void add_text(std::stringstream& text, std::stringstream& tooltip);
		void add_text(const std::string& text, const std::string& tooltip);
		void add_image(const std::string& image, const std::string& tooltip);
		void add_image(std::stringstream& image, std::stringstream& tooltip);
	};

	report generate_report(TYPE type, const gamemap& map, const unit_map& units,
	                       const std::vector<team>& teams, const team& current_team,
	                       int current_side, int active_side,
	                       const gamemap::location& loc, const gamemap::location& mouseover,
	                       const gamestatus& status, const std::set<std::string>& observers);
	// Set what will be shown for the report with type
	// which_report. Note that this only works for some reports,
	// i.e. reports that can not be deducted from the supplied arguments
	// to generate_report. 
	// Currently: SELECTED_TERRAIN, EDIT_LEFT_BUTTON_FUNCTION
	void set_report_content(const TYPE which_report, const std::string &content);
}

#endif
