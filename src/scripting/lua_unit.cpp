/*
	Copyright (C) 2009 - 2024
	by Guillaume Melquiond <guillaume.melquiond@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "scripting/lua_unit.hpp"

#include "formatter.hpp"
#include "game_board.hpp"
#include "log.hpp"
#include "map/location.hpp"             // for map_location
#include "map/map.hpp"
#include "resources.hpp"
#include "scripting/lua_attributes.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_unit_attacks.hpp"
#include "scripting/push_check.hpp"
#include "units/unit.hpp"
#include "units/map.hpp"
#include "units/animation_component.hpp"
#include "utils/optional_fwd.hpp"
#include "game_version.hpp"
#include "deprecation.hpp"
#include <vector>

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

static const char getunitKey[] = "unit";
static const char ustatusKey[] = "unit status";
static const char unitvarKey[] = "unit variables";

lua_unit::~lua_unit()
{
}

unit* lua_unit::get() const
{
	if (ptr) return ptr.get();
	if (c_ptr) return c_ptr;
	if (side) {
		return resources::gameboard->get_team(side).recall_list().find_if_matches_underlying_id(uid).get();
	}
	unit_map::unit_iterator ui = resources::gameboard->units().find(uid);
	if (!ui.valid()) return nullptr;
	return ui.get_shared_ptr().get(); //&*ui would not be legal, must get new shared_ptr by copy ctor because the unit_map itself is holding a boost shared pointer.
}
unit_ptr lua_unit::get_shared() const
{
	if (ptr) return ptr;
	if (side) {
		return resources::gameboard->get_team(side).recall_list().find_if_matches_underlying_id(uid);
	}
	unit_map::unit_iterator ui = resources::gameboard->units().find(uid);
	if (!ui.valid()) return unit_ptr();
	return ui.get_shared_ptr(); //&*ui would not be legal, must get new shared_ptr by copy ctor because the unit_map itself is holding a boost shared pointer.
}

// Having this function here not only simplifies other code, it allows us to move
// pointers around from one structure to another.
// This makes bare pointer->map in particular about 2 orders of magnitude faster,
// as benchmarked from Lua code.
bool lua_unit::put_map(const map_location &loc)
{
	if (ptr) {
		auto [unit_it, success] = resources::gameboard->units().replace(loc, ptr);

		if(success) {
			ptr.reset();
			uid = unit_it->underlying_id();
		} else {
			ERR_LUA << "Could not move unit " << ptr->underlying_id() << " onto map location " << loc;
			return false;
		}
	} else if (side) { // recall list
		unit_ptr it = resources::gameboard->get_team(side).recall_list().extract_if_matches_underlying_id(uid);
		if (it) {
			side = 0;
			// uid may be changed by unit_map on insertion
			uid = resources::gameboard->units().replace(loc, it).first->underlying_id();
		} else {
			ERR_LUA << "Could not find unit " << uid << " on recall list of side " << side;
			return false;
		}
	} else { // on map
		unit_map::unit_iterator ui = resources::gameboard->units().find(uid);
		if (ui != resources::gameboard->units().end()) {
			map_location from = ui->get_location();
			if (from != loc) { // This check is redundant in current usage
				resources::gameboard->units().erase(loc);
				resources::gameboard->units().move(from, loc);
			}
			// No need to change our contents
		} else {
			ERR_LUA << "Could not find unit " << uid << " on the map";
			return false;
		}
	}
	return true;
}

bool luaW_isunit(lua_State* L, int index)
{
	return luaL_testudata(L, index,getunitKey) != nullptr;
}

enum {
	LU_OK,
	LU_NOT_UNIT,
	LU_NOT_ON_MAP,
	LU_NOT_VALID,
};

static lua_unit* internal_get_unit(lua_State *L, int index, bool only_on_map, int& error)
{
	error = LU_OK;
	if(!luaW_isunit(L, index)) {
		error = LU_NOT_UNIT;
		return nullptr;
	}
	lua_unit* lu = static_cast<lua_unit*>(lua_touserdata(L, index));
	if(only_on_map && !lu->on_map()) {
		error = LU_NOT_ON_MAP;
	}
	if(!lu->get()) {
		error = LU_NOT_VALID;
	}
	return lu;
}

unit* luaW_tounit(lua_State *L, int index, bool only_on_map)
{
	int error;
	lua_unit* lu = internal_get_unit(L, index, only_on_map, error);
	if(error != LU_OK) {
		return nullptr;
	}
	return lu->get();
}

unit_ptr luaW_tounit_ptr(lua_State *L, int index, bool only_on_map)
{
	int error;
	lua_unit* lu = internal_get_unit(L, index, only_on_map, error);
	if(error != LU_OK) {
		return nullptr;
	}
	return lu->get_shared();
}

lua_unit* luaW_tounit_ref(lua_State *L, int index)
{
	int error;
	return internal_get_unit(L, index, false, error);
}

static void unit_show_error(lua_State *L, int index, int error)
{
	switch(error) {
		case LU_NOT_UNIT:
			luaW_type_error(L, index, "unit");
			break;
		case LU_NOT_VALID:
			luaL_argerror(L, index, "unit not found");
			break;
		case LU_NOT_ON_MAP:
			luaL_argerror(L, index, "unit not found on map");
			break;
	}
}

unit_ptr luaW_checkunit_ptr(lua_State *L, int index, bool only_on_map)
{
	int error;
	lua_unit* lu = internal_get_unit(L, index, only_on_map, error);
	unit_show_error(L, index, error);
	return lu->get_shared();
}

unit& luaW_checkunit(lua_State *L, int index, bool only_on_map)
{
	int error;
	lua_unit* lu = internal_get_unit(L, index, only_on_map, error);
	unit_show_error(L, index, error);
	return *lu->get();
}

lua_unit* luaW_checkunit_ref(lua_State *L, int index)
{
	int error;
	lua_unit* lu = internal_get_unit(L, index, false, error);
	unit_show_error(L, index, error);
	return lu;
}

void lua_unit::setmetatable(lua_State *L)
{
	luaL_setmetatable(L, getunitKey);
}

lua_unit* luaW_pushlocalunit(lua_State *L, unit& u)
{
	lua_unit* res = new(L) lua_unit(u);
	lua_unit::setmetatable(L);
	return res;
}

/**
 * Destroys a unit object before it is collected (__gc metamethod).
 */
