/*
	Copyright (C) 2009 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/// Attribute registration system, mainly for objects with a lot of attributes, like units
/// Not used for GUI2 widgets, as they're even more complicated with a deep hierarchy.

#pragma once

struct lua_State;
class t_string;
class vconfig;

#include "config.hpp"
#include "map/location.hpp"
#include "variable_info.hpp"

#include <string>
#include <string_view>
#include <vector>

template<typename T>
std::decay_t<T> lua_check(lua_State *L, int n);

/// Holds a lookup table for members of one type of object.
struct luaW_Registry {
	inline static std::map<std::string_view /* metatable */, std::reference_wrapper<luaW_Registry>> lookup;
	using getters_list = std::map<std::string /* attribute */, std::function<bool(lua_State* L,bool nop)>>;
	/// A map of callbacks that read data from the object.
	getters_list getters;
	using setters_list = std::map<std::string, std::function<bool(lua_State* L,int idx,bool nop)>>;
	/// A map of callbacks that write data to the object.
	setters_list setters;
	using validators_list = std::map<std::string, std::function<bool(lua_State* L)>>;
	/// A map of callbacks that check if a member is available.
	validators_list validators;
	/// The internal metatable string for the object (from __metatable)
	std::string private_metatable;
	/// Optional external metatable for the object (eg "wesnoth", "units")
	/// All entries of this table will be treated as members of the object.
	std::vector<std::string> public_metatable;
	luaW_Registry() = delete;
	luaW_Registry(const std::initializer_list<std::string>& mt);
	~luaW_Registry();
	/// Implement __index metamethod
	int get(lua_State* L);
	/// Implement __newindex metamethod
	int set(lua_State* L);
	/// Implement __dir metamethod
	int dir(lua_State* L);
};

enum class lua_attrfunc_type { getter, setter, validator };

template<typename object_type, typename value_type>
struct lua_getter
{
	virtual value_type get(lua_State* L, const object_type& obj) const = 0;
	virtual ~lua_getter() = default;
};

template<typename object_type, typename value_type>
struct lua_setter
{
	virtual void set(lua_State* L, object_type& obj, const value_type& value) const = 0;
	virtual ~lua_setter() = default;
};

template<typename object_type>
struct lua_validator
{
	virtual bool is_active(lua_State* L, const object_type& obj) const = 0;
	virtual ~lua_validator() = default;
};

template<typename T> struct lua_object_traits;

template<typename object_type, typename value_type, typename action_type, lua_attrfunc_type type>
void register_lua_attribute(const char* name)
{
	using obj_traits = lua_object_traits<object_type>;
	using map_type = std::conditional_t<type == lua_attrfunc_type::validator, luaW_Registry::validators_list, std::conditional_t<type == lua_attrfunc_type::setter, luaW_Registry::setters_list, luaW_Registry::getters_list>>;
	using callback_type = typename map_type::mapped_type;
	map_type* map;
	callback_type fcn;
	if constexpr(type == lua_attrfunc_type::setter) {
		map = &luaW_Registry::lookup.at(obj_traits::metatable).get().setters;
		fcn = [action = action_type()](lua_State* L, int idx, bool nop) {
			if(nop) return true;
			decltype(auto) obj = obj_traits::get(L, 1);
			action.set(L, obj, lua_check<value_type>(L, idx));
			return true;
		};
	} else if constexpr(type == lua_attrfunc_type::getter) {
		map = &luaW_Registry::lookup.at(obj_traits::metatable).get().getters;
		fcn = [action = action_type()](lua_State* L, bool nop) {
			if(nop) return true;
			lua_push(L, action.get(L, obj_traits::get(L, 1)));
			return true;
		};
	} else if constexpr(type == lua_attrfunc_type::validator) {
		map = &luaW_Registry::lookup.at(obj_traits::metatable).get().validators;
		fcn = [action = action_type()](lua_State* L) {
			return action.is_active(L, obj_traits::get(L, 1));
		};
	}
	(*map)[std::string(name)] = fcn;
}

#define LATTR_MAKE_UNIQUE_ID(base, id, obj_name) BOOST_PP_CAT(BOOST_PP_CAT(base, id), BOOST_PP_CAT(_for_, obj_name))

