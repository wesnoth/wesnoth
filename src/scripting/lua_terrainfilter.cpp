/*
	Copyright (C) 2018 the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "scripting/push_check.hpp"
#include "scripting/game_lua_kernel.hpp"

#include "formula/callable_objects.hpp"
#include "formula/formula.hpp"
#include "formula/string_utils.hpp"

#include <boost/dynamic_bitset.hpp>
#include <boost/range/iterator_range.hpp>
#include <unordered_map>
#include "lua/lauxlib.h"
#include "lua/lua.h"

static lg::log_domain log_scripting_lua_mapgen("scripting/lua/mapgen");
#define LOG_LMG LOG_STREAM(info, log_scripting_lua_mapgen)
#define ERR_LMG LOG_STREAM(err, log_scripting_lua_mapgen)
//general helper functions for parsing

struct inalid_lua_argument : public std::exception
{
	explicit inalid_lua_argument(const std::string& msg) : errormessage_(msg) {}
	const char* what() const noexcept { return errormessage_.c_str(); }

private:
	std::string errormessage_;
};

using knows_sets_t = std::map<std::string, std::set<map_location>>;
using offset_list_t = std::vector<std::pair<int, int>>;
using utils::string_view;
using dynamic_bitset  = boost::dynamic_bitset<>;
using location_set = std::set<map_location>;

static const char terrinfilterKey[] = "terrainfilter";
#define LOG_MATCHES(NAME) \
LOG_LMG << #NAME << ":matches(" << l << ") line:" << __LINE__   << "\n";
namespace utils {
	// todoc++14: use std::make_unique
	template<typename T, typename... Args>
	std::unique_ptr<T> make_unique(Args&&... args)
	{
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}

}
//helper functions for parsing
namespace {
	bool iswhitespace(char c)
	{
		return c == ' ' || c == '\t' || c == '\n' || c == '\r';
	}

	void trim(string_view& s)
	{
		while(!s.empty() && iswhitespace(s.front())) {
			s.remove_prefix(1);
		}
		while(!s.empty() && iswhitespace(s.back())) {
			s.remove_suffix(1);
		}
	}

	template<typename F>
	void split_foreach(string_view s, char sep, const F& f)
	{
		if(s.empty()) {
			return;
		}
		while(true)
		{
			int partend = s.find(sep);
			if(partend == int(string_view::npos)) {
				break;
			}
			f(s.substr(0, partend));
			s.remove_prefix(partend + 1);
		}
		f(s);
	}

	int atoi(string_view s)
	{
		if(s.empty()) {
			return 0;
		}

		char** end = 0;
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
		split_foreach(s, ',', [&](string_view part){
			trim(part);
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
		split_foreach(s, ',', [&](string_view part){
			trim(part);
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

static int luaW_push_locationset(lua_State* L, const std::set<map_location>& locs)
{
	LOG_LMG <<  "push_locationset\n";
	lua_createtable(L, locs.size(), 0);
	int i = 1;
	for (const map_location& loc : locs)
	{
		lua_createtable(L, 2, 0);
		lua_pushinteger(L, loc.wml_x());
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, loc.wml_y());
		lua_rawseti(L, -2, 2);
		lua_rawseti(L, -2, i);
		++i;
	}
	return 1;

}

static std::set<map_location> luaW_to_locationset(lua_State* L, int index)
{
	std::set<map_location> res;
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
	virtual bool matches(const mapgen_gamemap& m, map_location l) = 0;
	virtual ~filter_impl() {};
};

//build_filter impl
namespace {

std::unique_ptr<filter_impl> build_filter(lua_State* L, int res_index, knows_sets_t& ks);

class con_filter : public filter_impl
{
public:
	con_filter(lua_State* L, int res_index, knows_sets_t& ks)
		:list_()
	{
		LOG_LMG << "creating con filter\n";
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
	and_filter(lua_State* L, int res_index, knows_sets_t& ks)
		: con_filter(L, res_index, ks)
	{
		LOG_LMG << "created and filter\n";
	}

	bool matches(const mapgen_gamemap& m, map_location l) override
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
	or_filter(lua_State* L, int res_index, knows_sets_t& ks)
		: con_filter(L, res_index, ks)
	{
		LOG_LMG << "created or filter\n";
	}

	bool matches(const mapgen_gamemap& m, map_location l) override
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
	nand_filter(lua_State* L, int res_index, knows_sets_t& ks)
		: con_filter(L, res_index, ks)
	{
		LOG_LMG << "created nand filter\n";
	}

	bool matches(const mapgen_gamemap& m, map_location l) override
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
	nor_filter(lua_State* L, int res_index, knows_sets_t& ks)
		: con_filter(L, res_index, ks)
	{
		LOG_LMG << "created nor filter\n";
	}

	bool matches(const mapgen_gamemap& m, map_location l) override
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
	cached_filter(lua_State* L, int res_index, knows_sets_t& ks)
		: filter_()
		, cache_()
	{
		LOG_LMG << "creating cached filter\n";
		lua_geti(L, -1, 2);
		filter_ = build_filter(L, res_index, ks);
		lua_pop(L, 1);
	}

	bool matches(const mapgen_gamemap& m, map_location l) override
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
	x_filter(lua_State* L, int /*res_index*/, knows_sets_t&)
		: filter_()
	{
		LOG_LMG << "creating x filter\n";
		lua_geti(L, -1, 2);
		filter_ = parse_range(luaW_tostring(L, -1));
		lua_pop(L, 1);
	}
	bool matches(const mapgen_gamemap&, map_location l) override
	{
		LOG_MATCHES(x);
		return l.x >= 0 && l.x < int(filter_.size()) && filter_[l.x];
	}
	dynamic_bitset filter_;
};

