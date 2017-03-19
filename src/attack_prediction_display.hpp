/*
   Copyright (C) 2006 - 2017 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef ATTACK_PREDICTION_DISPLAY_H_INCLUDED
#define ATTACK_PREDICTION_DISPLAY_H_INCLUDED

class  attack_type;
class  battle_context;
struct battle_context_unit_stats;
struct map_location;
class  unit;
#include "show_dialog.hpp"

// This preview pane is shown in the "Damage Calculations" dialog.
class battle_prediction_pane : public gui::preview_pane
{
public:

	// Lengthy constructor.
	battle_prediction_pane(battle_context& bc,
		const map_location& attacker_loc, const map_location& defender_loc);
	battle_prediction_pane(battle_context&& bc,
		const map_location& attacker_loc, const map_location& defender_loc) :
		battle_prediction_pane(bc, attacker_loc, defender_loc) {}

	// This method is called to draw the dialog contents.
	void draw_contents();

	// Hack: pretend the preview pane goes to the left.
	bool left_side() const { return 1; }

	// Unused.
	void set_selection(int) {}

private:
	const map_location& attacker_loc_;
	const map_location& defender_loc_;
	const unit& attacker_;
	const unit& defender_;

	// Layout constants.
	static const int inter_line_gap_;
	static const int inter_column_gap_;
	static const int inter_units_gap_;
	static const int max_hp_distrib_rows_;

	// Layout computations.
	std::string attacker_label_, defender_label_;
	int attacker_label_width_, defender_label_width_;

	std::vector<std::string> attacker_left_strings_, attacker_right_strings_;
	std::vector<std::string> defender_left_strings_, defender_right_strings_;
	int attacker_strings_width_, attacker_left_strings_width_, attacker_right_strings_width_;
	int defender_strings_width_, defender_left_strings_width_, defender_right_strings_width_;
	int units_strings_height_;

	std::string hp_distrib_string_;
	surface attacker_hp_distrib_, defender_hp_distrib_;
	int hp_distrib_string_width_;
	int attacker_hp_distrib_width_, defender_hp_distrib_width_;
	int attacker_hp_distrib_height_, defender_hp_distrib_height_, hp_distribs_height_;

	int attacker_width_, defender_width_, units_width_;
	int dialog_width_, dialog_height_;

	// This method builds the strings describing the unit damage modifiers.
	// Read the code to understand the arguments.
	void get_unit_strings(const battle_context_unit_stats& stats,
					  const unit& u, const map_location& u_loc, float u_unscathed,
					  const unit& opp, const map_location& opp_loc, const attack_type *opp_weapon,
					  std::vector<std::string>& left_strings, std::vector<std::string>& right_strings,
				      int& left_strings_width, int& right_strings_width, int& strings_width);

	// Utility method that returns the length of the longest string in a vector of strings.
	int get_strings_max_length(const std::vector<std::string>& strings);

	// This method builds the vector containing the <HP, probability> pairs
	// that are required to draw the image of the hitpoints distribution of
	// a combatant after a fight. The method takes as input the probability
	// distribution of the hitpoints of the combatant after the fight.
	void get_hp_prob_vector(const std::vector<double>& hp_dist,
							std::vector<std::pair<int, double> >& hp_prob_vector);

	// This method draws a unit in the dialog pane. Read the code to understand
	// the arguments.
	void draw_unit(int x_off, int damage_line_skip, int left_strings_width,
				   const std::vector<std::string>& left_strings,
				   const std::vector<std::string>& right_strings,
				   const std::string& label, int label_width,
				   surface& hp_distrib, int hp_distrib_width);

	// This method draws the image of the hitpoints distribution of a
	// combatant after a fight. The method takes as input the
	// "hp_prob_vector" computed above and the stats of the combatants.
	// It draws the image in the surface 'surf' and set the width and
	// height of the image in the fields specified.
	void get_hp_distrib_surface(const std::vector<std::pair<int, double> >& hp_prob_vector,
							const battle_context_unit_stats& stats,
								const battle_context_unit_stats& opp_stats,
								surface& surf, int& width, int& height);
};

#endif
