--! #textdomain wesnoth

-- This file may provide an implementation of Lua functions removed from the engine.

local helper = wesnoth.require "lua/helper.lua"

local get_side = function(i)
	return wesnoth.sides[i]
end
wesnoth.get_side = helper.deprecate("wesnoth.get_side is deprecated, use wesnoth.sides instead", get_side)

local get_side_count = function()
	return #wesnoth.sides
end
wesnoth.get_side_count = helper.deprecate("wesnoth.get_side_count is deprecated, use #wesnoth.sides instead", get_side_count)

local get_unit_type_ids = function()
	local t = {}
	for k,v in pairs(wesnoth.unit_types) do
		table.insert(t, k)
	end
	table.sort(t)
	return t
end
wesnoth.get_unit_type_ids = helper.deprecate("wesnoth.get_unit_type_ids is deprecated, use wesnoth.unit_types instead", get_unit_type_ids)

local get_unit_type = function(t)
	return wesnoth.unit_types[t]
end
wesnoth.get_unit_type = helper.deprecate("wesnoth.get_unit_type is deprecated, use wesnoth.unit_types instead", get_unit_type)

local register_wml_action = function(name, handler)
	local old = wesnoth.wml_actions[name]
	wesnoth.wml_actions[name] = handler
	return old
end
wesnoth.register_wml_action = helper.deprecate("wesnoth.register_wml_action is deprecated, use wesnoth.wml_actions instead", register_wml_action)

-- Calling wesnoth.fire isn't the same as calling wesnoth.wml_actions[name] due to the passed vconfig userdata
-- which also provides "constness" of the passed wml object from the point of view of the caller.
-- So please don't remove since it's not deprecated.
function wesnoth.fire(name, cfg)
	wesnoth.wml_actions[name](wesnoth.tovconfig(cfg or {}))
end