class y_filter : public filter_impl
{
public:
	y_filter(lua_State* L, int /*res_index*/, knows_sets_t&)
	: filter_()
	{
		LOG_LMG << "creating y filter\n";
		lua_geti(L, -1, 2);
		filter_ = parse_range(luaW_tostring(L, -1));
		lua_pop(L, 1);
	}

	bool matches(const mapgen_gamemap&, map_location l) override
	{
		LOG_MATCHES(y);
		return l.y >= 0 && l.y < int(filter_.size()) && filter_[l.y];
	}

	dynamic_bitset filter_;
};

class onborder_filter : public filter_impl
{
public:
	onborder_filter(lua_State*, int /*res_index*/, knows_sets_t&)
	{
		LOG_LMG << "creating onborder filter\n";
	}

	bool matches(const mapgen_gamemap& m, map_location l) override
	{
		LOG_MATCHES(onborder);
		return !m.on_map_noborder(l);
	}
};

class terrain_filter : public filter_impl
{
public:
	terrain_filter(lua_State* L, int /*res_index*/, knows_sets_t&)
	: filter_()
	{
		LOG_LMG << "creating terrain filter\n";
		lua_geti(L, -1, 2);
		//fixme: use string_view
		filter_ = t_translation::ter_match(luaW_tostring(L, -1));
		lua_pop(L, 1);
	}

	bool matches(const mapgen_gamemap& m, map_location l) override
	{
		LOG_MATCHES(terrain);
		const t_translation::terrain_code letter = m[l];
		return t_translation::terrain_matches(letter, filter_);
	}

	t_translation::ter_match filter_;
};

static const offset_list_t even_offsets_default = {{1 , 0}, {1 , 1}, {0 , 1}, {-1 , 1}, {-1 , 0}, {0, -1}};
static const offset_list_t odd_offsets_default = {{1 , -1}, {1 , 0}, {0 , 1}, {-1 , 0}, {-1 , -1}, {0, -1}};