static int impl_unit_collect(lua_State *L)
{
	lua_unit *u = static_cast<lua_unit *>(lua_touserdata(L, 1));
	u->lua_unit::~lua_unit();
	return 0;
}

/**
 * Checks two lua proxy units for equality. (__eq metamethod)
 */
static int impl_unit_equality(lua_State* L)
{
	unit& left = luaW_checkunit(L, 1);
	unit& right = luaW_checkunit(L, 2);
	const bool equal = left.underlying_id() == right.underlying_id();
	lua_pushboolean(L, equal);
	return 1;
}

/**
 * Turns a lua proxy unit to string. (__tostring metamethod)
 */
static int impl_unit_tostring(lua_State* L)
{
	const lua_unit* lu = luaW_tounit_ref(L, 1);
	unit* u = lu->get();
	std::ostringstream str;

	str << "unit: <";
	if(!u) {
		str << "invalid";
	} else if(!u->id().empty()) {
		str << u->id() << " ";
	} else {
		str << u->type_id() << " ";
	}
	if(u) {
		if(int side = lu->on_recall_list()) {
			str << "at (side " << side << " recall list)";
		} else {
			if(!lu->on_map()) {
				str << "private ";
			}
			str << "at (" << u->get_location() << ")";
		}
	}
	str << '>';

	lua_push(L, str.str());
	return 1;
}

#define UNIT_GETTER(name, type) LATTR_GETTER(name, type, unit, u)
#define UNIT_SETTER(name, type) LATTR_SETTER(name, type, unit, u)
luaW_Registry unitReg{"wesnoth", "units", getunitKey};

template<> struct lua_object_traits<lua_unit*> {
	inline static auto metatable = getunitKey;
	inline static lua_unit* get(lua_State* L, int n) {
		auto lu = luaW_tounit_ref(L, n);
		if(!lu) unit_show_error(L, n, LU_NOT_UNIT);
		return lu;
	}
};

template<> struct lua_object_traits<unit> {
	inline static auto metatable = getunitKey;
	inline static unit& get(lua_State* L, int n) {
		return luaW_checkunit(L, n);
	}
};

static void handle_unit_move(lua_State* L, lua_unit* lu, map_location dst) {
	if(!lu->on_map()) {
		(*lu)->set_location(dst);
	} else {
		unit& u = *lu->get();

		// Handle moving an on-map unit
		game_board* gb = resources::gameboard;

		if(!gb) {
			return;
		}

		map_location src = u.get_location();

		// TODO: could probably be relegated to a helper function.
		if(src != dst) {
			// If the dst isn't on the map, the unit will be clobbered. Guard against that.
			if(!gb->map().on_board(dst)) {
				std::string err_msg = formatter() << "destination hex not on map (excluding border): " << dst;
				return void(luaL_argerror(L, 2, err_msg.c_str()));
			}

			auto [unit_iterator, success] = gb->units().move(src, dst);

			if(success) {
				unit_iterator->anim_comp().set_standing();
			}
		}
	}
}

LATTR_GETTER("valid", utils::optional<std::string>, lua_unit*, lu) {
	const unit* pu = lu->get();
	if(!pu) {
		return utils::nullopt;
	}
	using namespace std::literals;
	if(lu->on_map()) {
		return "map"s;
	} else if(lu->on_recall_list()) {
		return "recall"s;
	}
	return "private"s;
}

UNIT_GETTER("x", int) {
	return u.get_location().wml_x();
}

LATTR_SETTER("x", int, lua_unit*, lu) {
	if(!lu->get()) return;
	map_location loc = (*lu)->get_location();
	loc.set_wml_x(value);
	handle_unit_move(L, lu, loc);
}

UNIT_GETTER("y", int) {
	return u.get_location().wml_y();
}

LATTR_SETTER("y", int, lua_unit*, lu) {
	if(!lu->get()) return;
	map_location loc = (*lu)->get_location();
	loc.set_wml_y(value);
	handle_unit_move(L, lu, loc);
}

UNIT_GETTER("loc", map_location) {
	return u.get_location();
}

LATTR_SETTER("loc", map_location, lua_unit*, lu) {
	if(!lu->get()) return;
	handle_unit_move(L, lu, value);
}

UNIT_GETTER("goto", map_location) {
	return u.get_goto();
}

UNIT_SETTER("goto", map_location) {
	u.set_goto(value);
}

UNIT_GETTER("side", int) {
	return u.side();
}

UNIT_SETTER("side", int) {
	u.set_side(value);
}

UNIT_GETTER("id", std::string) {
	return u.id();
}

LATTR_SETTER("id", std::string, lua_unit*, lu) {
	if(!lu->get()) return;
	if(lu->on_map()) luaL_argerror(L, 3, "can't modify id of on-map unit");
	(*lu)->set_id(value);
}

UNIT_GETTER("type", std::string) {
	return u.type_id();
}

UNIT_GETTER("image_mods", std::string) {
	return u.effect_image_mods();
}

UNIT_GETTER("usage", std::string) {
	return u.usage();
}

UNIT_SETTER("usage", std::string) {
	u.set_usage(value);
}

UNIT_GETTER("ellipse", std::string) {
	return u.image_ellipse();
}

UNIT_SETTER("ellipse", std::string) {
	u.set_image_ellipse(value);
}

UNIT_GETTER("halo", std::string) {
	return u.image_halo();
}

UNIT_SETTER("halo", std::string) {
	u.set_image_halo(value);
}

UNIT_GETTER("hitpoints", int) {
	return u.hitpoints();
}

UNIT_SETTER("hitpoints", int) {
	u.set_hitpoints(value);
}

UNIT_GETTER("max_hitpoints", int) {
	return u.max_hitpoints();
}

UNIT_SETTER("max_hitpoints", int) {
	u.set_max_hitpoints(value);
}

UNIT_GETTER("experience", int) {
	return u.experience();
}

UNIT_SETTER("experience", int) {
	u.set_experience(value);
}

UNIT_GETTER("max_experience", int) {
	return u.max_experience();
}

UNIT_SETTER("max_experience", int) {
	u.set_max_experience(value);
}

UNIT_GETTER("recall_cost", int) {
	return u.recall_cost();
}

UNIT_SETTER("recall_cost", int) {
	u.set_recall_cost(value);
}

UNIT_GETTER("moves", int) {
	return u.movement_left();
}

UNIT_SETTER("moves", int) {
	u.set_movement(value);
}

UNIT_GETTER("max_moves", int) {
	return u.total_movement();
}

UNIT_SETTER("max_moves", int) {
	u.set_total_movement(value);
}

UNIT_GETTER("max_attacks", int) {
	return u.max_attacks();
}

UNIT_SETTER("max_attacks", int) {
	u.set_max_attacks(value);
}

UNIT_GETTER("attacks_left", int) {
	return u.attacks_left();
}

UNIT_SETTER("attacks_left", int) {
	u.set_attacks(value);
}

UNIT_GETTER("vision", int) {
	return u.vision();
}

UNIT_GETTER("jamming", int) {
	return u.jamming();
}

UNIT_GETTER("name", t_string) {
	return u.name();
}

UNIT_SETTER("name", t_string) {
	u.set_name(value);
}

UNIT_GETTER("description",  t_string) {
	return u.unit_description();
}

UNIT_SETTER("description",  t_string) {
	u.set_unit_description(value);
}

UNIT_GETTER("canrecruit", bool) {
	return u.can_recruit();
}

UNIT_SETTER("canrecruit", bool) {
	u.set_can_recruit(value);
}

UNIT_GETTER("renamable", bool) {
	return !u.unrenamable();
}

UNIT_SETTER("renamable", bool) {
	u.set_unrenamable(!value);
}

UNIT_GETTER("level", int) {
	return u.level();
}

UNIT_SETTER("level", int) {
	u.set_level(value);
}

UNIT_GETTER("cost", int) {
	return u.cost();
}

UNIT_GETTER("extra_recruit", std::vector<std::string>) {
	return u.recruits();
}

UNIT_SETTER("extra_recruit", std::vector<std::string>) {
	u.set_recruits(value);
}

UNIT_GETTER("advances_to", std::vector<std::string>) {
	return u.advances_to();
}

UNIT_SETTER("advances_to", std::vector<std::string>) {
	u.set_advances_to(value);
}

UNIT_GETTER("alignment", std::string) {
	return unit_alignments::get_string(u.alignment());
}

UNIT_SETTER("alignment", lua_index_raw) {
	auto alignment = unit_alignments::get_enum(lua_check<std::string_view>(L, value.index));
	if(!alignment) luaL_argerror(L, value.index, "invalid unit alignment");
	u.set_alignment(*alignment);
}

UNIT_GETTER("upkeep", lua_index_raw) {
	unit::upkeep_t upkeep = u.upkeep_raw();

	// Need to keep these separate in order to ensure an int value is always used if applicable.
	if(int* v = utils::get_if<int>(&upkeep)) {
		lua_push(L, *v);
	} else {
		const std::string type = utils::visit(unit::upkeep_type_visitor(), upkeep);
		lua_push(L, type);
	}
	return lua_index_raw(L);
}