#define LATTR_GETTER5(name, value_type, obj_type, obj_name, id) \
struct LATTR_MAKE_UNIQUE_ID(getter_, id, obj_name) : public lua_getter<obj_type, value_type> { \
	using object_type = obj_type; \
	virtual value_type get(lua_State* L, const object_type& obj_name) const override; \
}; \
struct LATTR_MAKE_UNIQUE_ID(getter_adder_, id, obj_name) { \
	LATTR_MAKE_UNIQUE_ID(getter_adder_, id, obj_name) () \
	{ \
		register_lua_attribute<obj_type, value_type, LATTR_MAKE_UNIQUE_ID(getter_, id, obj_name), lua_attrfunc_type::getter>(name); \
	} \
}; \
static LATTR_MAKE_UNIQUE_ID(getter_adder_, id, obj_name) LATTR_MAKE_UNIQUE_ID(getter_adder_instance_, id, obj_name) ; \
value_type LATTR_MAKE_UNIQUE_ID(getter_, id, obj_name)::get([[maybe_unused]] lua_State* L, const LATTR_MAKE_UNIQUE_ID(getter_, id, obj_name)::object_type& obj_name) const


#define LATTR_SETTER5(name, value_type, obj_type, obj_name, id) \
struct LATTR_MAKE_UNIQUE_ID(setter_, id, obj_name) : public lua_setter<obj_type, value_type> { \
	using object_type = obj_type; \
	void set(lua_State* L, object_type& obj_name, const value_type& value) const override; \
}; \
struct LATTR_MAKE_UNIQUE_ID(setter_adder_, id, obj_name) { \
	LATTR_MAKE_UNIQUE_ID(setter_adder_, id, obj_name) ()\
	{ \
		register_lua_attribute<obj_type, value_type, LATTR_MAKE_UNIQUE_ID(setter_, id, obj_name), lua_attrfunc_type::setter>(name); \
	} \
}; \
static LATTR_MAKE_UNIQUE_ID(setter_adder_, id, obj_name) LATTR_MAKE_UNIQUE_ID(setter_adder_instance_, id, obj_name); \
void LATTR_MAKE_UNIQUE_ID(setter_, id, obj_name)::set([[maybe_unused]] lua_State* L, LATTR_MAKE_UNIQUE_ID(setter_, id, obj_name)::object_type& obj_name, const value_type& value) const


#define LATTR_VALID5(name, obj_type, obj_name, id) \
struct LATTR_MAKE_UNIQUE_ID(check_, id, obj_name) : public lua_validator<obj_type> { \
	using object_type = obj_type; \
	bool is_active(lua_State* L, const object_type& obj_name) const override; \
}; \
struct LATTR_MAKE_UNIQUE_ID(check_adder_, id, obj_name) { \
	LATTR_MAKE_UNIQUE_ID(check_adder_, id, obj_name) ()\
	{ \
		register_lua_attribute<obj_type, void, LATTR_MAKE_UNIQUE_ID(check_, id, obj_name), lua_attrfunc_type::validator>(name); \
	} \
}; \
static LATTR_MAKE_UNIQUE_ID(check_adder_, id, obj_name) LATTR_MAKE_UNIQUE_ID(check_adder_instance_, id, obj_name); \
bool LATTR_MAKE_UNIQUE_ID(check_, id, obj_name)::is_active([[maybe_unused]] lua_State* L, const LATTR_MAKE_UNIQUE_ID(check_, id, obj_name)::object_type& obj_name) const


/**
 * @param name: string  attribute name
 * @param value_type: the type of the attribute, for example int or std::string
 * @param obj_type: the type of the object, for example lua_unit
 * @param obj_name: a name for the variable that will hold the object
 */
#define LATTR_GETTER(name, value_type, obj_type, obj_name) LATTR_GETTER5(name, value_type, obj_type, obj_name, __LINE__)

#define LATTR_SETTER(name, value_type, obj_type, obj_name) LATTR_SETTER5(name, value_type, obj_type, obj_name, __LINE__)

#define LATTR_VALID(name, obj_type, obj_name) LATTR_VALID5(name, obj_type, obj_name, __LINE__)
