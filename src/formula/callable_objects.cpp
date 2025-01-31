/*
	Copyright (C) 2014 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "formula/callable_objects.hpp"

#include "config.hpp"
#include "formula/function.hpp"
#include "map/map.hpp"
#include "display_context.hpp"
#include "team.hpp"
#include "units/attack_type.hpp"
#include "units/formula_manager.hpp"
#include "units/unit.hpp"
#include "units/types.hpp"
#include "log.hpp"
#include "recall_list_manager.hpp"
#include "deprecation.hpp"
#include "game_board.hpp"
#include "game_version.hpp"
#include "resources.hpp"
#include "tod_manager.hpp"
#include "play_controller.hpp"
#include "game_events/pump.hpp"

static lg::log_domain log_scripting_formula("scripting/formula");
#define LOG_SF LOG_STREAM(info, log_scripting_formula)
#define ERR_SF LOG_STREAM(err, log_scripting_formula)

namespace wfl
{

variant location_callable::get_value(const std::string& key) const
{
	if(key == "x") {
		return variant(loc_.wml_x());
	} else if(key == "y") {
		return variant(loc_.wml_y());
	}

	return variant();
}

void location_callable::get_inputs(formula_input_vector& inputs) const
{
	add_input(inputs, "x");
	add_input(inputs, "y");
}

int location_callable::do_compare(const formula_callable* callable) const
{
	const location_callable* loc_callable = dynamic_cast<const location_callable*>(callable);
	if(loc_callable == nullptr) {
		return formula_callable::do_compare(callable);
	}

	const map_location& other_loc = loc_callable->loc();
	return loc_.do_compare(other_loc);
}

void location_callable::serialize_to_string(std::string& str) const
{
	std::ostringstream s;
	s << "loc(" << (loc_.wml_x()) << "," << (loc_.wml_y()) << ")";
	str += s.str();
}

attack_type_callable::attack_type_callable(const attack_type& attack) : att_(attack.shared_from_this())
{
	type_ = ATTACK_TYPE_C;
}

variant attack_type_callable::get_value(const std::string& key) const
{
	if(key == "id" || key == "name") {
		return variant(att_->id());
	} else if(key == "description") {
		return variant(att_->name());
	} else if(key == "type") {
		return variant(att_->type());
	} else if(key == "icon") {
		return variant(att_->icon());
	} else if(key == "range") {
		return variant(att_->range());
	} else if(key == "alignment") {
		return variant(att_->alignment_str());
	} else if(key == "damage") {
		return variant(att_->damage());
	} else if(key == "number_of_attacks" || key == "number" || key == "num_attacks" || key == "attacks") {
		return variant(att_->num_attacks());
	} else if(key == "attack_weight") {
		return variant(att_->attack_weight(), variant::DECIMAL_VARIANT);
	} else if(key == "defense_weight") {
		return variant(att_->defense_weight(), variant::DECIMAL_VARIANT);
	} else if(key == "accuracy") {
		return variant(att_->accuracy());
	} else if(key == "parry") {
		return variant(att_->parry());
	} else if(key == "movement_used") {
		return variant(att_->movement_used());
	} else if(key == "attacks_used") {
		return variant(att_->attacks_used());
	} else if(key == "min_range") {
		return variant(att_->min_range());
	} else if(key == "max_range") {
		return variant(att_->max_range());
	} else if(key == "specials" || key == "special") {
		std::vector<variant> res;

		for(const auto [_, special_cfg] : att_->specials().all_children_view()) {
			if(!special_cfg["id"].empty()) {
				res.emplace_back(special_cfg["id"].str());
			}
		}
		return variant(res);
	}

	return variant();
}

void attack_type_callable::get_inputs(formula_input_vector& inputs) const
{
	add_input(inputs, "name");
	add_input(inputs, "type");
	add_input(inputs, "description");
	add_input(inputs, "icon");
	add_input(inputs, "range");
	add_input(inputs, "alignment");
	add_input(inputs, "damage");
	add_input(inputs, "number");
	add_input(inputs, "accuracy");
	add_input(inputs, "parry");
	add_input(inputs, "movement_used");
	add_input(inputs, "attacks_used");
	add_input(inputs, "attack_weight");
	add_input(inputs, "defense_weight");
	add_input(inputs, "min_range");
	add_input(inputs, "max_range");
	add_input(inputs, "specials");
}

int attack_type_callable::do_compare(const formula_callable* callable) const
{
	const attack_type_callable* att_callable = dynamic_cast<const attack_type_callable*>(callable);
	if(att_callable == nullptr) {
		return formula_callable::do_compare(callable);
	}

	if(att_->damage() != att_callable->att_->damage()) {
		return att_->damage() - att_callable->att_->damage();
	}

	if(att_->num_attacks() != att_callable->att_->num_attacks()) {
		return att_->num_attacks() - att_callable->att_->num_attacks();
	}

	if(att_->id() != att_callable->att_->id()) {
		return att_->id().compare(att_callable->att_->id());
	}

	if(att_->type() != att_callable->att_->type()) {
		return att_->type().compare(att_callable->att_->type());
	}

	if(att_->range() != att_callable->att_->range()) {
		return att_->range().compare(att_callable->att_->range());
	}

	if(att_->alignment_str() != att_callable->att_->alignment_str()) {
		return att_->alignment_str().compare(att_callable->att_->alignment_str());
	}

	const auto self_specials = att_->specials().all_children_range();
	const auto other_specials = att_callable->att_->specials().all_children_range();
	if(self_specials.size() != other_specials.size()) {
		return self_specials.size() < other_specials.size() ? -1 : 1;
	}
	for(std::size_t i = 0; i < self_specials.size(); ++i) {
		const auto& s = self_specials[i].cfg["id"];
		const auto& o = other_specials[i].cfg["id"];
		if(s != o) {
			return s.str().compare(o.str());
		}
	}

	return 0;
}

unit_callable::unit_callable(const unit& u) : loc_(u.get_location()), u_(u)
{
	type_ = UNIT_C;
}

variant unit_callable::get_value(const std::string& key) const
{

	if(key == "x") {
		if(loc_ == map_location::null_location()) {
			return variant();
		}

		return variant(loc_.wml_x());
	} else if(key == "y") {
		if(loc_ == map_location::null_location()) {
			return variant();
		}

		return variant(loc_.wml_y());
	} else if(key == "loc") {
		if(loc_ == map_location::null_location()) {
			return variant();
		}

		return variant(std::make_shared<location_callable>(loc_));
	} else if(key == "terrain") {
		if(loc_ == map_location::null_location()) {
			return variant();
		}
		return variant(std::make_shared<terrain_callable>(*resources::gameboard, loc_));
	} else if(key == "id") {
		return variant(u_.id());
	} else if(key == "type") {
		return variant(u_.type_id());
	} else if(key == "name") {
		return variant(u_.name());
	} else if(key == "usage") {
		return variant(u_.usage());
	} else if(key == "leader" || key == "canrecruit") {
		return variant(u_.can_recruit());
	} else if(key == "undead") {
		return variant(u_.get_state("not_living") ? 1 : 0);
	} else if(key == "attacks") {
		std::vector<variant> res;
		for(const attack_type& att : u_.attacks()) {
			res.emplace_back(std::make_shared<attack_type_callable>(att));
		}

		return variant(res);
	} else if(key == "abilities") {
		return formula_callable::convert_vector(u_.get_ability_list());
	} else if(key == "hitpoints") {
		return variant(u_.hitpoints());
	} else if(key == "max_hitpoints") {
		return variant(u_.max_hitpoints());
	} else if(key == "experience") {
		return variant(u_.experience());
	} else if(key == "max_experience") {
		return variant(u_.max_experience());
	} else if(key == "level" || key == "full") {
		// This allows writing "upkeep == full"
		return variant(u_.level());
	} else if(key == "total_movement" || key == "max_moves") {
		return variant(u_.total_movement());
	} else if(key == "movement_left" || key == "moves") {
		return variant(u_.movement_left());
	} else if(key == "attacks_left") {
		return variant(u_.attacks_left());
	} else if(key == "max_attacks") {
		return variant(u_.max_attacks());
	} else if(key == "traits") {
		return formula_callable::convert_vector(u_.get_traits_list());
	} else if(key == "advancements_taken") {
		return formula_callable::convert_vector(u_.get_advancements_list());
	} else if(key == "objects") {
		return formula_callable::convert_vector(u_.get_objects_list());
	} else if(key == "traits_count") {
		return variant(u_.traits_count());
	} else if(key == "advancements_taken_count") {
		return variant(u_.advancements_count());
	} else if(key == "objects_count") {
		return variant(u_.objects_count());
	} else if(key == "extra_recruit") {
		return formula_callable::convert_vector(u_.recruits());
	} else if(key == "advances_to") {
		return formula_callable::convert_vector(u_.advances_to());
	} else if(key == "states" || key == "status") {
		return formula_callable::convert_set(u_.get_states());
	} else if(key == "side_number") {
		return variant(u_.side());
	} else if(key == "cost") {
		return variant(u_.cost());
	} else if(key == "upkeep") {
		return variant(u_.upkeep());
	} else if(key == "loyal") {
		// So we can write "upkeep == loyal"
		return variant(0);
	} else if(key == "hidden") {
		return variant(u_.get_hidden());
	} else if(key == "petrified") {
		return variant(u_.incapacitated());
	} else if(key == "resting") {
		return variant(u_.resting());
	} else if(key == "role") {
		return variant(u_.get_role());
	} else if(key == "race") {
		return variant(u_.race()->id());
	} else if(key == "gender") {
		return variant(gender_string(u_.gender()));
	} else if(key == "variation") {
		return variant(u_.variation());
	} else if(key == "zoc") {
		return variant(u_.get_emit_zoc());
	} else if(key == "alignment") {
		return variant(unit_alignments::get_string(u_.alignment()));
	} else if(key == "facing") {
		return variant(map_location::write_direction(u_.facing()));
	} else if(key == "resistance" || key == "movement_cost" || key == "vision_cost" || key == "jamming_cost" || key == "defense") {
		const auto& mt = u_.movement_type();
		config cfg;
		bool needs_flip = false;
		if(key == "resistance") {
			mt.get_resistances().write(cfg);
			needs_flip = true;
		} else if(key == "movement_cost") {
			mt.get_movement().write(cfg);
		} else if(key == "vision_cost") {
			mt.get_vision().write(cfg);
		} else if(key == "jamming_cost") {
			mt.get_jamming().write(cfg);
		} else if(key == "defense") {
			mt.get_defense().write(cfg);
			needs_flip = true;
		}
		std::map<variant, variant> res;
		for(const auto& [key, value] : cfg.attribute_range()) {
			int val = value.to_int();
			if(needs_flip) {
				val = 100 - val;
			}
			res.emplace(variant(key), variant(val));
		}

		return variant(res);
	} else if(key == "flying") {
		return variant(u_.is_flying());
	} else if(key == "fearless") {
		return variant(u_.is_fearless());
	} else if(key == "healthy") {
		return variant(u_.is_healthy());
	} else if(key == "vars") {
		if(u_.formula_manager().formula_vars()) {
			return variant(u_.formula_manager().formula_vars());
		}

		return variant();
	} else if(key == "wml_vars") {
		return variant(std::make_shared<config_callable>(u_.variables()));
	} else if(key == "n"      || key == "s"       || key == "ne"      || key == "se"      || key == "nw" || key == "sw" ||
	          key == "lawful" || key == "neutral" || key == "chaotic" || key == "liminal" ||
	          key == "male"   || key == "female")
	{
		return variant(key);
	}

	return variant();
}

void unit_callable::get_inputs(formula_input_vector& inputs) const
{
	add_input(inputs, "x");
	add_input(inputs, "y");
	add_input(inputs, "loc");
	add_input(inputs, "terrain");
	add_input(inputs, "id");
	add_input(inputs, "type");
	add_input(inputs, "name");
	add_input(inputs, "canrecruit");
	add_input(inputs, "undead");
	add_input(inputs, "traits");
	add_input(inputs, "advancements_taken");
	add_input(inputs, "objects");
	add_input(inputs, "traits_count");
	add_input(inputs, "advancements_taken_count");
	add_input(inputs, "objects_count");
	add_input(inputs, "attacks");
	add_input(inputs, "abilities");
	add_input(inputs, "hitpoints");
	add_input(inputs, "max_hitpoints");
	add_input(inputs, "experience");
	add_input(inputs, "max_experience");
	add_input(inputs, "level");
	add_input(inputs, "moves");
	add_input(inputs, "max_moves");
	add_input(inputs, "attacks_left");
	add_input(inputs, "max_attacks");
	add_input(inputs, "side_number");
	add_input(inputs, "extra_recruit");
	add_input(inputs, "advances_to");
	add_input(inputs, "status");
	add_input(inputs, "cost");
	add_input(inputs, "usage");
	add_input(inputs, "upkeep");
	add_input(inputs, "hidden");
	add_input(inputs, "petrified");
	add_input(inputs, "resting");
	add_input(inputs, "role");
	add_input(inputs, "race");
	add_input(inputs, "gender");
	add_input(inputs, "variation");
	add_input(inputs, "zoc");
	add_input(inputs, "alignment");
	add_input(inputs, "facing");
	add_input(inputs, "resistance");
	add_input(inputs, "movement_cost");
	add_input(inputs, "vision_cost");
	add_input(inputs, "jamming_cost");
	add_input(inputs, "defense");
	add_input(inputs, "flying");
	add_input(inputs, "fearless");
	add_input(inputs, "healthy");
	add_input(inputs, "vars");
	add_input(inputs, "wml_vars");
}

int unit_callable::do_compare(const formula_callable* callable) const
{
	const unit_callable* u_callable = dynamic_cast<const unit_callable*>(callable);
	if(u_callable == nullptr) {
		return formula_callable::do_compare(callable);
	}

	return u_.underlying_id() - u_callable->u_.underlying_id();
}

variant unit_type_callable::get_value(const std::string& key) const
{
	if(key == "id") {
		return variant(u_.id());
	} else if(key == "type") {
		return variant(u_.type_name());
	} else if(key == "alignment") {
		return variant(unit_alignments::get_string(u_.alignment()));
	} else if(key == "race") {
		return variant(u_.race_id());
	} else if(key == "abilities") {
		return formula_callable::convert_vector(u_.get_ability_list());
	} else if(key == "traits") {
		std::vector<variant> res;
		for(const auto& config : u_.possible_traits()) {
			res.emplace_back(config["id"].str());
		}

		return variant(res);
	} else if(key == "attacks") {
		std::vector<variant> res;
		for(const attack_type& att : u_.attacks()) {
			res.emplace_back(std::make_shared<attack_type_callable>(att));
		}

		return variant(res);
	} else if(key == "hitpoints" || key == "max_hitpoints") {
		return variant(u_.hitpoints());
	} else if(key == "experience" || key == "max_experience") {
		return variant(u_.experience_needed(true));
	} else if(key == "level") {
		return variant(u_.level());
	} else if(key == "total_movement" || key == "max_moves" || key == "moves") {
		return variant(u_.movement());
	} else if(key == "undead") {
		return variant(u_.musthave_status("unpoisonable") && u_.musthave_status("undrainable") && u_.musthave_status("unplagueable"));
	} else if(key == "unpoisonable") {
		return variant(u_.musthave_status("unpoisonable"));
	} else if(key == "unslowable") {
		return variant(u_.musthave_status("unslowable"));
	} else if(key == "unpetrifiable") {
		return variant(u_.musthave_status("unpetrifiable"));
	} else if(key == "undrainable") {
		return variant(u_.musthave_status("undrainable"));
	} else if(key == "unplagueable") {
		return variant(u_.musthave_status("unplagueable"));
	} else if(key == "cost") {
		return variant(u_.cost());
	} else if(key == "recall_cost") {
		return variant(u_.recall_cost());
	} else if(key == "usage") {
		return variant(u_.usage());
	}

	return variant();
}

void unit_type_callable::get_inputs(formula_input_vector& inputs) const
{
	add_input(inputs, "id");
	add_input(inputs, "type");
	add_input(inputs, "race");
	add_input(inputs, "alignment");
	add_input(inputs, "abilities");
	add_input(inputs, "traits");
	add_input(inputs, "attacks");
	add_input(inputs, "hitpoints");
	add_input(inputs, "experience");
	add_input(inputs, "level");
	add_input(inputs, "total_movement");
	add_input(inputs, "undead");
	add_input(inputs, "cost");
	add_input(inputs, "recall_cost");
	add_input(inputs, "usage");
}

int unit_type_callable::do_compare(const formula_callable* callable) const
{
	const unit_type_callable* u_callable = dynamic_cast<const unit_type_callable*>(callable);
	if(u_callable == nullptr) {
		return formula_callable::do_compare(callable);
	}

	return u_.id().compare(u_callable->u_.id());
}

struct fai_variant_visitor
#ifdef USING_BOOST_VARIANT
	: public boost::static_visitor<variant>
#endif
{
	variant operator()(bool b) const               { return variant(b ? 1 : 0); }
	variant operator()(int i) const                { return variant(i); }
	variant operator()(unsigned long long i) const { return variant(i); }
	variant operator()(double i) const             { return variant(i * 1000, variant::DECIMAL_VARIANT); }
	// TODO: Should comma-separated lists of stuff be returned as a list?
	// The challenge is to distinguish them from ordinary strings that happen to contain a comma
	// (or should we assume that such strings will be translatable?).
	variant operator()(const std::string& s) const { return variant(s); }
	variant operator()(const t_string& s) const    { return variant(s.str()); }
	variant operator()(utils::monostate) const         { return variant(); }
};

variant config_callable::get_value(const std::string& key) const
{
	if(cfg_.has_attribute(key)) {
		return cfg_[key].apply_visitor(fai_variant_visitor());
	} else if(cfg_.has_child(key)) {
		std::vector<variant> result;
		for(const auto& child : cfg_.child_range(key)) {
			result.emplace_back(std::make_shared<config_callable>(child));
		}

		return variant(result);
	} else if(key == "__all_children") {
		std::vector<variant> result;
		for(const auto [child_key, child_cfg] : cfg_.all_children_view()) {
			const variant cfg_child(std::make_shared<config_callable>(child_cfg));
			const variant kv(std::make_shared<key_value_pair>(variant(child_key), cfg_child));
			result.push_back(kv);
		}

		return variant(result);
	} else if(key == "__children") {
		std::map<std::string, std::vector<variant>> build;
		for(const auto [child_key, child_cfg] : cfg_.all_children_view()) {
			const variant cfg_child(std::make_shared<config_callable>(child_cfg));
			build[child_key].push_back(cfg_child);
		}

		std::map<variant,variant> result;
		for(auto& p : build) {
			result[variant(p.first)] = variant(p.second);
		}

		return variant(result);
	} else if(key == "__attributes") {
		std::map<variant,variant> result;
		for(const auto& [key, value] : cfg_.attribute_range()) {
			result[variant(key)] = value.apply_visitor(fai_variant_visitor());
		}

		return variant(result);
	}

	return variant();
}

void config_callable::get_inputs(formula_input_vector& inputs) const
{
	add_input(inputs, "__all_children");
	add_input(inputs, "__children");
	add_input(inputs, "__attributes");

	for(const auto& [key, _] : cfg_.attribute_range()) {
		if(key.find_first_not_of(formula::id_chars) != std::string::npos) {
			add_input(inputs, key);
		}
	}
}

int config_callable::do_compare(const formula_callable* callable) const
{
	const config_callable* cfg_callable = dynamic_cast<const config_callable*>(callable);
	if(cfg_callable == nullptr) {
		return formula_callable::do_compare(callable);
	}

	if(cfg_ == cfg_callable->get_config()) {
		return 0;
	}

	return cfg_.hash().compare(cfg_callable->get_config().hash());
}

terrain_callable::terrain_callable(const display_context& dc, const map_location& loc) : loc_(loc), t_(dc.map().get_terrain_info(loc)), owner_(dc.village_owner(loc))
{
	type_ = TERRAIN_C;
}

variant terrain_callable::get_value(const std::string& key) const
{
	if(key == "x") {
		return variant(loc_.wml_x());
	} else if(key == "y") {
		return variant(loc_.wml_y());
	} else if(key == "loc") {
		return variant(std::make_shared<location_callable>(loc_));
	} else if(key == "id") {
		return variant(std::string(t_.id()));
	} else if(key == "name") {
		return variant(t_.name());
	} else if(key == "editor_name") {
		return variant(t_.editor_name());
	} else if(key == "description") {
		return variant(t_.description());
	} else if(key == "icon") {
		return variant(t_.icon_image());
	} else if(key == "light") {
		return variant(t_.light_bonus(0));
	} else if(key == "village") {
		return variant(t_.is_village());
	} else if(key == "castle") {
		return variant(t_.is_castle());
	} else if(key == "keep") {
		return variant(t_.is_keep());
	} else if(key == "healing") {
		return variant(t_.gives_healing());
	} else if(key == "owner_side") {
		return variant(owner_);
	}

	return variant();
}

void terrain_callable::get_inputs(formula_input_vector& inputs) const
{
	add_input(inputs, "x");
	add_input(inputs, "y");
	add_input(inputs, "loc");
	add_input(inputs, "id");
	add_input(inputs, "name");
	add_input(inputs, "editor_name");
	add_input(inputs, "description");
	add_input(inputs, "icon");
	add_input(inputs, "light");
	add_input(inputs, "village");
	add_input(inputs, "castle");
	add_input(inputs, "keep");
	add_input(inputs, "healing");
	add_input(inputs, "owner_side");
}

int terrain_callable::do_compare(const formula_callable* callable) const
{
	const terrain_callable* terr_callable = dynamic_cast<const terrain_callable*>(callable);
	if(terr_callable == nullptr) {
		return formula_callable::do_compare(callable);
	}

	const map_location& other_loc = terr_callable->loc_;
	return loc_.do_compare(other_loc);
}

const gamemap& gamemap_callable::get_gamemap() const {
	return board_.map();
}

void gamemap_callable::get_inputs(formula_input_vector& inputs) const
{
	add_input(inputs, "w");
	add_input(inputs, "h");
}

variant gamemap_callable::get_value(const std::string& key) const
{
	if(key == "terrain") {
		int w = get_gamemap().w();
		int h = get_gamemap().h();

		std::vector<variant> vars;
		for(int i = 0; i < w; i++) {
			for(int j = 0; j < h; j++) {
				const map_location loc(i, j);
				vars.emplace_back(std::make_shared<terrain_callable>(board_, loc));
			}
		}

		return variant(vars);
	} else if(key == "gamemap") {
		int w = get_gamemap().w();
		int h = get_gamemap().h();

		std::map<variant, variant> vars;
		for(int i = 0; i < w; i++) {
			for(int j = 0; j < h; j++) {
				const map_location loc(i, j);
				vars.emplace(std::make_shared<location_callable>(loc), std::make_shared<terrain_callable>(board_, loc));
			}
		}

		return variant(vars);
	} else if(key == "w") {
		return variant(get_gamemap().w());
	} else if(key == "h") {
		return variant(get_gamemap().h());
	} else {
		return variant();
	}
}

void team_callable::get_inputs(formula_input_vector& inputs) const
{
	add_input(inputs, "side_number");
	add_input(inputs, "id");
	add_input(inputs, "gold");
	add_input(inputs, "start_gold");
	add_input(inputs, "base_income");
	add_input(inputs, "total_income");
	add_input(inputs, "village_gold");
	add_input(inputs, "village_support");
	add_input(inputs, "recall_cost");
	add_input(inputs, "is_human");
	add_input(inputs, "is_ai");
	add_input(inputs, "is_network");
	add_input(inputs, "fog");
	add_input(inputs, "shroud");
	add_input(inputs, "hidden");
	add_input(inputs, "flag");
	add_input(inputs, "flag_icon");
	add_input(inputs, "team_name");
	add_input(inputs, "faction");
	add_input(inputs, "faction_name");
	add_input(inputs, "color");
	add_input(inputs, "share_vision");
	add_input(inputs, "carryover_bonus");
	add_input(inputs, "carryover_percentage");
	add_input(inputs, "carryover_add");
	add_input(inputs, "recruit");
	add_input(inputs, "wml_vars");
}

variant team_callable::get_value(const std::string& key) const
{
	if(key == "side") {
		deprecated_message("team.side", DEP_LEVEL::INDEFINITE, version_info("1.17"), "Use side_number instead.");
		return variant(team_.side());
	} else if(key == "side_number") {
		return variant(team_.side());
	} else if(key == "id") {
		return variant(team_.save_id());
	} else if(key == "save_id") {
		return variant(team_.save_id());
	} else if(key == "gold") {
		return variant(team_.gold());
	} else if(key == "start_gold") {
		return variant(team_.start_gold());
	} else if(key == "base_income") {
		return variant(team_.base_income());
	} else if(key == "total_income") {
		return variant(team_.total_income());
	} else if(key == "village_gold") {
		return variant(team_.village_gold());
	} else if(key == "village_support") {
		return variant(team_.village_support());
	} else if(key == "recall_cost") {
		return variant(team_.recall_cost());
	} else if(key == "is_human") {
		return variant(team_.is_local_human());
	} else if(key == "is_ai") {
		return variant(team_.is_local_ai());
	} else if(key == "is_network") {
		return variant(team_.is_network());
	} else if(key == "fog") {
		return variant(team_.uses_fog());
	} else if(key == "shroud") {
		return variant(team_.uses_shroud());
	} else if(key == "hidden") {
		return variant(team_.hidden());
	} else if(key == "flag") {
		return variant(team_.flag());
	} else if(key == "flag_icon") {
		return variant(team_.flag_icon());
	} else if(key == "team_name") {
		return variant(team_.team_name());
	} else if(key == "faction") {
		return variant(team_.faction());
	} else if(key == "faction_name") {
		return variant(team_.faction_name());
	} else if(key == "color") {
		return variant(team_.color());
	} else if(key == "share_vision") {
		return variant(team_shared_vision::get_string(team_.share_vision()));
	} else if(key == "carryover_bonus") {
		return variant(team_.carryover_bonus(), variant::DECIMAL_VARIANT);
	} else if(key == "carryover_percentage") {
		return variant(team_.carryover_percentage());
	} else if(key == "carryover_add") {
		return variant(team_.carryover_add());
	} else if(key == "recruit") {
		std::vector<variant> result;
		for(const auto& recruit : team_.recruits()) {
			result.emplace_back(recruit);
		}
		return variant(result);
	} else if(key == "recall") {
		std::vector<variant> result;
		for(const auto& u : team_.recall_list()) {
			result.push_back(std::make_shared<unit_callable>(*u));
		}
		return variant(result);
	} else if(key == "wml_vars") {
		return variant(std::make_shared<config_callable>(team_.variables()));
	}

	return variant();
}

variant set_var_callable::get_value(const std::string& key) const
{
	if(key == "key") {
		return variant(key_);
	} else if(key == "value") {
		return value_;
	}

	return variant();
}

void set_var_callable::get_inputs(formula_input_vector& inputs) const
{
	add_input(inputs, "key");
	add_input(inputs, "value");
}

variant set_var_callable::execute_self(variant ctxt)
{
	//if(infinite_loop_guardian_.set_var_check()) {
	if(auto obj = ctxt.try_convert<formula_callable>()) {
		LOG_SF << "Setting variable: " << key_ << " -> " << value_.to_debug_string();
		obj->mutate_value(key_, value_);
		return variant(true);
	}
	//}
	//too many calls in a row - possible infinite loop
	ERR_SF << "ERROR #" << 5001 << " while executing 'set_var' formula function";

	return variant(std::make_shared<safe_call_result>(fake_ptr(), 5001));
}

variant safe_call_callable::get_value(const std::string& key) const
{
	if(key == "main") {
		return variant(main_);
	} else if(key == "backup") {
		return variant(backup_);
	}

	return variant();
}

void safe_call_callable::get_inputs(formula_input_vector& inputs) const
{
	add_input(inputs, "main");
	add_input(inputs, "backup");
}

variant safe_call_callable::execute_self(variant ctxt)
{
	variant res;
	if(auto action = main_.try_convert<action_callable>()) {
		res = action->execute_self(ctxt);
	}

	if(res.try_convert<safe_call_result>()) {
		/* If we have safe_call formula and either an error occurred, or the current action
		 * was not recognized, then evaluate backup formula from safe_call and execute it
		 * during the next loop
		 */

		map_formula_callable callable(ctxt.as_callable());
		callable.add("error", res);

		/* Store the result in safe_call_callable in case we would like to display it to the user,
		 * for example if this formula was executed from the commandline.
		 */
		backup_ = get_backup()->evaluate(callable);
		ctxt.execute_variant(backup_);
	}
	return variant(true);
}

