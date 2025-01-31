/*
	Copyright (C) 2018 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "scripting/lua_terrainfilter.hpp"
#include "scripting/lua_terrainmap.hpp"

#include "formatter.hpp"
#include "log.hpp"
#include "map/location.hpp"
#include "map/map.hpp"
#include "pathutils_impl.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_formula_bridge.hpp"
#include "scripting/push_check.hpp"
#include "serialization/string_utils.hpp"

#include "formula/callable_objects.hpp"
#include "formula/formula.hpp"

#include <boost/dynamic_bitset.hpp>
#include <unordered_map>

static lg::log_domain log_scripting_lua_mapgen("scripting/lua/mapgen");
#define LOG_LMG LOG_STREAM(info, log_scripting_lua_mapgen)
#define ERR_LMG LOG_STREAM(err, log_scripting_lua_mapgen)
//general helper functions for parsing

struct invalid_lua_argument : public std::exception
{
	explicit invalid_lua_argument(const std::string& msg) : errormessage_(msg) {}
	const char* what() const noexcept { return errormessage_.c_str(); }

private:
	std::string errormessage_;
};

using known_sets_t = std::map<std::string, std::set<map_location>>;
using offset_list_t = std::vector<std::pair<int, int>>;
using std::string_view;
using dynamic_bitset  = boost::dynamic_bitset<>;
using location_set = std::set<map_location>;

static const char terrinfilterKey[] = "terrainfilter";
#define LOG_MATCHES(NAME) \
LOG_LMG << #NAME << ":matches(" << l << ") line:" << __LINE__;

//helper functions for parsing
namespace {
	int atoi(string_view s)
	{
		if(s.empty()) {
			return 0;
		}

		char** end = nullptr;
		int res = strtol(&s[0], end, 10);
		return res;
	}

	std::pair<int, int> parse_single_range(string_view s)
	{
		int dash_pos = s.find('-');
		if(dash_pos == int(string_view::npos)) {
			int res = atoi(s);
			return {res, res};
		}
		else {
			string_view first = s.substr(0, dash_pos);
			string_view second = s.substr(dash_pos + 1);
			return {atoi(first), atoi(second)};
		}
	}

	dynamic_bitset parse_range(string_view s)
	{
		dynamic_bitset res;
		utils::split_foreach(s, ',', utils::STRIP_SPACES, [&](string_view part){
			auto pair = parse_single_range(part);
			int m = std::max(pair.first, pair.second);
			if(m >= int(res.size())) {
				res.resize(m + 1);
				for(int i = pair.first; i <= pair.second; ++i) {
					res[i] = true;
				}
			}
		});
		return res;
	}
	void parse_rel(string_view str, offset_list_t& even, offset_list_t& odd)
	{
		//sw = 1*s -1*se
		//nw = -1*se
		//ne = 1*se - 1*s
		int s = 0;
		int se = 0;
		bool last_was_n = false;
		while(!str.empty()) {
			switch(str.front()) {
			case 'n':
				--s;
				last_was_n = true;
				break;
			case 's':
				++s;
				last_was_n = false;
				break;
			case 'e':
				++se;
				if(!last_was_n) {
					--s;
				}
				break;
			case 'w':
				--se;
				if(last_was_n) {
					++s;
				}
				break;
			default:
				break;
			}
			str.remove_prefix(1);
		}
		if((se & 2) == 0) {
			odd.emplace_back(se, s + se/2);
			even.emplace_back(se, s + se/2);
		}
		else {
			odd.emplace_back(se, s + (se - 1)/2);
			even.emplace_back(se, s + (se + 1)/2);
		}
	}

	void parse_rel_sequence(string_view s, offset_list_t& even, offset_list_t& odd)
	{
		utils::split_foreach(s, ',', utils::STRIP_SPACES, [&](string_view part){
			parse_rel(part, even, odd);
		});
	}
	/**
 * TODO: move to a template header.
 * Function that will add to @a result all elements of @a locs, plus all
 * on-board locations matching @a pred that are connected to elements of
 * locs by a chain of at most @a radius tiles, each of which matches @a pred.
 * @a add_result a function that takes a location_range
*/

} //end namespace

