/*
   Copyright (C) 2009 - 2017 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * Aspect: attacks
 * @file
 */

#pragma once

#include "ai/composite/aspect.hpp"
#include "units/filter.hpp"

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {


namespace ai_default_rca {

class aspect_attacks_base : public typesafe_aspect<attacks_vector> {
public:
	aspect_attacks_base(readonly_context &context, const config &cfg, const std::string &id);


	virtual ~aspect_attacks_base() {}


	virtual void recalculate() const;


	virtual bool is_allowed_attacker(const unit& u) const = 0;
	virtual bool is_allowed_enemy(const unit& u) const = 0;


protected:
	std::shared_ptr<attacks_vector> analyze_targets() const;

	void do_attack_analysis(const map_location& loc,
	                const move_map& srcdst, const move_map& dstsrc,
			const move_map& fullmove_srcdst, const move_map& fullmove_dstsrc,
	                const move_map& enemy_srcdst, const move_map& enemy_dstsrc,
			const map_location* tiles, bool* used_locations,
	                std::vector<map_location>& units,
	                std::vector<attack_analysis>& result,
			attack_analysis& cur_analysis,
			 const team &current_team) const;
	static int rate_terrain(const unit& u, const map_location& loc);
};

class aspect_attacks : public aspect_attacks_base {
public:
	aspect_attacks(readonly_context &context, const config &cfg, const std::string &id);
	virtual ~aspect_attacks() {}

	virtual bool is_allowed_attacker(const unit& u) const;
	virtual bool is_allowed_enemy(const unit& u) const;
	virtual config to_config() const;
private:
	std::shared_ptr<unit_filter> filter_own_, filter_enemy_;
};

} // end of namespace testing_ai_default

struct aspect_attacks_lua_filter {
	lua_State* lua;
	std::shared_ptr<unit_filter> filter_own_, filter_enemy_;
	int ref_own_, ref_enemy_;
};

class aspect_attacks_lua : public ai_default_rca::aspect_attacks_base {
public:
	aspect_attacks_lua(readonly_context &context, const config &cfg, const std::string &id, std::shared_ptr<lua_ai_context>& l_ctx);
	virtual ~aspect_attacks_lua() {}

	virtual bool is_allowed_attacker(const unit& u) const;
	virtual bool is_allowed_enemy(const unit& u) const;
	virtual config to_config() const;
	virtual void recalculate() const;
private:
	std::shared_ptr<lua_ai_action_handler> handler_;
	mutable std::shared_ptr<lua_object<aspect_attacks_lua_filter> > obj_;
	std::string code_;
	const config params_;
};

} // end of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif
