#ifndef REPORTS_HPP_INCLUDED
#define REPORTS_HPP_INCLUDED

#include <string>

#include "gamestatus.hpp"
#include "map.hpp"
#include "team.hpp"
#include "unit.hpp"

//this module is responsible for outputting textual reports of
//various game and unit statistics
namespace reports {

	enum TYPE { UNIT_DESCRIPTION, UNIT_TYPE, UNIT_LEVEL, UNIT_TRAITS, UNIT_STATUS,
	            UNIT_ALIGNMENT, UNIT_HP, UNIT_XP, UNIT_MOVES, UNIT_WEAPONS,
				UNIT_IMAGE, UNIT_PROFILE, TIME_OF_DAY,
				TURN, GOLD, VILLAGES, NUM_UNITS, UPKEEP, EXPENSES, INCOME, TERRAIN, POSITION,
	            NUM_REPORTS};

	enum { UNIT_REPORTS_BEGIN=UNIT_DESCRIPTION, UNIT_REPORTS_END=UNIT_PROFILE+1 };
	enum { STATUS_REPORTS_BEGIN=TIME_OF_DAY, STATUS_REPORTS_END=NUM_REPORTS};

	const std::string& report_name(TYPE type);

	struct report {
		report() {}
		explicit report(const std::string& text) : text(text) {}
		report(const std::string& text, const std::string& image) : text(text), image(image) {}
		bool empty() const { return text.empty() && image.empty(); }
		bool operator==(const report& o) const { return o.text == text && o.image == image; }
		bool operator!=(const report& o) const { return !(o == *this); }
		std::string text, image;
	};

	report generate_report(TYPE type, const gamemap& map, const unit_map& units,
		                   const team& current_team, int current_side,
						   const gamemap::location& loc, const gamemap::location& mouseover,
						   const gamestatus& status, const std::string* format_string=NULL);
}

#endif