UNIT_SETTER("upkeep", lua_index_raw) {
	if(lua_isnumber(L, value.index)) {
		u.set_upkeep(static_cast<int>(luaL_checkinteger(L, 3)));
		return;
	}
	auto v = lua_check<std::string_view>(L, value.index);
	if(v == "loyal" || v == "free") {
		u.set_upkeep(unit::upkeep_loyal());
	} else if(v == "full") {
		u.set_upkeep(unit::upkeep_full());
	} else {
		std::string err_msg = "unknown upkeep value of unit: ";
		err_msg += v;
		luaL_argerror(L, 2, err_msg.c_str());
	}
	return;
}

UNIT_GETTER("advancements", std::vector<config>) {
	return u.modification_advancements();
}

UNIT_SETTER("advancements", std::vector<config>) {
	u.set_advancements(value);
}

UNIT_GETTER("overlays", std::vector<std::string>) {
	return u.overlays();
}

UNIT_GETTER("traits", std::vector<std::string>) {
	return u.get_traits_list();
}

UNIT_GETTER("abilities", std::vector<std::string>) {
	return u.get_ability_list();
}

UNIT_GETTER("status", lua_index_raw) {
	(void)u;
	lua_createtable(L, 1, 0);
	lua_pushvalue(L, 1);
	lua_rawseti(L, -2, 1);
	luaL_setmetatable(L, ustatusKey);
	return lua_index_raw(L);
}

UNIT_GETTER("variables", lua_index_raw) {
	(void)u;
	lua_createtable(L, 1, 0);
	lua_pushvalue(L, 1);
	lua_rawseti(L, -2, 1);
	luaL_setmetatable(L, unitvarKey);
	return lua_index_raw(L);
}

UNIT_GETTER("attacks", lua_index_raw) {
	(void)u;
	push_unit_attacks_table(L, 1);
	return lua_index_raw(L);
}

UNIT_GETTER("petrified", bool) {
	deprecated_message("(unit).petrified", DEP_LEVEL::INDEFINITE, {1,17,0}, "use (unit).status.petrified instead");
	return u.incapacitated();
}

UNIT_GETTER("animations", std::vector<std::string>) {
	return u.anim_comp().get_flags();
}

UNIT_GETTER("recall_filter", config) {
	return u.recall_filter();
}

UNIT_SETTER("recall_filter", config) {
	u.set_recall_filter(value);
}

UNIT_GETTER("hidden", bool) {
	return u.get_hidden();
}

UNIT_SETTER("hidden", bool) {
	u.set_hidden(value);
}

UNIT_GETTER("resting", bool) {
	return u.resting();
}

UNIT_SETTER("resting", bool) {
	u.set_resting(value);
}

UNIT_GETTER("flying", bool) {
	return u.is_flying();
}

UNIT_GETTER("fearless", bool) {
	return u.is_fearless();
}

UNIT_GETTER("healthy", bool) {
	return u.is_healthy();
}

UNIT_GETTER("zoc", bool) {
	return u.get_emit_zoc();
}

UNIT_SETTER("zoc", bool) {
	u.set_emit_zoc(value);
}

UNIT_GETTER("role", std::string) {
	return u.get_role();
}

UNIT_SETTER("role", std::string) {
	u.set_role(value);
}

UNIT_GETTER("race", std::string) {
	return u.race()->id();
}

UNIT_GETTER("gender", std::string) {
	return gender_string(u.gender());
}

UNIT_GETTER("variation", std::string) {
	return u.variation();
}

UNIT_GETTER("undead_variation", std::string) {
	return u.undead_variation();
}

UNIT_SETTER("undead_variation", std::string) {
	u.set_undead_variation(value);
}

UNIT_GETTER("facing", std::string) {
	return map_location::write_direction(u.facing());
}

UNIT_SETTER("facing", std::string) {
	u.set_facing(map_location::parse_direction(value));
}

