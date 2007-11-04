/* $Id$ */
/*
   Copyright (C) 2006 - 2007 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "mouse_events.hpp"

#include "attack_prediction.hpp"
#include "cursor.hpp"
#include "dialogs.hpp"
#include "game_events.hpp"
#include "gettext.hpp"
#include "marked-up_text.hpp"
#include "menu_events.hpp"
#include "preferences_display.hpp"
#include "sound.hpp"
#include "replay.hpp"
#include "show_dialog.hpp"
#include "unit_abilities.hpp"
#include "wassert.hpp"
#include "wml_separators.hpp"
#include "unit_display.hpp"
#include "sdl_utils.hpp"

#include <cstdlib>

namespace events{

int commands_disabled = 0;

command_disabler::command_disabler()
{
	++commands_disabled;
}

command_disabler::~command_disabler()
{
	--commands_disabled;
}

static bool command_active()
{
#ifdef __APPLE__
	return (SDL_GetModState()&KMOD_META) != 0;
#else
	return false;
#endif
}

namespace{
	//minimum dragging distance to fire the drag&drop
	const double drag_threshold = 14.0;

	// This preview pane is shown in the "Damage Calculations" dialog.
	class battle_prediction_pane : public gui::preview_pane
	{
	public:

		// Lengthy constructor.
		battle_prediction_pane(game_display &disp, const battle_context& bc, const gamemap& map,
							   const std::vector<team>& teams, const unit_map& units,
							   const gamestatus& status, const game_data& gamedata,
							   const gamemap::location& attacker_loc, const gamemap::location& defender_loc);

		// This method is called to draw the dialog contents.
		void draw_contents();

		// Hack: pretend the preview pane goes to the left.
		bool left_side() const { return 1; }

		// Unused.
		void set_selection(int) {}

	private:
		game_display &disp_;
		const battle_context& bc_;
		const gamemap& map_;
		const std::vector<team>& teams_;
		const unit_map& units_;
		const gamestatus& status_;
		const game_data& gamedata_;
		const gamemap::location& attacker_loc_;
		const gamemap::location& defender_loc_;
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
		void get_unit_strings(const battle_context::unit_stats& stats,
						  const unit& u, const gamemap::location& u_loc, float u_unscathed,
						  const unit& opp, const gamemap::location& opp_loc, const attack_type *opp_weapon,
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
								const battle_context::unit_stats& stats,
									const battle_context::unit_stats& opp_stats,
									surface& surf, int& width, int& height);

		// This method blends a RGB color. The method takes as input a surface,
		// the RGB color to blend and a value specifying how much blending to
		// apply. The blended color is returned. Caution: if you use a
		// transparent color, make sure the resulting color is not equal to the
		// transparent color.
		Uint32 blend_rgb(const surface& surf, unsigned char r, unsigned char g, unsigned char b, unsigned char drop);
	};

	const int battle_prediction_pane::inter_line_gap_ = 3;
	const int battle_prediction_pane::inter_column_gap_ = 30;
	const int battle_prediction_pane::inter_units_gap_ = 30;
	const int battle_prediction_pane::max_hp_distrib_rows_ = 10;

	battle_prediction_pane::battle_prediction_pane(game_display &disp, const battle_context& bc, const gamemap& map,
												   const std::vector<team>& teams, const unit_map& units,
												   const gamestatus& status, const game_data& gamedata,
												   const gamemap::location& attacker_loc, const gamemap::location& defender_loc)
				: gui::preview_pane(disp.video()), disp_(disp), bc_(bc), map_(map), teams_(teams), units_(units), status_(status),
				  gamedata_(gamedata), attacker_loc_(attacker_loc), defender_loc_(defender_loc),
				  attacker_(units.find(attacker_loc)->second), defender_(units.find(defender_loc)->second)
	{
		// Predict the battle outcome.
		combatant attacker_combatant(bc.get_attacker_stats());
		combatant defender_combatant(bc.get_defender_stats());
		attacker_combatant.fight(defender_combatant);

		const battle_context::unit_stats& attacker_stats = bc.get_attacker_stats();
		const battle_context::unit_stats& defender_stats = bc.get_defender_stats();

		// Create the hitpoints distribution graphics.
		std::vector<std::pair<int, double> > hp_prob_vector;
		get_hp_prob_vector(attacker_combatant.hp_dist, hp_prob_vector);
		get_hp_distrib_surface(hp_prob_vector, attacker_stats, defender_stats, attacker_hp_distrib_,
							   attacker_hp_distrib_width_, attacker_hp_distrib_height_);
		get_hp_prob_vector(defender_combatant.hp_dist, hp_prob_vector);
		get_hp_distrib_surface(hp_prob_vector, defender_stats, attacker_stats, defender_hp_distrib_,
						   defender_hp_distrib_width_, defender_hp_distrib_height_);
		hp_distribs_height_ = maximum<int>(attacker_hp_distrib_height_, defender_hp_distrib_height_);

		// Build the strings and compute the layout.
		std::stringstream str;

		attacker_label_ = _("Attacker");
		defender_label_ = _("Defender");
		attacker_label_width_ = font::line_width(attacker_label_, font::SIZE_PLUS, TTF_STYLE_BOLD);
		defender_label_width_ = font::line_width(defender_label_, font::SIZE_PLUS, TTF_STYLE_BOLD);

		// Get the units strings.
		get_unit_strings(attacker_stats, attacker_, attacker_loc_, attacker_combatant.untouched,
						 defender_, defender_loc_, defender_stats.weapon,
						 attacker_left_strings_, attacker_right_strings_,
						 attacker_left_strings_width_, attacker_right_strings_width_, attacker_strings_width_);

		get_unit_strings(defender_stats, defender_, defender_loc_, defender_combatant.untouched,
						 attacker_, attacker_loc_, attacker_stats.weapon,
						 defender_left_strings_, defender_right_strings_,
						 defender_left_strings_width_, defender_right_strings_width_, defender_strings_width_);

		units_strings_height_ = maximum<int>(attacker_left_strings_.size(), defender_left_strings_.size())
							    * (font::SIZE_NORMAL + inter_line_gap_) + 14;

		hp_distrib_string_ = _("Expected Battle Result (HP)");
		hp_distrib_string_width_ = font::line_width(hp_distrib_string_, font::SIZE_SMALL);

		attacker_width_ = maximum<int>(attacker_label_width_, attacker_strings_width_);
		attacker_width_ = maximum<int>(attacker_width_, hp_distrib_string_width_);
		attacker_width_ = maximum<int>(attacker_width_, attacker_hp_distrib_width_);
		defender_width_ = maximum<int>(defender_label_width_, defender_strings_width_);
		defender_width_ = maximum<int>(defender_width_, hp_distrib_string_width_);
		defender_width_ = maximum<int>(defender_width_, defender_hp_distrib_width_);
		units_width_ = maximum<int>(attacker_width_, defender_width_);

		dialog_width_ = 2 * units_width_ + inter_units_gap_;
		dialog_height_ = 15 + 24 + units_strings_height_ + 14 + 19 + hp_distribs_height_ + 18;

		// Set the dialog size.
		set_measurements(dialog_width_, dialog_height_);
	}

	void battle_prediction_pane::get_unit_strings(const battle_context::unit_stats& stats,
											  const unit& u, const gamemap::location& u_loc, float u_unscathed,
											  const unit& opp, const gamemap::location& opp_loc, const attack_type *opp_weapon,
												  std::vector<std::string>& left_strings, std::vector<std::string>& right_strings,
										      int& left_strings_width, int& right_strings_width, int& strings_width)
	{
		std::stringstream str;
		char str_buf[10];

		// With a weapon.
		if(stats.weapon != NULL) {

			// Set specials context (for safety, it should not have changed normally).
			const attack_type *weapon = stats.weapon;
			weapon->set_specials_context(u_loc, opp_loc, &gamedata_, &units_, &map_, &status_, &teams_, stats.is_attacker, opp_weapon);

			// Get damage modifiers.
			unit_ability_list dmg_specials = weapon->get_specials("damage");
			unit_abilities::effect dmg_effect(dmg_specials, weapon->damage(), stats.backstab_pos);

			// Get the SET damage modifier, if any.
			const unit_abilities::individual_effect *set_dmg_effect = NULL;
			unit_abilities::effect_list::const_iterator i;
			for(i = dmg_effect.begin(); i != dmg_effect.end(); ++i) {
				if(i->type == unit_abilities::SET) {
					set_dmg_effect = &*i;
					break;
				}
			}

			// Either user the SET modifier or the base weapon damage.
			if(set_dmg_effect == NULL) {
				left_strings.push_back(weapon->name());
				str.str("");
				str << weapon->damage();
				right_strings.push_back(str.str());
			} else {
				left_strings.push_back((*set_dmg_effect->ability)["name"]);
				str.str("");
				str << set_dmg_effect->value;
				right_strings.push_back(str.str());
			}

			// Process the ADD damage modifiers.
			for(i = dmg_effect.begin(); i != dmg_effect.end(); ++i) {
				if(i->type == unit_abilities::ADD) {
					left_strings.push_back((*i->ability)["name"]);
					str.str("");
					if(i->value >= 0) str << "+" << i->value;
					else str << i->value;
					right_strings.push_back(str.str());
				}
			}

			// Process the MUL damage modifiers.
			for(i = dmg_effect.begin(); i != dmg_effect.end(); ++i) {
				if(i->type == unit_abilities::MUL) {
					left_strings.push_back((*i->ability)["name"]);
					str.str("");
					str << "* " << (i->value / 100);
					if(i->value % 100) {
						str << "." << ((i->value % 100) / 10);
						if(i->value % 10) str << (i->value % 10);
					}
					right_strings.push_back(str.str());
				}
			}

			// Resistance modifier.
			int resistance_modifier = opp.damage_from(*weapon, !stats.is_attacker, opp_loc);
			if(resistance_modifier != 100) {
				str.str("");
				if(stats.is_attacker) str << _("Defender");
				else str << _("Attacker");
				if(resistance_modifier < 100) str << _(" resistance vs ");
				else str << _(" vulnerability vs ");
				str << gettext(weapon->type().c_str());
				left_strings.push_back(str.str());
				str.str("");
				str << "* " << (resistance_modifier / 100) << "." << ((resistance_modifier % 100) / 10);
				right_strings.push_back(str.str());
			}

			// Slowed penalty.
			if(stats.is_slowed) {
				left_strings.push_back(_("Slowed"));
				right_strings.push_back("* 0.5");
			}

			// Time of day modifier.
			int tod_modifier = combat_modifier(status_, units_, u_loc, u.alignment(), u.is_fearless(), map_);
			if(tod_modifier != 0) {
				left_strings.push_back(_("Time of day"));
				str.str("");
				str << (tod_modifier > 0 ? "+" : "") << tod_modifier << "%";
				right_strings.push_back(str.str());
			}

	// Leadership bonus.
	int leadership_bonus = 0;
	under_leadership(units_, u_loc, &leadership_bonus);
			if(leadership_bonus != 0) {
				left_strings.push_back(_("Leadership"));
				str.str("");
				str << "+" << leadership_bonus << "%";
				right_strings.push_back(str.str());
			}

			// Total damage.
			left_strings.push_back(_("Total damage"));
			str.str("");
			str << stats.damage << "-" << stats.num_blows << " (" << stats.chance_to_hit << "%)";
			right_strings.push_back(str.str());

		// Without a weapon.
		} else {
			left_strings.push_back(_("No usable weapon"));
			right_strings.push_back("");
		}

		// Unscathed probability.
		left_strings.push_back(_("Chance of being unscathed"));
		snprintf(str_buf, 10, "%.1f%%", static_cast<float>(u_unscathed * 100.0));
		str_buf[9] = '\0';  //prevents _snprintf error
		right_strings.push_back(str_buf);

#if 0 // might not be en English!
		// Fix capitalisation of left strings.
		for(int i = 0; i < (int) left_strings.size(); i++)
			if(left_strings[i].size() > 0) left_strings[i][0] = toupper(left_strings[i][0]);
#endif

		// Compute the width of the strings.
		left_strings_width = get_strings_max_length(left_strings);
		right_strings_width = get_strings_max_length(right_strings);
		strings_width = left_strings_width + inter_column_gap_ + right_strings_width;
	}

	int battle_prediction_pane::get_strings_max_length(const std::vector<std::string>& strings)
	{
		int max_len = 0;

		for(int i = 0; i < static_cast<int>(strings.size()); i++)
			max_len = maximum<int>(font::line_width(strings[i], font::SIZE_NORMAL), max_len);

		return max_len;
	}

	void battle_prediction_pane::get_hp_prob_vector(const std::vector<double>& hp_dist,
													std::vector<std::pair<int, double> >& hp_prob_vector)
	{
		hp_prob_vector.clear();

		// First, we sort the probabilities in ascending order.
		std::vector<std::pair<double, int> > prob_hp_vector;
		int i;

		for(i = 0; i < static_cast<int>(hp_dist.size()); i++) {
			double prob = hp_dist[i];

			// We keep only values above 0.1%.
			if(prob > 0.001)
				prob_hp_vector.push_back(std::pair<double, int>(prob, i));
		}

		std::sort(prob_hp_vector.begin(), prob_hp_vector.end());

		// We store a few of the highest probability hitpoint values.
		int nb_elem = minimum<int>(max_hp_distrib_rows_, prob_hp_vector.size());

		for(i = prob_hp_vector.size() - nb_elem;
				i < static_cast<int>(prob_hp_vector.size()); i++) {

			hp_prob_vector.push_back(std::pair<int, double>
				(prob_hp_vector[i].second, prob_hp_vector[i].first));
			}

		// Then, we sort the hitpoint values in ascending order.
		std::sort(hp_prob_vector.begin(), hp_prob_vector.end());
	}

	void battle_prediction_pane::draw_contents()
	{
		// We must align both damage lines.
		int damage_line_skip = maximum<int>(attacker_left_strings_.size(), defender_left_strings_.size()) - 2;

		draw_unit(0, damage_line_skip,
				  attacker_left_strings_width_, attacker_left_strings_, attacker_right_strings_,
				  attacker_label_, attacker_label_width_, attacker_hp_distrib_, attacker_hp_distrib_width_);

		draw_unit(units_width_ + inter_units_gap_, damage_line_skip,
				  defender_left_strings_width_, defender_left_strings_, defender_right_strings_,
				  defender_label_, defender_label_width_, defender_hp_distrib_, defender_hp_distrib_width_);
	}

	void battle_prediction_pane::draw_unit(int x_off, int damage_line_skip, int left_strings_width,
										   const std::vector<std::string>& left_strings,
										   const std::vector<std::string>& right_strings,
										   const std::string& label, int label_width,
										   surface& hp_distrib, int hp_distrib_width)
	{
		CVideo& screen = disp_.video();
		int i;

		// NOTE. A preview pane is not made to be used alone and it is not
		// centered in the middle of the dialog. We "fix" this problem by moving
		// the clip rectangle 10 pixels to the right. This is a kludge and it
		// should be removed by 1) writing a custom dialog handler, or
		// 2) modify preview_pane so that it accepts {left, middle, right} as
		// layout possibilities.

		// Get clip rectangle and center it
		SDL_Rect clip_rect = location();
		clip_rect.x += 10;

		// Current vertical offset. We draw the dialog line-by-line, starting at the top.
		int y_off = 15;

		// Draw unit label.
		font::draw_text_line(&screen, clip_rect, font::SIZE_15, font::NORMAL_COLOUR, label,
							 clip_rect.x + x_off + (units_width_ - label_width) / 2, clip_rect.y + y_off, 0, TTF_STYLE_BOLD);

		y_off += 24;

		// Draw unit left and right strings except the last two (total damage and unscathed probability).
		for(i = 0; i < static_cast<int>(left_strings.size()) - 2; i++) {
			font::draw_text_line(&screen, clip_rect, font::SIZE_NORMAL, font::NORMAL_COLOUR, left_strings[i],
								 clip_rect.x + x_off, clip_rect.y + y_off + (font::SIZE_NORMAL + inter_line_gap_) * i,
								 0, TTF_STYLE_NORMAL);

			font::draw_text_line(&screen, clip_rect, font::SIZE_NORMAL, font::NORMAL_COLOUR, right_strings[i],
								 clip_rect.x + x_off + left_strings_width + inter_column_gap_,
								 clip_rect.y + y_off + (font::SIZE_NORMAL + inter_line_gap_) * i, 0, TTF_STYLE_NORMAL);
		}

		// Ensure both damage lines are aligned.
		y_off += damage_line_skip * (font::SIZE_NORMAL + inter_line_gap_) + 14;

		// Draw total damage and unscathed probability.
		for(i = 0; i < 2; i++) {
			const std::string& left_string = left_strings[left_strings.size() - 2 + i];
			const std::string& right_string = right_strings[right_strings.size() - 2 + i];

			font::draw_text_line(&screen, clip_rect, font::SIZE_NORMAL, font::NORMAL_COLOUR, left_string,
								 clip_rect.x + x_off, clip_rect.y + y_off + (font::SIZE_NORMAL + inter_line_gap_) * i,
								 0, TTF_STYLE_NORMAL);

			font::draw_text_line(&screen, clip_rect, font::SIZE_NORMAL, font::NORMAL_COLOUR, right_string,
								 clip_rect.x + x_off + left_strings_width + inter_column_gap_,
								 clip_rect.y + y_off + (font::SIZE_NORMAL + inter_line_gap_) * i, 0, TTF_STYLE_NORMAL);
		}

		y_off += 2 * (font::SIZE_NORMAL + inter_line_gap_) + 14;

		// Draw hitpoints distribution string.
		font::draw_text(&screen, clip_rect, font::SIZE_SMALL, font::NORMAL_COLOUR, hp_distrib_string_,
						clip_rect.x + x_off + (units_width_ - hp_distrib_string_width_) / 2, clip_rect.y + y_off);

		y_off += 19;

		// Draw hitpoints distributions.
		video().blit_surface(clip_rect.x + x_off + (units_width_ - hp_distrib_width) / 2, clip_rect.y + y_off, hp_distrib);
	}

	void battle_prediction_pane::get_hp_distrib_surface(const std::vector<std::pair<int, double> >& hp_prob_vector,
														const battle_context::unit_stats& stats,
														const battle_context::unit_stats& opp_stats,
														surface& surf, int& width, int& height)
	{
		// Font size. If you change this, you must update the separator space.
		int fs = font::SIZE_SMALL;

		// Space before HP separator.
		int hp_sep = 24 + 6;

		// Bar space between both separators.
		int bar_space = 150;

		// Space after percentage separator.
		int percent_sep = 43 + 6;

		// Surface width and height.
		width = 30 + 2 + bar_space + 2 + percent_sep;
		height = 5 + (fs + 2) * hp_prob_vector.size();

		// Create the surface.
		surf = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height,
									image::pixel_format->BitsPerPixel,
								image::pixel_format->Rmask,
								image::pixel_format->Gmask,
								image::pixel_format->Bmask,
								image::pixel_format->Amask);

		SDL_Rect clip_rect = {0, 0, width, height};
		Uint32 grey_color = SDL_MapRGB(surf->format, 0xb7, 0xc1, 0xc1);
		Uint32 transparent_color = SDL_MapRGB(surf->format, 1, 1, 1);

		// Enable transparency.
		SDL_SetColorKey(surf, SDL_SRCCOLORKEY, transparent_color);
		SDL_FillRect(surf, &clip_rect, transparent_color);

		// Draw the surrounding borders and separators.
		SDL_Rect top_border_rect = {0, 0, width, 2};
		SDL_FillRect(surf, &top_border_rect, grey_color);

		SDL_Rect bottom_border_rect = {0, height - 2, width, 2};
		SDL_FillRect(surf, &bottom_border_rect, grey_color);

		SDL_Rect left_border_rect = {0, 0, 2, height};
		SDL_FillRect(surf, &left_border_rect, grey_color);

		SDL_Rect right_border_rect = {width - 2, 0, 2, height};
		SDL_FillRect(surf, &right_border_rect, grey_color);

		SDL_Rect hp_sep_rect = {hp_sep, 0, 2, height};
		SDL_FillRect(surf, &hp_sep_rect, grey_color);

		SDL_Rect percent_sep_rect = {width - percent_sep - 2, 0, 2, height};
		SDL_FillRect(surf, &percent_sep_rect, grey_color);

		// Draw the rows (lower HP values are at the bottom).
		for(int i = 0; i < static_cast<int>(hp_prob_vector.size()); i++) {
			char str_buf[10];

			// Get the HP and probability.
			int hp = hp_prob_vector[hp_prob_vector.size() - i - 1].first;
			double prob = hp_prob_vector[hp_prob_vector.size() - i - 1].second;

			SDL_Color row_color;

			// Death line is red.
			if(hp == 0) {
				SDL_Color color = {0xe5, 0, 0, 0};
				row_color = color;
			}

			// Below current hitpoints value is orange.
			else if(hp < static_cast<int>(stats.hp)) {
				// Stone is grey.
				if(opp_stats.stones) {
					SDL_Color color = {0x9a, 0x9a, 0x9a, 0};
					row_color = color;
				} else {
					SDL_Color color = {0xf4, 0xc9, 0, 0};
					row_color = color;
				}
			}

			// Current hitpoints value and above is green.
			else {
				SDL_Color color = {0x08, 0xca, 0, 0};
				row_color = color;
			}

			// Print HP, aligned right.
			snprintf(str_buf, 10, "%d", hp);
			str_buf[9] = '\0';  //prevents _snprintf error
			int hp_width = font::line_width(str_buf, fs);

			// Draw bars.
			font::draw_text_line(surf, clip_rect, fs, font::NORMAL_COLOUR, str_buf,
								 hp_sep - hp_width - 2, 2 + (fs + 2) * i, 0, TTF_STYLE_NORMAL);

			int bar_len = maximum<int>(static_cast<int>((prob * (bar_space - 4)) + 0.5), 2);

			SDL_Rect bar_rect_1 = {hp_sep + 4, 6 + (fs + 2) * i, bar_len, 8};
			SDL_FillRect(surf, &bar_rect_1, blend_rgb(surf, row_color.r, row_color.g, row_color.b, 100));

			SDL_Rect bar_rect_2 = {hp_sep + 4, 7 + (fs + 2) * i, bar_len, 6};
			SDL_FillRect(surf, &bar_rect_2, blend_rgb(surf, row_color.r, row_color.g, row_color.b, 66));

			SDL_Rect bar_rect_3 = {hp_sep + 4, 8 + (fs + 2) * i, bar_len, 4};
			SDL_FillRect(surf, &bar_rect_3, blend_rgb(surf, row_color.r, row_color.g, row_color.b, 33));

			SDL_Rect bar_rect_4 = {hp_sep + 4, 9 + (fs + 2) * i, bar_len, 2};
			SDL_FillRect(surf, &bar_rect_4, blend_rgb(surf, row_color.r, row_color.g, row_color.b, 0));

			// Draw probability percentage, aligned right.
			const char *prob_str_format = NULL;

			if(prob > 0.9995) prob_str_format = "100 %%";
			else if(prob >= 0.1) prob_str_format = "%4.1f %%";
			else prob_str_format = " %3.1f %%";

			snprintf(str_buf, 10, prob_str_format, static_cast<float>(100.0 * (prob + 0.0005)));
			str_buf[9] = '\0';  //prevents _snprintf error
			int prob_width = font::line_width(str_buf, fs);
			font::draw_text_line(surf, clip_rect, fs, font::NORMAL_COLOUR, str_buf,
							 width - prob_width - 4, 2 + (fs + 2) * i, 0, TTF_STYLE_NORMAL);
		}
	}

	Uint32 battle_prediction_pane::blend_rgb(const surface& surf, unsigned char r, unsigned char g, unsigned char b, unsigned char drop)
	{
		// We simply decrement each component.
		if(r < drop) r = 0; else r -= drop;
		if(g < drop) g = 0; else g -= drop;
		if(b < drop) b = 0; else b -= drop;

		return SDL_MapRGB(surf->format, r, g, b);
	}

	// This class is used when the user clicks on the button
	// to show the "Damage Calculations" dialog.
	class attack_prediction_displayer : public gui::dialog_button_action
	{
	public:
		attack_prediction_displayer(game_display& disp, const std::vector<battle_context>& bc_vector, const gamemap& map,
								    const std::vector<team>& teams, const unit_map& units,
								    const gamestatus& status, const game_data& gamedata,
									const gamemap::location& attacker_loc, const gamemap::location& defender_loc)
				: disp_(disp), bc_vector_(bc_vector), map_(map), teams_(teams), units_(units), status_(status),
				  gamedata_(gamedata), attacker_loc_(attacker_loc), defender_loc_(defender_loc) {}

		// This method is called when the button is pressed.
		RESULT button_pressed(int selection)
		{
			// Get the selected weapon, if any.
			const size_t index = size_t(selection);

			if(index < bc_vector_.size()) {
				battle_prediction_pane battle_pane(disp_, bc_vector_[index], map_, teams_, units_, status_,
												   gamedata_, attacker_loc_, defender_loc_);
				std::vector<gui::preview_pane*> preview_panes;
				preview_panes.push_back(&battle_pane);

				gui::show_dialog(disp_, NULL, _("Damage Calculations"), "", gui::OK_ONLY, NULL, &preview_panes);
			}

			return gui::CONTINUE_DIALOG;
		}

	private:
		game_display &disp_;
		const std::vector<battle_context>& bc_vector_;
		const gamemap& map_;
	const std::vector<team>& teams_;
		const unit_map& units_;
		const gamestatus& status_;
		const game_data& gamedata_;
		const gamemap::location& attacker_loc_;
		const gamemap::location& defender_loc_;
	};
} //end anonymous namespace

mouse_handler::mouse_handler(game_display* gui, std::vector<team>& teams, unit_map& units, gamemap& map,
				gamestatus& status, const game_data& gameinfo, undo_list& undo_stack, undo_list& redo_stack, game_state& game_state):
gui_(gui), teams_(teams), units_(units), map_(map), status_(status), gameinfo_(gameinfo),
undo_stack_(undo_stack), redo_stack_(redo_stack), game_state_(game_state)
{
	minimap_scrolling_ = false;
	dragging_ = false;
	dragging_started_ = false;
	drag_from_x_ = 0;
	drag_from_y_ = 0;
	enemy_paths_ = false;
	path_turns_ = 0;
	undo_ = false;
	show_menu_ = false;
	over_route_ = false;
	team_num_ = 1;
}

void mouse_handler::set_team(const int team_number)
{
	team_num_ = team_number;
}

void mouse_handler::mouse_motion(const SDL_MouseMotionEvent& event, const bool browse)
{
	mouse_motion(event.x,event.y, browse);
}

void mouse_handler::mouse_update(const bool browse)
{
	int x, y;
	SDL_GetMouseState(&x,&y);
	mouse_motion(x, y, browse, true);
}

void mouse_handler::mouse_motion(int x, int y, const bool browse, bool update)
{
	if(minimap_scrolling_) {
		//if the game is run in a window, we could miss a LMB/MMB up event
		// if it occurs outside our window.
		// thus, we need to check if the LMB/MMB is still down
		minimap_scrolling_ = ((SDL_GetMouseState(NULL,NULL) & (SDL_BUTTON(1) | SDL_BUTTON(2))) != 0);
		if(minimap_scrolling_) {
			const gamemap::location& loc = (*gui_).minimap_location_on(x,y);
			if(loc.valid()) {
				if(loc != last_hex_) {
					last_hex_ = loc;
					(*gui_).scroll_to_tile(loc,game_display::WARP,false);
				}
			} else {
				// clicking outside of the minimap will end minimap scrolling
				minimap_scrolling_ = false;
			}
		}
		if(minimap_scrolling_) return;
	}

	const gamemap::location new_hex = (*gui_).hex_clicked_on(x,y);

	// Fire the drag & drop only after minimal drag distance
	// While we check the mouse buttons state, we also grab fresh position data.
	int mx = drag_from_x_; // some default value to prevent unlikely SDL bug
	int my = drag_from_y_;
	if (dragging_ && !dragging_started_ && (SDL_GetMouseState(&mx,&my) & SDL_BUTTON_LEFT != 0)) {
		const double drag_distance = pow(drag_from_x_- mx, 2) + pow(drag_from_y_- my, 2);
		if (drag_distance > drag_threshold*drag_threshold) {
			dragging_started_ = true;
			cursor::set_dragging(true);
		}
	}

	if(new_hex != last_hex_) {
		update = true;

		if (last_hex_.valid()) {
			// we store the previous hexes used to propose attack direction
			previous_hex_ = last_hex_;
			// the hex of the selected unit is also "free"
			if (last_hex_ == selected_hex_ || find_unit(last_hex_) == units_.end()) {
					previous_free_hex_ = last_hex_;
			}
		}
		last_hex_ = new_hex;
	}

	if (update) {
		if(new_hex.valid() == false) {
			current_route_.steps.clear();
			(*gui_).set_route(NULL);
		}

		(*gui_).highlight_hex(new_hex);

		const unit_map::iterator selected_unit = find_unit(selected_hex_);
		const unit_map::iterator mouseover_unit = find_unit(new_hex);

		// we search if there is an attack possibility and where
		gamemap::location attack_from = current_unit_attacks_from(new_hex);

		//see if we should show the normal cursor, the movement cursor, or
		//the attack cursor
		//If the cursor is on WAIT, we don't change it and let the setter
		//of this state end it
		if (cursor::get() != cursor::WAIT) {
			if(selected_unit != units_.end() && selected_unit->second.side() == team_num_
			   && !selected_unit->second.incapacitated() && !browse) {
				if (attack_from.valid()) {
					cursor::set(dragging_started_ ? cursor::ATTACK_DRAG : cursor::ATTACK);
				} else if (mouseover_unit==units_.end() && current_paths_.routes.count(new_hex)) {
					cursor::set(dragging_started_ ? cursor::MOVE_DRAG : cursor::MOVE);
				} else {
					// selecte unit can't attack or move there
					cursor::set(cursor::NORMAL);
				}
			} else {
				// no selected unit or we can't move it
				cursor::set(cursor::NORMAL);
			}
		}

		// show (or cancel) the attack direction indicator
		if (attack_from.valid() && !browse) {
			gui_->set_attack_indicator(attack_from, new_hex);
		} else {
			gui_->clear_attack_indicator();
		}

		if(enemy_paths_) {
			enemy_paths_ = false;
			current_paths_ = paths();
			gui_->unhighlight_reach();
		} else if(over_route_) {
			over_route_ = false;
			current_route_.steps.clear();
			(*gui_).set_route(NULL);
		}

		// the destination is the pointed hex or the adjacent hex
		// used to attack it
		gamemap::location dest;
		unit_map::const_iterator dest_un;
		if (attack_from.valid()) {
			dest = attack_from;
			dest_un = find_unit(dest);
		}	else {
			dest = new_hex;
			dest_un = mouseover_unit;
		}

		if(dest == selected_hex_ || dest_un != units_.end()) {
			current_route_.steps.clear();
			(*gui_).set_route(NULL);
		} else if(!current_paths_.routes.empty() && map_.on_board(selected_hex_) &&
		   map_.on_board(new_hex)) {

			if(selected_unit != units_.end() && !selected_unit->second.incapacitated()) {
				// the movement_reset is active only if it's not the unit's turn
				unit_movement_resetter move_reset(selected_unit->second,
						selected_unit->second.side() != team_num_); 
				current_route_ = get_route(selected_unit, dest, teams_[selected_unit->second.side()-1]);
				if(!browse) {
					(*gui_).set_route(&current_route_);
				}
			}
		}

		unit_map::iterator un = mouseover_unit;

		if(un != units_.end() && current_paths_.routes.empty() && !(*gui_).fogged(un->first)) {
			if (un->second.side() != team_num_) {
				//unit under cursor is not on our team, highlight reach
				unit_movement_resetter move_reset(un->second);

				const bool teleport = un->second.get_ability_bool("teleport",un->first);
				current_paths_ = paths(map_,status_,gameinfo_,units_,new_hex,teams_,
									false,teleport,viewing_team(),path_turns_);
				gui_->highlight_reach(current_paths_);
				enemy_paths_ = true;
			} else {
				//unit is on our team, show path if the unit has one
				const gamemap::location go_to = un->second.get_goto();
				if(map_.on_board(go_to)) {
					paths::route route = get_route(un, go_to, current_team());
					gui_->set_route(&route);
				}
				over_route_ = true;
			}
		}
	}
}

unit_map::iterator mouse_handler::selected_unit()
{
	unit_map::iterator res = find_unit(selected_hex_);
	if(res != units_.end()) {
		return res;
	} else {
		return find_unit(last_hex_);
	}
}

unit_map::iterator mouse_handler::find_unit(const gamemap::location& hex)
{
	return find_visible_unit(units_,hex,map_,teams_,viewing_team());
}

unit_map::const_iterator mouse_handler::find_unit(const gamemap::location& hex) const
{
	return find_visible_unit(units_,hex,map_,teams_,viewing_team());
}

gamemap::location mouse_handler::current_unit_attacks_from(const gamemap::location& loc)
{
	const unit_map::const_iterator current = find_unit(selected_hex_);
	if(current == units_.end() || current->second.side() != team_num_
		|| current->second.attacks_left()==0) {
		return gamemap::location();
	}

	const unit_map::const_iterator enemy = find_unit(loc);
	if(enemy == units_.end() || current_team().is_enemy(enemy->second.side()) == false
		|| enemy->second.incapacitated())
	{
		return gamemap::location();
	}

	const gamemap::location::DIRECTION preferred = loc.get_relative_dir(previous_hex_);
	const gamemap::location::DIRECTION second_preferred = loc.get_relative_dir(previous_free_hex_);

	int best_rating = 100;//smaller is better
	gamemap::location res;
	gamemap::location adj[6];
	get_adjacent_tiles(loc,adj);

	for(size_t n = 0; n != 6; ++n) {
		if(map_.on_board(adj[n]) == false) {
			continue;
		}

		if(adj[n] != selected_hex_ && find_unit(adj[n]) != units_.end()) {
			continue;
		}

		if(current_paths_.routes.count(adj[n])) {
			static const size_t NDIRECTIONS = gamemap::location::NDIRECTIONS;
			unsigned int difference = abs(int(preferred - n));
			if(difference > NDIRECTIONS/2) {
				difference = NDIRECTIONS - difference;
			}
			unsigned int second_difference = abs(int(second_preferred - n));
			if(second_difference > NDIRECTIONS/2) {
				second_difference = NDIRECTIONS - second_difference;
			}
			const int rating = difference * 2 + (second_difference > difference);
			if(rating < best_rating || res.valid() == false) {
				best_rating = rating;
				res = adj[n];
			}
		}
	}

	return res;
}

paths::route mouse_handler::get_route(unit_map::const_iterator un, gamemap::location go_to, team &team)
{
	// The pathfinder will check unit visibility (fogged/stealthy).
	unit u = un->second;
	const shortest_path_calculator calc(u,team,units_,teams_,map_);

	const std::set<gamemap::location>* teleports = NULL;
	std::set<gamemap::location> allowed_teleports;
	if(u.get_ability_bool("teleport",un->first)) {
		allowed_teleports = vacant_villages(team.villages(),units_);
		teleports = &allowed_teleports;
		if(team.villages().count(un->first))
			allowed_teleports.insert(un->first);

	}

	paths::route route = a_star_search(un->first, go_to, 10000.0, &calc, map_.w(), map_.h(), teleports);
	route.move_left = route_turns_to_complete(u,map_,route,units_,teams_);

	return route;
}

void mouse_handler::mouse_press(const SDL_MouseButtonEvent& event, const bool browse)
{
	show_menu_ = false;
	mouse_update(browse);
	int scrollx = 0;
	int scrolly = 0;

	if(is_left_click(event) && event.state == SDL_RELEASED) {
		minimap_scrolling_ = false;
		dragging_ = false;
		cursor::set_dragging(false);
		if (dragging_started_ && !browse && !commands_disabled) {
			left_click(event, browse);
		}
		dragging_started_= false;
	} else if(is_middle_click(event) && event.state == SDL_RELEASED) {
		minimap_scrolling_ = false;
	} else if(is_left_click(event) && event.state == SDL_PRESSED) {
		left_click(event, browse);
		if (!browse && !commands_disabled) {
			dragging_ = true;
			dragging_started_ = false;
			SDL_GetMouseState(&drag_from_x_, &drag_from_y_);
		}
	} else if(is_right_click(event) && event.state == SDL_PRESSED) {
		// The first right-click cancel the selection if any,
		// the second open the context menu
		dragging_ = false;
		dragging_started_ = false;
		cursor::set_dragging(false);
		if (selected_hex_.valid() && find_unit(selected_hex_) != units_.end()) {
			select_hex(gamemap::location(), browse);
		} else {
			gui_->draw(); // redraw highlight (and maybe some more)
			const theme::menu* const m = gui_->get_theme().context_menu();
			if (m != NULL)
				show_menu_ = true;
			else
				LOG_STREAM(warn, display) << "no context menu found...\n";
		}
	} else if(is_middle_click(event) && event.state == SDL_PRESSED) {
		// clicked on a hex on the minimap? then initiate minimap scrolling
		const gamemap::location& loc = gui_->minimap_location_on(event.x,event.y);
		minimap_scrolling_ = false;
		if(loc.valid()) {
			minimap_scrolling_ = true;
			last_hex_ = loc;
			gui_->scroll_to_tile(loc,game_display::WARP,false);
		} else {
		const SDL_Rect& rect = gui_->map_area();
		const int centerx = (rect.x + rect.w)/2;
		const int centery = (rect.y + rect.h)/2;

		const int xdisp = event.x - centerx;
		const int ydisp = event.y - centery;

		gui_->scroll(xdisp,ydisp);
		}
	} else if (event.button == SDL_BUTTON_WHEELUP) {
		scrolly = - preferences::scroll_speed();
	} else if (event.button == SDL_BUTTON_WHEELDOWN) {
		scrolly = preferences::scroll_speed();
	} else if (event.button == SDL_BUTTON_WHEELLEFT) {
		scrollx = - preferences::scroll_speed();
	} else if (event.button == SDL_BUTTON_WHEELRIGHT) {
		scrollx = preferences::scroll_speed();
	}

	if (scrollx != 0 || scrolly != 0) {
		CKey pressed;
		// Alt + mousewheel do an 90° rotation on the scroll direction
		if (pressed[SDLK_LALT] || pressed[SDLK_RALT])
			gui_->scroll(scrolly,scrollx);
		else
			gui_->scroll(scrollx,scrolly);
	}

	if (!dragging_ && dragging_started_) {
		dragging_started_ = false;
		cursor::set_dragging(false);
	}

	mouse_update(browse);

}

bool mouse_handler::is_left_click(const SDL_MouseButtonEvent& event)
{
	return event.button == SDL_BUTTON_LEFT && !command_active();
}

bool mouse_handler::is_middle_click(const SDL_MouseButtonEvent& event)
{
	return event.button == SDL_BUTTON_MIDDLE;
}

bool mouse_handler::is_right_click(const SDL_MouseButtonEvent& event)
{
	return event.button == SDL_BUTTON_RIGHT || event.button == SDL_BUTTON_LEFT && command_active();
}

void mouse_handler::left_click(const SDL_MouseButtonEvent& event, const bool browse)
{
	dragging_ = false;
	dragging_started_ = false;
	cursor::set_dragging(false);
	undo_ = false;
	bool check_shroud = teams_[team_num_ - 1].auto_shroud_updates();

	// clicked on a hex on the minimap? then initiate minimap scrolling
	const gamemap::location& loc = gui_->minimap_location_on(event.x,event.y);
	minimap_scrolling_ = false;
	if(loc.valid()) {
		minimap_scrolling_ = true;
		last_hex_ = loc;
		gui_->scroll_to_tile(loc,game_display::WARP,false);
		return;
	}

	//we use the last registered highlighted hex
	//since it's what update our global state
	gamemap::location hex = last_hex_;

	unit_map::iterator u = find_unit(selected_hex_);

	//if the unit is selected and then itself clicked on,
	//any goto command is cancelled
	if(u != units_.end() && !browse && selected_hex_ == hex && u->second.side() == team_num_) {
		u->second.set_goto(gamemap::location());
	}

	unit_map::iterator clicked_u = find_unit(hex);

	//if we can move to that tile
	std::map<gamemap::location,paths::route>::const_iterator
			route = enemy_paths_ ? current_paths_.routes.end() :
	                               current_paths_.routes.find(hex);

	const gamemap::location src = selected_hex_;
	paths orig_paths = current_paths_;
	const gamemap::location& attack_from = current_unit_attacks_from(hex);

	//see if we're trying to do a attack or move-and-attack
	if(!browse && !commands_disabled && attack_from.valid()) {
		if (attack_from == selected_hex_) { //no move needed
			if (attack_enemy(u, clicked_u) == false) {
				return;
			}
		}
		else if (move_unit_along_current_route(false)) {//move the unit without updating shroud
			// a WML event could have invalidated both attacker and defender
			// so make sure they're valid before attacking
			u = find_unit(attack_from);
			unit_map::iterator enemy = find_unit(hex);
			if(u != units_.end() && u->second.side() == team_num_ &&
				enemy != units_.end() && current_team().is_enemy(enemy->second.side()) && !enemy->second.incapacitated()) {
				//if shroud or fog is active, rememember units and after attack check if someone isn't seen
				std::set<gamemap::location> known_units;

				if (teams_[team_num_-1].uses_shroud() || teams_[team_num_-1].uses_fog()){
					 for(unit_map::const_iterator u = units_.begin(); u != units_.end(); ++u) {
				   if(teams_[team_num_-1].fogged(u->first.x,u->first.y) == false) {
					 known_units.insert(u->first);
					 teams_[team_num_-1].see(u->second.side()-1);
							}
						}
				}
				if(!commands_disabled && attack_enemy(u,enemy) == false) {
					undo_ = true;
					selected_hex_ = src;
					gui_->select_hex(src);
					current_paths_ = orig_paths;
					gui_->highlight_reach(current_paths_);
					return;
				}
				else //attack == true
				{
					if (teams_[team_num_-1].uses_shroud() || teams_[team_num_-1].uses_fog()){
						//check if some new part of map discovered or is active delay shroud updates, which need special care
						if (clear_shroud(*gui_, status_, map_, gameinfo_, units_, teams_, team_num_ - 1)||!teams_[team_num_-1].auto_shroud_updates()){
							clear_undo_stack();
							gui_->invalidate_all();
							gui_->draw();
							//some new part of map discovered
							for(unit_map::const_iterator u = units_.begin(); u != units_.end(); ++u) {
								if(teams_[team_num_-1].fogged(u->first.x,u->first.y) == false) {
									//check if unit is not known
									if (known_units.find(u->first)==known_units.end())
									{
										game_events::raise("sighted",u->first,attack_from);
									}
								 }
							}
							game_events::pump();
							return;
						}
					}
				}
			}
		}

		if(check_shroud && clear_shroud(*gui_, status_, map_, gameinfo_, units_, teams_, team_num_ - 1)) {
			clear_undo_stack();
			gui_->invalidate_all();
			gui_->draw();
		}

		return;
	}

	//otherwise we're trying to move to a hex
	else if(!commands_disabled && !browse && selected_hex_.valid() && selected_hex_ != hex &&
		     u != units_.end() && u->second.side() == team_num_ &&
		     clicked_u == units_.end() && !current_route_.steps.empty() &&
		     current_route_.steps.front() == selected_hex_) {
		move_unit_along_current_route(check_shroud);
	} else {
		// we select a (maybe empty) hex
		select_hex(hex, browse);
	}
}

void mouse_handler::select_hex(const gamemap::location& hex, const bool browse) {
	selected_hex_ = hex;
	gui_->select_hex(hex);
	gui_->clear_attack_indicator();
	gui_->set_route(NULL);

	unit_map::iterator u = find_unit(hex);
	if(hex.valid() && u != units_.end() ) {
		next_unit_ = u->first;

		// if it's not the unit's turn, we reset its moves
		unit_movement_resetter move_reset(u->second, u->second.side() != team_num_);
		const bool teleport = u->second.get_ability_bool("teleport",u->first);
		current_paths_ = paths(map_,status_,gameinfo_,units_,hex,teams_,
						   false,teleport,viewing_team(),path_turns_);
		show_attack_options(u);
		gui_->highlight_reach(current_paths_);
		// the highlight now comes from selection
		// and not from the mouseover on an enemy
		enemy_paths_ = false;
		gui_->set_route(NULL);

		// selection have impact only if we are not observing and it's our unit
		if (!browse && u->second.side() == gui_->viewing_team()+1) {
			sound::play_UI_sound("select-unit.wav");
			u->second.set_selecting(*gui_, u->first);

			game_events::fire("select", selected_hex_);
		}

	} else {
		gui_->unhighlight_reach();
		current_paths_ = paths();
		current_route_.steps.clear();
	}
}

void mouse_handler::clear_undo_stack()
{
	if(teams_[team_num_ - 1].auto_shroud_updates() == false)
		apply_shroud_changes(undo_stack_,gui_,status_,map_,gameinfo_,units_,teams_,team_num_-1);
	undo_stack_.clear();
}

bool mouse_handler::move_unit_along_current_route(bool check_shroud)
{
	const std::vector<gamemap::location> steps = current_route_.steps;
	if(steps.empty()) {
		return false;
	}

	const size_t moves = ::move_unit(gui_,gameinfo_,status_,map_,units_,teams_,
	                   steps,&recorder,&undo_stack_,&next_unit_,false,check_shroud);

	cursor::set(cursor::NORMAL);

	gui_->invalidate_game_status();

	selected_hex_ = gamemap::location();
	gui_->select_hex(gamemap::location());

	gui_->set_route(NULL);
	gui_->unhighlight_reach();
	current_paths_ = paths();

	if(moves == 0)
		return false;

	redo_stack_.clear();

	wassert(moves <= steps.size());
	const gamemap::location& dst = steps[moves-1];
	const unit_map::const_iterator u = units_.find(dst);

	//u may be equal to units_.end() in the case of e.g. a [teleport]
	if(u != units_.end()) {
		//Reselect the unit if the move was interrupted
		if(dst != steps.back()) {
			selected_hex_ = dst;
			gui_->select_hex(dst);
		}

		current_route_.steps.clear();

		//check if we are now adjacent to an enemy,
		//we reselect the unit (old 1.2.x behavior)
		gamemap::location adj[6];
		get_adjacent_tiles(dst,adj);
		for(int i = 0; i != 6; i++) {
			unit_map::iterator adj_unit = find_unit(adj[i]);
			if (adj_unit != units_.end() &&  current_team().is_enemy(adj_unit->second.side())) {
				selected_hex_ = dst;
				gui_->select_hex(dst);
				const bool teleport = u->second.get_ability_bool("teleport",u->first);
				current_paths_ = paths(map_,status_,gameinfo_,units_,dst,teams_,
									false,teleport,viewing_team(),path_turns_);
				show_attack_options(u);
				gui_->highlight_reach(current_paths_);
				break;
			}
		}
	}

	return moves == steps.size();
}

bool mouse_handler::attack_enemy(unit_map::iterator attacker, unit_map::iterator defender)
{
	try {
		return attack_enemy_(attacker, defender);
	} catch(std::bad_alloc) {
		lg::wml_error << "Memory exhausted a unit has either a lot hitpoints or a negative amount.\n";
		return false;
	}

}

bool mouse_handler::attack_enemy_(unit_map::iterator attacker, unit_map::iterator defender)
{
	//we must get locations by value instead of by references, because the iterators
	//may become invalidated later
	const gamemap::location attacker_loc = attacker->first;
	const gamemap::location defender_loc = defender->first;

	std::vector<std::string> items;

	std::vector<battle_context> bc_vector;
	unsigned int i, best = 0;
	for (i = 0; i < attacker->second.attacks().size(); i++) {
		// skip weapons with attack_weight=0
		if (attacker->second.attacks()[i].attack_weight() > 0) {
			battle_context bc(map_, teams_, units_, status_, gameinfo_, attacker->first, defender->first, i);
			bc_vector.push_back(bc);
			if (bc.better_attack(bc_vector[best], 0.5)) {
				best = i;
			}
		}
	}

	for (i = 0; i < bc_vector.size(); i++) {
		const battle_context::unit_stats& att = bc_vector[i].get_attacker_stats();
		const battle_context::unit_stats& def = bc_vector[i].get_defender_stats();
		config tmp_config;
		attack_type no_weapon(tmp_config, "fake_attack", false);
		const attack_type& attw = attack_type(*att.weapon);
		const attack_type& defw = attack_type(def.weapon ? *def.weapon : no_weapon);

		//if there is an attack special or defend special, we output a single space for the other unit, to make sure
		//that the attacks line up nicely.
		std::string special_pad = "";
		if (!attw.weapon_specials().empty() || !defw.weapon_specials().empty())
			special_pad = " ";

		std::stringstream atts;
		if (i == best) {
			atts << DEFAULT_ITEM;
		}
		atts << IMAGE_PREFIX << attw.icon() << COLUMN_SEPARATOR
			 << font::BOLD_TEXT << attw.name() << "\n" << att.damage << "-"
			 << att.num_blows << " " << (attw.range().empty() ? "" : gettext(attw.range().c_str())) << " (" << att.chance_to_hit << "%)\n"
			 << attw.weapon_specials() << special_pad
			 << COLUMN_SEPARATOR << _("vs") << COLUMN_SEPARATOR
			 << font::BOLD_TEXT << defw.name() << "\n" << def.damage << "-"
			 << def.num_blows << " " << (defw.range().empty() ? "" : gettext(defw.range().c_str())) << " (" << def.chance_to_hit << "%)\n"
			 << defw.weapon_specials() << special_pad << COLUMN_SEPARATOR
			 << IMAGE_PREFIX << defw.icon();

		items.push_back(atts.str());
	}

	//make it so that when we attack an enemy, the attacking unit
	//is again shown in the status bar, so that we can easily
	//compare between the attacking and defending unit
	gui_->highlight_hex(gamemap::location());
	gui_->draw(true,true);

	attack_prediction_displayer ap_displayer(*gui_, bc_vector, map_, teams_, units_, status_, gameinfo_, attacker_loc, defender_loc);
	std::vector<gui::dialog_button_info> buttons;
	buttons.push_back(gui::dialog_button_info(&ap_displayer, _("Damage Calculations")));

	int res = 0;

	{
		dialogs::unit_preview_pane attacker_preview(*gui_,&map_,attacker->second,dialogs::unit_preview_pane::SHOW_BASIC,true);
		dialogs::unit_preview_pane defender_preview(*gui_,&map_,defender->second,dialogs::unit_preview_pane::SHOW_BASIC,false);
		std::vector<gui::preview_pane*> preview_panes;
		preview_panes.push_back(&attacker_preview);
		preview_panes.push_back(&defender_preview);

		res = gui::show_dialog(*gui_,NULL,_("Attack Enemy"),
				_("Choose weapon:")+std::string("\n"),
				gui::OK_CANCEL,&items,&preview_panes,"",NULL,-1,NULL,-1,-1,
				NULL,&buttons);
	}

	cursor::set(cursor::NORMAL);
	if(size_t(res) < bc_vector.size()) {
		const battle_context::unit_stats &att = bc_vector[res].get_attacker_stats();
		const battle_context::unit_stats &def = bc_vector[res].get_defender_stats();

		attacker->second.set_goto(gamemap::location());
		clear_undo_stack();
		redo_stack_.clear();

		current_paths_ = paths();
		gui_->clear_attack_indicator();
		gui_->unhighlight_reach();

		gui_->draw();

		const bool defender_human = teams_[defender->second.side()-1].is_human();

		recorder.add_attack(attacker_loc,defender_loc,att.attack_num,def.attack_num);

		//MP_COUNTDOWN grant time bonus for attacking
		current_team().set_action_bonus_count(1 + current_team().action_bonus_count());

		try {
			attack(*gui_,map_,teams_,attacker_loc,defender_loc,att.attack_num,def.attack_num,units_,status_,gameinfo_);
		} catch(end_level_exception&) {
			//if the level ends due to a unit being killed, still see if
			//either the attacker or defender should advance
			dialogs::advance_unit(gameinfo_,map_,units_,attacker_loc,*gui_);
			dialogs::advance_unit(gameinfo_,map_,units_,defender_loc,*gui_,!defender_human);
			throw;
		}

		dialogs::advance_unit(gameinfo_,map_,units_,attacker_loc,*gui_);
		dialogs::advance_unit(gameinfo_,map_,units_,defender_loc,*gui_,!defender_human);

		selected_hex_ = gamemap::location();
		current_route_.steps.clear();
		gui_->set_route(NULL);

		check_victory(units_,teams_);

		gui_->draw();

		return true;
	} else {
		return false;
	}
}

void mouse_handler::show_attack_options(unit_map::const_iterator u)
{
	team& current_team = teams_[team_num_-1];

	if(u == units_.end() || u->second.attacks_left() == 0)
		return;

	for(unit_map::const_iterator target = units_.begin(); target != units_.end(); ++target) {
		if(current_team.is_enemy(target->second.side()) &&
			distance_between(target->first,u->first) == 1 && !target->second.incapacitated()) {
			current_paths_.routes[target->first] = paths::route();
		}
	}
}

bool mouse_handler::unit_in_cycle(unit_map::const_iterator it)
{
	if(it->second.side() == team_num_ && unit_can_move(it->first,units_,map_,teams_) && it->second.user_end_turn() == false && !gui_->fogged(it->first)) {
		bool is_enemy = current_team().is_enemy(int(gui_->viewing_team()+1));
		return is_enemy == false || it->second.invisible(it->first,units_,teams_) == false;
	}

	return false;

}

#define LOCAL_VARIABLES \
unit_map::const_iterator it = units_.find(next_unit_);\
const unit_map::const_iterator itx = it;\
const unit_map::const_iterator begin = units_.begin();\
const unit_map::const_iterator end = units_.end()

void mouse_handler::cycle_units(const bool browse)
{
	LOCAL_VARIABLES;

	if (it == end) {
		for (it = begin; it != end && !unit_in_cycle(it); ++it);
	} else {
		do {
			++it;
			if (it == end) it = begin;
		} while (it != itx && !unit_in_cycle(it));
	}
	if (it!=itx) {
		gui_->scroll_to_tile(it->first,game_display::WARP);
		select_hex(it->first, browse);
	} else {
		next_unit_ = gamemap::location();
	}

	mouse_update(browse);
}

void mouse_handler::cycle_back_units(const bool browse)
{
	LOCAL_VARIABLES;

	if (it == end) {
		while (it != begin && !unit_in_cycle(--it));
		if (!unit_in_cycle(it)) it = itx;
	} else {
		do {
			if (it == begin) it = end;
			--it;
		} while (it != itx && !unit_in_cycle(it));
	}

	if (it!=itx) {
		gui_->scroll_to_tile(it->first,game_display::WARP);
		select_hex(it->first, browse);
	} else {
		next_unit_ = gamemap::location();
	}

	mouse_update(browse);
}

void mouse_handler::set_current_paths(paths new_paths) {
	gui_->unhighlight_reach();
	current_paths_ = new_paths;
	current_route_.steps.clear();
	gui_->set_route(NULL);
}

}