variant safe_call_result::get_value(const std::string& key) const
{
	if(key == "status") {
		return variant(status_);
	} else if(key == "object") {
		if(failed_callable_) {
			return variant(failed_callable_);
		}

		return variant();
	} else if(key == "current_loc" && current_unit_location_ != map_location()) {
		return variant(std::make_shared<location_callable>(current_unit_location_));
	}

	return variant();
}

void safe_call_result::get_inputs(formula_input_vector& inputs) const
{
	add_input(inputs, "status");
	add_input(inputs, "object");

	if(current_unit_location_ != map_location()) {
		add_input(inputs, "current_loc");
	}
}

void gamestate_callable::get_inputs(formula_input_vector &inputs) const
{
	add_input(inputs, "turn_number");
	add_input(inputs, "time_of_day");
	add_input(inputs, "side_number");
	add_input(inputs, "sides");
	add_input(inputs, "units");
	add_input(inputs, "map");
}

variant gamestate_callable::get_value(const std::string &key) const
{
	if(key == "turn_number") {
		return variant(resources::tod_manager->turn());
	} else if(key == "time_of_day") {
		return variant(resources::tod_manager->get_time_of_day().id);
	} else if(key == "side_number") {
		return variant(resources::controller->current_side());
	} else if(key == "sides") {
		std::vector<variant> vars;
		for(const auto& team : resources::gameboard->teams()) {
			vars.emplace_back(std::make_shared<team_callable>(team));
		}
		return variant(vars);
	} else if(key == "units") {
		std::vector<variant> vars;
		for(const auto& unit : resources::gameboard->units()) {
			vars.emplace_back(std::make_shared<unit_callable>(unit));
		}
		return variant(vars);
	} else if(key == "map") {
		return variant(std::make_shared<gamemap_callable>(*resources::gameboard));
	}

	return variant();
}