UNIT_GETTER("portrait", std::string) {
	return u.big_profile() == u.absolute_image()
		? u.absolute_image() + u.image_mods() + "~SCALE_SHARP(144,144)"
		: u.big_profile();
}

UNIT_SETTER("portrait", std::string) {
	u.set_big_profile(value);
}

UNIT_GETTER("__cfg", config) {
	config cfg;
	u.write(cfg);
	u.get_location().write(cfg);
	return cfg;
}

/**
 * Gets some data on a unit (__index metamethod).
 * - Arg 1: full userdata containing the unit id.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_unit_get(lua_State *L)
{
	return unitReg.get(L);
}

/**
 * Sets some data on a unit (__newindex metamethod).
 * - Arg 1: full userdata containing the unit id.
 * - Arg 2: string containing the name of the property.
 * - Arg 3: something containing the attribute.
 */
static int impl_unit_set(lua_State *L)
{
	return unitReg.set(L);
}

/**
 * Prints valid attributes on a unit (__dir metamethod).
 * - Arg 1: full userdata containing the unit id.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: a list of attributes.
 */
static int impl_unit_dir(lua_State *L)
{
	return unitReg.dir(L);
}

/**
 * Gets the status of a unit (__index metamethod).
 * - Arg 1: table containing the userdata containing the unit id.
 * - Arg 2: string containing the name of the status.
 * - Ret 1: boolean.
 */
static int impl_unit_status_get(lua_State *L)
{
	if(!lua_istable(L, 1)) {
		return luaW_type_error(L, 1, "unit status");
	}
	lua_rawgeti(L, 1, 1);
	const unit* u = luaW_tounit(L, -1);
	if(!u) {
		return luaL_argerror(L, 1, "unknown unit");
	}
	char const *m = luaL_checkstring(L, 2);
	lua_pushboolean(L, u->get_state(m));
	return 1;
}

/**
 * Sets the status of a unit (__newindex metamethod).
 * - Arg 1: table containing the userdata containing the unit id.
 * - Arg 2: string containing the name of the status.
 * - Arg 3: boolean.
 */
static int impl_unit_status_set(lua_State *L)
{
	if(!lua_istable(L, 1)) {
		return luaW_type_error(L, 1, "unit status");
	}
	lua_rawgeti(L, 1, 1);
	unit* u = luaW_tounit(L, -1);
	if(!u) {
		return luaL_argerror(L, 1, "unknown unit");
	}
	char const *m = luaL_checkstring(L, 2);
	u->set_state(m, luaW_toboolean(L, 3));
	return 0;
}

/**
 * List statuses on a unit (__dir metamethod)
 * This returns all known statuses (regardless of state) plus any currently set to true.
 */
static int impl_unit_status_dir(lua_State *L)
{
	if(!lua_istable(L, 1)) {
		return luaW_type_error(L, 1, "unit status");
	}
	lua_rawgeti(L, 1, 1);
	unit* u = luaW_tounit(L, -1);
	if(!u) {
		return luaL_argerror(L, 1, "unknown unit");
	}
	std::vector<std::string> states;
	states.reserve(unit::NUMBER_OF_STATES);
	for(unit::state_t s = unit::STATE_SLOWED; s < unit::NUMBER_OF_STATES; s = unit::state_t(s + 1)) {
		states.push_back(unit::get_known_boolean_state_name(s));
	}
	for(auto s : u->get_states()) {
		states.push_back(s);
	}
	lua_push(L, states);
	return 1;
}

/**
 * Gets the variable of a unit (__index metamethod).
 * - Arg 1: table containing the userdata containing the unit id.
 * - Arg 2: string containing the name of the status.
 * - Ret 1: boolean.
 */