static std::set<map_location> luaW_to_locationset(lua_State* L, int index)
{
	std::set<map_location> res;
	map_location single;
	if(luaW_tolocation(L, index, single)) {
		res.insert(single);
		return res;
	}
	if(!lua_istable(L, index)) return res;
	lua_pushvalue(L, index);
	size_t len = lua_rawlen(L, -1);
	for(size_t i = 0; i != len; ++i) {
		lua_geti(L, -1, i + 1);
		res.insert(luaW_checklocation(L, -1));
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
	return res;
}

class filter_impl
{
public:
	filter_impl() {};
	virtual bool matches(const gamemap_base& m, map_location l) const = 0;
	virtual ~filter_impl() {};
};

//build_filter impl
namespace {

std::unique_ptr<filter_impl> build_filter(lua_State* L, int res_index, known_sets_t& ks);

class con_filter : public filter_impl
{
public:
	con_filter(lua_State* L, int res_index, known_sets_t& ks)
		:list_()
	{
		LOG_LMG << "creating con filter";
		size_t len = lua_rawlen(L, -1);
		for(size_t i = 1; i != len; ++i) {
			lua_geti(L, -1, i + 1);
			list_.emplace_back(build_filter(L, res_index, ks));
			lua_pop(L, 1);
		}
	}
	std::vector<std::unique_ptr<filter_impl>> list_;
};

class and_filter : public con_filter
{
public:
	and_filter(lua_State* L, int res_index, known_sets_t& ks)
		: con_filter(L, res_index, ks)
	{
		LOG_LMG << "created and filter";
	}

	bool matches(const gamemap_base& m, map_location l) const override
	{
		LOG_MATCHES(and);
		for(const auto& pfilter : list_) {
			if(!pfilter->matches(m, l)) {
				return false;
			}
		}
		return true;
	}
};

class or_filter : public con_filter
{
public:
	or_filter(lua_State* L, int res_index, known_sets_t& ks)
		: con_filter(L, res_index, ks)
	{
		LOG_LMG << "created or filter";
	}

	bool matches(const gamemap_base& m, map_location l) const override
	{
		LOG_MATCHES(or);
		for(const auto& pfilter : list_) {
			if(pfilter->matches(m, l)) {
				return true;
			}
		}
		return false;
	}
};

class nand_filter : public con_filter
{
public:
	nand_filter(lua_State* L, int res_index, known_sets_t& ks)
		: con_filter(L, res_index, ks)
	{
		LOG_LMG << "created nand filter";
	}

	bool matches(const gamemap_base& m, map_location l) const override
	{
		LOG_MATCHES(nand);
		for(const auto& pfilter : list_) {
			if(!pfilter->matches(m, l)) {
				return true;
			}
		}
		return false;
	}
};

class nor_filter : public con_filter
{
public:
	nor_filter(lua_State* L, int res_index, known_sets_t& ks)
		: con_filter(L, res_index, ks)
	{
		LOG_LMG << "created nor filter";
	}

	bool matches(const gamemap_base& m, map_location l) const override
	{
		LOG_MATCHES(nor);
		for(const auto& pfilter : list_) {
			if(pfilter->matches(m, l)) {
				return false;
			}
		}
		return true;
	}
};

class cached_filter : public filter_impl
{
public:
	cached_filter(lua_State* L, int res_index, known_sets_t& ks)
		: filter_()
		, cache_()
	{
		LOG_LMG << "creating cached filter";
		lua_geti(L, -1, 2);
		filter_ = build_filter(L, res_index, ks);
		lua_pop(L, 1);
	}

	bool matches(const gamemap_base& m, map_location l) const override
	{
		LOG_MATCHES(cached);
		int cache_size = 2 * m.total_width() * m.total_height();
		int loc_index = 2 * (l.wml_x() + l.wml_y() * m.total_width());

		if(int(cache_.size()) != cache_size) {
			cache_ = dynamic_bitset(cache_size);
		}
		if(cache_[loc_index]) {
			return cache_[loc_index + 1];
		}
		else {
			bool res = filter_->matches(m, l);
			cache_[loc_index] = true;
			cache_[loc_index + 1] = res;
			return res;
		}
	}

	std::unique_ptr<filter_impl> filter_;
	mutable dynamic_bitset cache_;
};

class x_filter : public filter_impl
{
public:
	x_filter(lua_State* L, int /*res_index*/, known_sets_t&)
		: filter_()
	{
		LOG_LMG << "creating x filter";
		lua_geti(L, -1, 2);
		filter_ = parse_range(luaW_tostring(L, -1));
		lua_pop(L, 1);
	}
	bool matches(const gamemap_base&, map_location l) const override
	{
		LOG_MATCHES(x);
		const auto value = l.wml_x();
		return value >= 0 && value < int(filter_.size()) && filter_[value];
	}
	dynamic_bitset filter_;
};

class y_filter : public filter_impl
{
public:
	y_filter(lua_State* L, int /*res_index*/, known_sets_t&)
	: filter_()
	{
		LOG_LMG << "creating y filter";
		lua_geti(L, -1, 2);
		filter_ = parse_range(luaW_tostring(L, -1));
		lua_pop(L, 1);
	}

	bool matches(const gamemap_base&, map_location l) const override
	{
		LOG_MATCHES(y);
		const auto value = l.wml_y();
		return value >= 0 && value < int(filter_.size()) && filter_[value];
	}

	dynamic_bitset filter_;
};

class onborder_filter : public filter_impl
{
public:
	onborder_filter(lua_State*, int /*res_index*/, known_sets_t&)
	{
		LOG_LMG << "creating onborder filter";
	}

	bool matches(const gamemap_base& m, map_location l) const override
	{
		LOG_MATCHES(onborder);
		return !m.on_board(l);
	}
};

class terrain_filter : public filter_impl
{
public:
	terrain_filter(lua_State* L, int /*res_index*/, known_sets_t&)
	: filter_()
	{
		LOG_LMG << "creating terrain filter";
		lua_geti(L, -1, 2);
		//fixme: use string_view
		filter_ = t_translation::ter_match(luaW_tostring(L, -1));
		lua_pop(L, 1);
	}

	bool matches(const gamemap_base& m, map_location l) const override
	{
		LOG_MATCHES(terrain);
		const t_translation::terrain_code letter = m.get_terrain(l);
		return t_translation::terrain_matches(letter, filter_);
	}

	t_translation::ter_match filter_;
};

static const offset_list_t even_offsets_default = {{1 , 0}, {1 , 1}, {0 , 1}, {-1 , 1}, {-1 , 0}, {0, -1}};
static const offset_list_t odd_offsets_default = {{1 , -1}, {1 , 0}, {0 , 1}, {-1 , 0}, {-1 , -1}, {0, -1}};

class adjacent_filter : public filter_impl
{
public:
	adjacent_filter(lua_State* L, int res_index, known_sets_t& ks)
	: filter_()
	{
		LOG_LMG << "creating adjacent filter";
		if(luaW_tableget(L, -1, "adjacent")) {
			parse_rel_sequence(luaW_tostring(L, -1), even_offsets_, odd_offsets_);
			lua_pop(L, 1);
		}
		else {
			even_offsets_ = even_offsets_default;
			odd_offsets_ = odd_offsets_default;
		}
		if(luaW_tableget(L, -1, "count")) {
			accepted_counts_ = parse_range(luaW_tostring(L, -1));
			lua_pop(L, 1);
		}
		lua_geti(L, -1, 2);
		filter_ = build_filter(L, res_index, ks);
		lua_pop(L, 1);
	}

	bool matches(const gamemap_base& m, map_location l) const override
	{
		LOG_MATCHES(adjacent);
		int count = 0;
		// is_odd == is_even in wml coordinates.
		const offset_list_t& offsets = (l.wml_x() & 1) ?  odd_offsets_ : even_offsets_;
		for(const auto& offset : offsets) {
			map_location ad = {l.x + offset.first, l.y + offset.second};
			if(m.on_board_with_border(ad) && filter_->matches(m, ad)) {
				if(accepted_counts_.size() == 0) {
					return true;
				}
				++count;
			}
		}
		return int(accepted_counts_.size()) > count && accepted_counts_[count];
	}
	offset_list_t even_offsets_;
	offset_list_t odd_offsets_;
	dynamic_bitset accepted_counts_;
	std::unique_ptr<filter_impl> filter_;
};

class findin_filter : public filter_impl
{
public:
	findin_filter(lua_State* L, int res_index, known_sets_t& ks)
		: set_(nullptr)
	{
		LOG_LMG << "creating findin filter";
		int idx = lua_absindex(L, -1);
		switch(lua_geti(L, idx, 2)) {
		case LUA_TTABLE:
			// Also accepts a single location of the form {x,y} or {x=x,y=y}
			init_from_inline_set(luaW_to_locationset(L, -1));
			break;
		case LUA_TNUMBER:
			lua_geti(L, idx, 3);
			init_from_single_loc(luaL_checkinteger(L, -2), luaL_checkinteger(L, -1));
			break;
		case LUA_TSTRING:
			if(lua_geti(L, idx, 3) == LUA_TSTRING) {
				init_from_ranges(luaL_checkstring(L, -2), luaL_checkstring(L, -1));
			} else {
				init_from_named_set(L, luaL_checkstring(L, -2), res_index, ks);
			}
			break;
		}
		lua_settop(L, idx);
	}

	void init_from_inline_set(const location_set& locs) {
		inline_ = locs;
		set_ = &inline_;
	}

	void init_from_single_loc(int x, int y) {
		map_location loc(x, y, wml_loc());
		inline_.insert(loc);
		set_ = &inline_;
	}

	void init_from_ranges(const std::string& xs, const std::string& ys) {
		auto xvals = utils::parse_ranges_unsigned(xs), yvals = utils::parse_ranges_unsigned(ys);
		// TODO: Probably error if they're different sizes?
		for(size_t i = 0; i < std::min(xvals.size(), yvals.size()); i++) {
			for(int x = xvals[i].first; x <= xvals[i].second; x++) {
				for(int y = yvals[i].first; y <= yvals[i].second; y++) {
					inline_.insert(map_location(x, y, wml_loc()));
				}
			}
		}
		set_ = &inline_;
	}

	void init_from_named_set(lua_State* L, const std::string& id, int res_index, known_sets_t& ks) {
		//TODO: c++14: use heterogenous lookup.
		auto insert_res = ks.insert(known_sets_t::value_type{id, {}});
		if(insert_res.second && res_index > 0) {
			// istable(L, res_index) was already checked.
			if(luaW_tableget(L, res_index, id.c_str())) {
				insert_res.first->second = luaW_to_locationset(L, -1);
				lua_pop(L, 1);
			}
		}
		set_ = &insert_res.first->second;
	}
	bool matches(const gamemap_base&, map_location l) const override
	{
		LOG_MATCHES(findin);
		if(set_) {
			return set_->find(l) != set_->end();
		}
		return false;
	}
	const location_set* set_;
	location_set inline_;
};

class radius_filter : public filter_impl
{
public:

	radius_filter(lua_State* L, int res_index, known_sets_t& ks)
		: radius_()
		, filter_radius_()
		, filter_()
	{
		LOG_LMG << "creating radius filter";
		if(luaW_tableget(L, -1, "filter_radius")) {
			filter_radius_ = build_filter(L, res_index, ks);
			lua_pop(L, 1);
		}
		lua_geti(L, -1, 2);
		radius_ = lua_tointeger(L, -1);
		lua_pop(L, 1);
		lua_geti(L, -1, 3);
		filter_ = build_filter(L, res_index, ks);
		lua_pop(L, 1);
	}

	bool matches(const gamemap_base& m, map_location l) const override
	{
		LOG_MATCHES(radius);
		std::set<map_location> result;

		get_tiles_radius({{ l }}, radius_, result,
			[&](const map_location& l) {
				return m.on_board_with_border(l);
			},
			[&](const map_location& l) {
				return !filter_radius_ || filter_radius_->matches(m, l);
			}
		);

		for (map_location lr : result) {
			if(!filter_ || filter_->matches(m, lr)) {
				return true;
			}
		}
		return false;
	}

	int radius_;
	std::unique_ptr<filter_impl> filter_radius_;
	std::unique_ptr<filter_impl> filter_;
};

class formula_filter : public filter_impl
{
public:
	formula_filter(lua_State* L, int, known_sets_t&)
		: formula_()
	{
		LOG_LMG << "creating formula filter";
		lua_geti(L, -1, 2);
		formula_ = luaW_check_formula(L, 1, true);
		lua_pop(L, 1);
	}
	bool matches(const gamemap_base&, map_location l) const override
	{
		LOG_MATCHES(formula);
		try {
			const wfl::location_callable callable1(l);
			wfl::map_formula_callable callable(callable1.fake_ptr());
			return (formula_.get() != nullptr) && formula_->evaluate(callable).as_bool();
		} catch(const wfl::formula_error& e) {
			ERR_LMG << "Formula error: " << e.type << " at " << e.filename << ':' << e.line << ")";
			return false;
		}
	}
	lua_formula_bridge::fpointer formula_;
};

// todo: maybe invent a general macro for this string_switch implementation.
enum filter_keys { F_AND, F_OR, F_NAND, F_NOR, F_X, F_Y, F_FIND_IN, F_ADJACENT, F_TERRAIN, F_RADIUS, F_FORMULA, F_ONBORDER, F_CACHED };
// todo: c++20: perhaps enable heterogenous lookup.
static const std::unordered_map<std::string, filter_keys> keys {
	{ "all", F_AND },
	{ "any", F_OR },
	{ "not_all", F_NAND },
	{ "none", F_NOR },
	{ "x", F_X },
	{ "y", F_Y },
	{ "find_in", F_FIND_IN },
	{ "adjacent", F_ADJACENT },
	{ "terrain", F_TERRAIN },
	{ "cached", F_CACHED },
	{ "formula", F_FORMULA },
	{ "onborder", F_ONBORDER },
	{ "radius", F_RADIUS }
};

std::unique_ptr<filter_impl> build_filter(lua_State* L, int res_index, known_sets_t& ks)
{
	LOG_LMG << "buildfilter: start";
	if(!lua_istable(L, -1)) {
		throw invalid_lua_argument("buildfilter: expected table");
	}
	lua_rawgeti(L, -1, 1);
	std::string s = std::string(luaW_tostring(L, -1));
	LOG_LMG << "buildfilter: got: " << s;
	auto it = keys.find(s);
	if(it == keys.end()) {
		//fixme use proper exception type.
		throw invalid_lua_argument(std::string("buildfilter: invalid filter type ") + s);
	}
	auto key = it->second;
	lua_pop(L, 1);
	switch(key)
	{
	case F_AND:
		return std::make_unique<and_filter>(L, res_index, ks);
	case F_OR:
		return std::make_unique<or_filter>(L, res_index, ks);
	case F_NAND:
		return std::make_unique<nand_filter>(L, res_index, ks);
	case F_NOR:
		return std::make_unique<nor_filter>(L, res_index, ks);
	case F_X:
		return std::make_unique<x_filter>(L, res_index, ks);
	case F_Y:
		return std::make_unique<y_filter>(L, res_index, ks);
	case F_FIND_IN:
		return std::make_unique<findin_filter>(L, res_index, ks);
	case F_ADJACENT:
		return std::make_unique<adjacent_filter>(L, res_index, ks);
	case F_TERRAIN:
		return std::make_unique<terrain_filter>(L, res_index, ks);
	case F_RADIUS:
		return std::make_unique<radius_filter>(L, res_index, ks);
	case F_CACHED:
		return std::make_unique<cached_filter>(L, res_index, ks);
	case F_FORMULA:
		return std::make_unique<formula_filter>(L, res_index, ks);
	case F_ONBORDER:
		return std::make_unique<onborder_filter>(L, res_index, ks);
	default:
		throw "invalid filter key enum";
	}
}
}

//////////////// PUBLIC API ////////////////

namespace lua_mapgen {
/**
 * @param L the pointer to the lua interpreter.
 * @param data_index a index to the lua stack pointing to the lua table that describes the filter.
 * @param res_index a _positive_ index to the lua stack pointing to the lua table that describes the filter resources.
 */
filter::filter(lua_State* L, int data_index, int res_index)
{
	LOG_LMG <<  "creating filter object";
	lua_pushvalue (L, data_index);
	impl_ = build_filter(L, res_index, known_sets_);
	lua_pop(L, 1);
	LOG_LMG <<  "finished creating filter object";
}

bool filter::matches(const gamemap_base& m, map_location l) const
{
	log_scope("filter::matches");
	return impl_->matches(m, l);
}

filter::~filter()
{

}

}

int intf_mg_get_locations(lua_State* L)
{
	LOG_LMG <<  "map:get_locations";
	gamemap_base& m = luaW_checkterrainmap(L, 1);
	const auto f = luaW_check_mgfilter(L, 2, true);
	location_set res;
	LOG_LMG <<  "map:get_locations vaidargs";
	if(!lua_isnone(L, 3)) {
		LOG_LMG <<  "map:get_locations some locations";
		location_set s = luaW_to_locationset(L, 3);
		LOG_LMG <<  "map:get_locations #args = " << s.size();
		for (const map_location& l : s) {
			if(f->matches(m, l)) {
				res.insert(l);
			}
		}
	}
	else {
		LOG_LMG <<  "map:get_locations all locations";
		m.for_each_loc([&](map_location l) {
			if(f->matches(m, l)) {
				res.insert(l);
			}
		});
	}
	LOG_LMG <<  "map:get_locations #res = " << res.size();
	luaW_push_locationset(L, res);
	LOG_LMG <<  "map:get_locations end";
	return 1;

}

int intf_mg_get_tiles_radius(lua_State* L)
{
	gamemap_base& m = luaW_checkterrainmap(L, 1);
	location_set s = luaW_to_locationset(L, 2);
	int r = luaL_checkinteger(L, 3);
	const auto f = luaW_check_mgfilter(L, 4, true);
	location_set res;
	get_tiles_radius(std::move(s), r, res,
		[&](const map_location& l) {
			return m.on_board_with_border(l);
		},
		[&](const map_location& l) {
			return f->matches(m, l);
		}
	);
	luaW_push_locationset(L, res);
	return 1;
}

bool luaW_is_mgfilter(lua_State* L, int index)
{
	return luaL_testudata(L, index, terrinfilterKey) != nullptr;
}


lua_mapgen::filter* luaW_to_mgfilter(lua_State *L, int index)
{
	if(luaW_is_mgfilter(L, index)) {
		return static_cast<lua_mapgen::filter*>(lua_touserdata(L, index));
	}
	return nullptr;
}

lua_mapgen::filter_ptr luaW_check_mgfilter(lua_State *L, int index, bool allow_compile)
{
	if(luaW_is_mgfilter(L, index)) {
		lua_mapgen::filter_ptr ptr;
		ptr.get_deleter() = [](lua_mapgen::filter*) {}; // don't delete the Lua-held filter pointer
		ptr.reset(static_cast<lua_mapgen::filter*>(lua_touserdata(L, index)));
		return ptr;
	}
	if(allow_compile && lua_istable(L, index)) {
		auto f = std::make_unique<lua_mapgen::filter>(L, index, 0);
		return f;
	}
	luaW_type_error(L, index, "terrainfilter");
	throw "luaW_type_error didn't throw";
}

void lua_mgfilter_setmetatable(lua_State *L)
{
	luaL_setmetatable(L, terrinfilterKey);
}

template<typename... T>
static lua_mapgen::filter* luaW_push_mgfilter(lua_State *L, T&&... params)
{
	LOG_LMG <<  "luaW_push_mgfilter";
	lua_mapgen::filter* res = new(L) lua_mapgen::filter(std::forward<T>(params)...);
	lua_mgfilter_setmetatable(L);
	return res;
}

/**
 * Create a filter.
*/
int intf_terrainfilter_create(lua_State *L)
{
	try {
		int res_index = 0;
		if(!lua_istable(L, 1)) {
			return luaL_argerror(L, 1, "table expected");
		}
		if(lua_istable(L, 2)) {
			res_index = 2;
		}
		lua_mapgen::filter res(L, 1, res_index);
		luaW_push_mgfilter(L, std::move(res));
		return 1;
	}
	catch(const invalid_lua_argument& e) {
		return luaL_argerror(L, 1, e.what());
	}
}


/**
 * Gets some data on a filter (__index metamethod).
 * - Arg 1: full userdata containing the filter.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_terrainfilter_get(lua_State *L)
{
	luaW_check_mgfilter(L, 1);
	return 0;
}

/**
 * Sets some data on a filter (__newindex metamethod).
 * - Arg 1: full userdata containing the filter.
 * - Arg 2: string containing the name of the property.
 * - Arg 3: something containing the attribute.
 */
static int impl_terrainfilter_set(lua_State *L)
{
	luaW_check_mgfilter(L, 1);
	char const *m = luaL_checkstring(L, 2);
	std::string err_msg = "unknown modifiable property of map: ";
	err_msg += m;
	return luaL_argerror(L, 2, err_msg.c_str());
}


/**
 * Clears the cache of a filter.
 */
static int intf_clearcache(lua_State *L)
{
	luaW_check_mgfilter(L, 1);
	return 0;
}
/**
 * Destroys a map object before it is collected (__gc metamethod).
 */
static int impl_terrainfilter_collect(lua_State *L)
{
	auto f = luaW_check_mgfilter(L, 1);
	f->~filter();
	return 0;
}


namespace lua_terrainfilter {
	std::string register_metatables(lua_State* L)
	{
		std::ostringstream cmd_out;

		cmd_out << "Adding terrainmamap metatable...\n";

		luaL_newmetatable(L, terrinfilterKey);
		lua_pushcfunction(L, impl_terrainfilter_collect);
		lua_setfield(L, -2, "__gc");
		lua_pushcfunction(L, impl_terrainfilter_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_terrainfilter_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushstring(L, "terrain_filter");
		lua_setfield(L, -2, "__metatable");
		// terrainmap methods
		lua_pushcfunction(L, intf_clearcache);
		lua_setfield(L, -2, "clear_cache");

		return cmd_out.str();
	}
}