void event_callable::get_inputs(formula_input_vector &inputs) const
{
	add_input(inputs, "event");
	add_input(inputs, "event_id");
	add_input(inputs, "event_data");
	add_input(inputs, "loc");
	add_input(inputs, "unit");
	add_input(inputs, "weapon");
	add_input(inputs, "second_loc");
	add_input(inputs, "second_unit");
	add_input(inputs, "second_weapon");
}

variant event_callable::get_value(const std::string &key) const
{
	if(key == "event") {
		return variant(event_info.name);
	} else if(key == "event_id") {
		return variant(event_info.id);
	} else if(key == "loc") {
		return variant(std::make_shared<location_callable>(event_info.loc1));
	} else if(key == "second_loc") {
		return variant(std::make_shared<location_callable>(event_info.loc2));
	} else if(key == "event_data") {
		return variant(std::make_shared<config_callable>(event_info.data));
	} else if(key == "unit") {
		if(auto u1 = event_info.loc1.get_unit()) {
			return variant(std::make_shared<unit_callable>(*u1));
		}
	} else if(key == "second_unit") {
		if(auto u2 = event_info.loc2.get_unit()) {
			return variant(std::make_shared<unit_callable>(*u2));
		}
	} else if(key == "weapon") {
		if(event_info.data.has_child("first")) {
			first_weapon = std::make_shared<attack_type>(event_info.data.mandatory_child("first"));
			return variant(std::make_shared<attack_type_callable>(*first_weapon));
		}
	} else if(key == "second_weapon") {
		if(event_info.data.has_child("second")) {
			second_weapon = std::make_shared<attack_type>(event_info.data.mandatory_child("second"));
			return variant(std::make_shared<attack_type_callable>(*second_weapon));
		}
	}

	return variant();
}

void color_callable::get_inputs(formula_input_vector& inputs) const
{
	add_input(inputs, "red");
	add_input(inputs, "green");
	add_input(inputs, "blue");
	add_input(inputs, "alpha");
}

variant color_callable::get_value(const std::string& key) const
{
	if(key == "red") {
		return variant(clr_.r);
	} else if(key == "green") {
		return variant(clr_.g);
	} else if(key == "blue") {
		return variant(clr_.b);
	} else if(key == "alpha") {
		return variant(clr_.a);
	}

	return variant();
}

} // namespace wfl