static int impl_unit_variables_get(lua_State *L)
{
	if(!lua_istable(L, 1)) {
		return luaW_type_error(L, 1, "unit variables");
	}
	lua_rawgeti(L, 1, 1);
	const unit* u = luaW_tounit(L, -1);
	if(!u) {
		return luaL_argerror(L, 2, "unknown unit");
	}
	char const *m = luaL_checkstring(L, 2);
	return_cfgref_attrib("__cfg", u->variables());

	variable_access_const v(m, u->variables());
	return luaW_pushvariable(L, v) ? 1 : 0;
}

/**
 * Sets the variable of a unit (__newindex metamethod).
 * - Arg 1: table containing the userdata containing the unit id.
 * - Arg 2: string containing the name of the status.
 * - Arg 3: scalar.
 */
static int impl_unit_variables_set(lua_State *L)
{
	if(!lua_istable(L, 1)) {
		return luaW_type_error(L, 1, "unit variables");
	}
	lua_rawgeti(L, 1, 1);
	unit* u = luaW_tounit(L, -1);
	if(!u) {
		return luaL_argerror(L, 2, "unknown unit");
	}
	char const *m = luaL_checkstring(L, 2);
	modify_cfg_attrib("__cfg", u->variables() = cfg);
	config& vars = u->variables();
	if(lua_isnoneornil(L, 3)) {
		try {
			variable_access_throw(m, vars).clear(false);
		} catch(const invalid_variablename_exception&) {
		}
		return 0;
	}
	variable_access_create v(m, vars);
	luaW_checkvariable(L, v, 3);
	return 0;
}

/**
 * List variables on a unit (__dir metamethod)
 */
static int impl_unit_variables_dir(lua_State *L)
{
	if(!lua_istable(L, 1)) {
		return luaW_type_error(L, 1, "unit variables");
	}
	lua_rawgeti(L, 1, 1);
	unit* u = luaW_tounit(L, -1);
	if(!u) {
		return luaL_argerror(L, 2, "unknown unit");
	}
	config& vars = u->variables();
	std::vector<std::string> variables;
	variables.reserve(vars.attribute_count() + vars.all_children_count());
	for(const auto& attr : vars.attribute_range()) {
		variables.push_back(attr.first);
	}
	for(auto [key, cfg] : vars.all_children_view()) {
		variables.push_back(key);
	}
	lua_push(L, variables);
	return 1;
}

namespace lua_units {
	std::string register_metatables(lua_State* L)
	{
		std::ostringstream cmd_out;

		// Create the getunit metatable.
		cmd_out << "Adding getunit metatable...\n";

		luaL_newmetatable(L, getunitKey);
		lua_pushcfunction(L, impl_unit_collect);
		lua_setfield(L, -2, "__gc");
		lua_pushcfunction(L, impl_unit_equality);
		lua_setfield(L, -2, "__eq");
		lua_pushcfunction(L, impl_unit_tostring);
		lua_setfield(L, -2, "__tostring");
		lua_pushcfunction(L, impl_unit_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_unit_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushcfunction(L, impl_unit_dir);
		lua_setfield(L, -2, "__dir");
		lua_pushstring(L, "unit");
		lua_setfield(L, -2, "__metatable");

		// Create the unit status metatable.
		cmd_out << "Adding unit status metatable...\n";

		luaL_newmetatable(L, ustatusKey);
		lua_pushcfunction(L, impl_unit_status_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_unit_status_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushcfunction(L, impl_unit_status_dir);
		lua_setfield(L, -2, "__dir");
		lua_pushstring(L, "unit status");
		lua_setfield(L, -2, "__metatable");

		// Create the unit variables metatable.
		cmd_out << "Adding unit variables metatable...\n";

		luaL_newmetatable(L, unitvarKey);
		lua_pushcfunction(L, impl_unit_variables_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_unit_variables_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushcfunction(L, impl_unit_variables_dir);
		lua_setfield(L, -2, "__dir");
		lua_pushstring(L, "unit variables");
		lua_setfield(L, -2, "__metatable");

		return cmd_out.str();
	}
}
