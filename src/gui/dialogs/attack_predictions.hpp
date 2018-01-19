/*
   Copyright (C) 2010 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "actions/attack.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "units/map.hpp"

class battle_context;

struct battle_context_unit_stats;
struct combatant;

namespace gui2
{
class drawing;

namespace dialogs
{

using hp_probability_t = std::pair<int, double>;
using hp_probability_vector = std::vector<hp_probability_t>;

class attack_predictions : public modal_dialog
{
public:
	attack_predictions(battle_context& bc, const unit& attacker, const unit& defender);

	DEFINE_SIMPLE_DISPLAY_WRAPPER(attack_predictions)

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	struct combatant_data
	{
		combatant_data(const unit& unit, const combatant& combatant, const battle_context_unit_stats& stats)
			: stats_(stats)
			, combatant_(combatant)
			, unit_(unit)
		{}

		const battle_context_unit_stats& stats_;
		const combatant& combatant_;
		const unit& unit_;
	};

	void set_data(window& window, const combatant_data& attacker, const combatant_data& defender);

	hp_probability_vector get_hitpoint_probabilities(const std::vector<double>& hp_dist);

	static const unsigned int graph_width;
	static const unsigned int graph_height;
	static const unsigned int graph_max_rows;

	void draw_hp_graph(drawing& hp_graph, const combatant_data& attacker, const combatant_data& defender);

	combatant_data attacker_data_;
	combatant_data defender_data_;
};

} // namespace dialogs
} // namespace gui2