class adjacent_filter : public filter_impl
{
public:
	adjacent_filter(lua_State* L, int res_index, knows_sets_t& ks)
	: filter_()
	{
		LOG_LMG << "creating adjacent filter\n";
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

	bool matches(const mapgen_gamemap& m, map_location l) override
	{
		LOG_MATCHES(adjacent);
		int count = 0;
		// is_odd == is_even in wml coordinates.
		offset_list_t& offsets = (l.wml_x() & 1) ?  odd_offsets_ : even_offsets_;
		for(const auto& offset : offsets) {
			map_location ad = {l.x + offset.first, l.y + offset.second};
			if(m.on_map(ad) && filter_->matches(m, ad)) {
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
	findin_filter(lua_State* L, int res_index, knows_sets_t& ks)
		: set_(nullptr)
	{
		LOG_LMG << "creating findin filter\n";
		lua_geti(L, -1, 2);
		std::string id = std::string(luaW_tostring(L, -1));
		lua_pop(L, 1);

		//TODO: c++14: use heterogenous lookup.
		auto insert_res = ks.insert(knows_sets_t::value_type{id, {}});
		if(insert_res.second && res_index > 0) {
			// istable(L, res_index) was already checked.
			if(luaW_tableget(L, res_index, id.c_str())) {
				insert_res.first->second = luaW_to_locationset(L, -1);
				lua_pop(L, 1);
			}
		}
		set_ = &insert_res.first->second;
	}
	bool matches(const mapgen_gamemap&, map_location l) override
	{
		LOG_MATCHES(findin);
		if(set_) {
			return set_->find(l) != set_->end();
		}
		return false;
	}
	const location_set* set_;
};

class radius_filter : public filter_impl
{
public:

	radius_filter(lua_State* L, int res_index, knows_sets_t& ks)
		: radius_()
		, filter_radius_()
		, filter_()
	{
		LOG_LMG << "creating radius filter\n";
		if(luaW_tableget(L, -1, "filter_radius")) {
			filter_radius_ = build_filter(L, res_index, ks);
			lua_pop(L, 1);
		}
		lua_geti(L, -1, 2);
		radius_ = lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_geti(L, -1, 3);
		filter_ = build_filter(L, res_index, ks);
		lua_pop(L, 1);
	}

	bool matches(const mapgen_gamemap& m, map_location l) override
	{
		LOG_MATCHES(radius);
		std::set<map_location> result;

		get_tiles_radius({{ l }}, radius_, result,
			[&](const map_location& l) {
				return m.on_map(l);
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
	formula_filter(lua_State* L, int, knows_sets_t&)
		: formula_()
	{
		LOG_LMG << "creating formula filter\n";
		lua_geti(L, -1, 2);
		std::string code = std::string(luaW_tostring(L, -1));
		lua_pop(L, 1);

		try {
			formula_ = utils::make_unique<wfl::formula>(code);
		} catch(const wfl::formula_error& e) {
			ERR_LMG << "formula error" << e.what() << "\n";
		}
	}
	bool matches(const mapgen_gamemap&, map_location l) override
	{
		LOG_MATCHES(formula);
		try {
			const wfl::location_callable callable1(l);
			wfl::map_formula_callable callable(callable1.fake_ptr());
			return (formula_.get() != nullptr) && formula_->evaluate(callable).as_bool();
		} catch(const wfl::formula_error& e) {
			ERR_LMG << "Formula error: " << e.type << " at " << e.filename << ':' << e.line << ")\n";
			return false;
		}
	}
	std::unique_ptr<wfl::formula> formula_;
};

// todo: maybe invent a gerneral macro for this string_switch implementation.
enum filter_keys { F_AND, F_OR, F_NAND, F_NOR, F_X, F_Y, F_FIND_IN, F_ADJACENT, F_TERRAIN, F_RADUIS, F_FORMULA, F_CACHED };
//todoc++14: std::unordered_map doesn'tsupport herterogrnous lookup.
//todo consider renaming and -> all ,or ->any, nor -> none, nand -> notall
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
	{ "radius", F_RADUIS }
};

std::unique_ptr<filter_impl> build_filter(lua_State* L, int res_index, knows_sets_t& ks)
{
	LOG_LMG << "buildfilter: start\n";
	if(!lua_istable(L, -1)) {
		throw inalid_lua_argument("buildfilter: expected table");
	}
	lua_rawgeti(L, -1, 1);
	std::string s = std::string(luaW_tostring(L, -1));
	LOG_LMG << "buildfilter: got: " << s << "\n";
	auto it = keys.find(s);
	if(it == keys.end()) {
		//fixme use proper exception type.
		throw inalid_lua_argument(std::string("buildfilter: invalid filter type ") + s);
	}
	auto key = it->second;
	lua_pop(L, 1);
	switch(key)
	{
	case F_AND:
		return utils::make_unique<and_filter>(L, res_index, ks);
	case F_OR:
		return utils::make_unique<or_filter>(L, res_index, ks);
	case F_NAND:
		return utils::make_unique<nand_filter>(L, res_index, ks);
	case F_NOR:
		return utils::make_unique<nor_filter>(L, res_index, ks);
	case F_X:
		return utils::make_unique<x_filter>(L, res_index, ks);
	case F_Y:
		return utils::make_unique<y_filter>(L, res_index, ks);
	case F_FIND_IN:
		return utils::make_unique<findin_filter>(L, res_index, ks);
	case F_ADJACENT:
		return utils::make_unique<adjacent_filter>(L, res_index, ks);
	case F_TERRAIN:
		return utils::make_unique<terrain_filter>(L, res_index, ks);
	case F_RADUIS:
		return utils::make_unique<radius_filter>(L, res_index, ks);
	case F_CACHED:
		return utils::make_unique<cached_filter>(L, res_index, ks);
	case F_FORMULA:
		return utils::make_unique<formula_filter>(L, res_index, ks);
	default:
		throw "invalid filter key enum";
	}
}
}

//////////////// PUBLIC API ////////////////

namespace lua_mapgen {
/// @a data_index a index to the lua stack pointing to the lua table that describes the filter.
/// @a res_index a _positive_ index to the lua stack pointing to the lua table that describes the filter resources.
filter::filter(lua_State* L, int data_index, int res_index)
{
	LOG_LMG <<  "creating filter object\n";
	lua_pushvalue (L, data_index);
	impl_ = build_filter(L, res_index, known_sets_);
	lua_pop(L, 1);
	LOG_LMG <<  "finished creating filter object\n";
}

bool filter::matches(const mapgen_gamemap& m, map_location l)
{
	log_scope("filter::matches");
	return impl_->matches(m, l);
}

filter::~filter()
{

}

}

static int intf_mg_get_locations_part2(lua_State* L, mapgen_gamemap& m, lua_mapgen::filter& f)
{
	location_set res;
	LOG_LMG <<  "map:get_locations vaidargs\n";
	if(lua_istable(L, 3)) {
		LOG_LMG <<  "map:get_locations some locations\n";
		location_set s = luaW_to_locationset(L, 3);
		LOG_LMG <<  "map:get_locations #args = " << s.size() << "\n";
		for (const map_location& l : s) {
			if(f.matches(m, l)) {
				res.insert(l);
			}
		}
	}
	else {
		LOG_LMG <<  "map:get_locations all locations\n";
		m.for_each_loc([&](map_location l) {
			if(f.matches(m, l)) {
				res.insert(l);
			}
		});
	}
	LOG_LMG <<  "map:get_locations #res = " << res.size() << "\n";
	luaW_push_locationset(L, res);
	LOG_LMG <<  "map:get_locations end\n";
	return 1;

}
int intf_mg_get_locations(lua_State* L)
{
	//todo: create filter form table if needed
	LOG_LMG <<  "map:get_locations\n";
	mapgen_gamemap& m = luaW_checkterrainmap(L, 1);
	if(luaW_is_mgfilter(L, 2)) {
		lua_mapgen::filter& f = luaW_check_mgfilter(L, 2);
		return intf_mg_get_locations_part2(L, m, f);
	}
	else if (lua_istable(L, 2)) {
		lua_mapgen::filter f(L, 2, 0);
		return intf_mg_get_locations_part2(L, m, f);
	}
	else {
		return luaW_type_error(L, 2, "terrainfilter");
	}
}

int intf_mg_get_tiles_radius(lua_State* L)
{
	mapgen_gamemap& m = luaW_checkterrainmap(L, 1);
	lua_mapgen::filter& f = luaW_check_mgfilter(L, 3);
	location_set s = luaW_to_locationset(L, 2);
	location_set res;
	int r = luaL_checkinteger(L, 4);
	get_tiles_radius(std::move(s), r, res,
		[&](const map_location& l) {
			return m.on_map(l);
		},
		[&](const map_location& l) {
			return f.matches(m, l);
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

lua_mapgen::filter& luaW_check_mgfilter(lua_State *L, int index)
{
	if(luaW_is_mgfilter(L, index)) {
		return *static_cast<lua_mapgen::filter*>(lua_touserdata(L, index));
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
	LOG_LMG <<  "luaW_push_mgfilter\n";
	lua_mapgen::filter* res = new(L) lua_mapgen::filter(std::forward<T>(params)...);
	lua_mgfilter_setmetatable(L);
	return res;
}

/**
 * Create a filter.
*/
int intf_terainfilter_create(lua_State *L)
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
	catch(const inalid_lua_argument& e) {
		return luaL_argerror(L, 1, e.what());
	}
}


/**
 * Gets some data on a filter (__index metamethod).
 * - Arg 1: full userdata containing the filter.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_terainfilter_get(lua_State *L)
{
	lua_mapgen::filter& f = luaW_check_mgfilter(L, 1);
	UNUSED(f);
	return 0;
}

/**
 * Sets some data on a filter (__newindex metamethod).
 * - Arg 1: full userdata containing the filter.
 * - Arg 2: string containing the name of the property.
 * - Arg 3: something containing the attribute.
 */
static int impl_terainfilter_set(lua_State *L)
{
	lua_mapgen::filter& f = luaW_check_mgfilter(L, 1);
	UNUSED(f);
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
	lua_mapgen::filter& f = luaW_check_mgfilter(L, 1);
	UNUSED(f);
	return 0;
}
/**
 * Destroys a map object before it is collected (__gc metamethod).
 */
static int impl_terainfilter_collect(lua_State *L)
{
	lua_mapgen::filter& f = luaW_check_mgfilter(L, 1);
	f.~filter();
	return 0;
}


namespace lua_terrainfilter {
	std::string register_metatables(lua_State* L)
	{
		std::ostringstream cmd_out;

		cmd_out << "Adding terrainmamap metatable...\n";

		luaL_newmetatable(L, terrinfilterKey);
		lua_pushcfunction(L, impl_terainfilter_collect);
		lua_setfield(L, -2, "__gc");
		lua_pushcfunction(L, impl_terainfilter_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_terainfilter_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushstring(L, "terrain_filter");
		lua_setfield(L, -2, "__metatable");
		// terainmap methods
		lua_pushcfunction(L, intf_clearcache);
		lua_setfield(L, -2, "clear_cache");

		return cmd_out.str();
	}
}
