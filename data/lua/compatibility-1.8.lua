--! #textdomain wesnoth

-- This file provides an implementation of all the Lua functions removed
-- from the engine between 1.8 and 1.10.

function wesnoth.get_side(i)
	return wesnoth.sides[i]
end

function wesnoth.get_side_count()
	return #wesnoth.sides
end

function wesnoth.get_unit_type_ids()
	local t = {}
	for k,v in pairs(wesnoth.unit_types) do
		table.insert(t, k)
	end
	table.sort(t)
	return t
end

function wesnoth.get_unit_type(t)
	return wesnoth.unit_types[t]
end

function wesnoth.register_wml_action(name, handler)
	local old = wesnoth.wml_actions[name]
	wesnoth.wml_actions[name] = handler
	return old
end

function wesnoth.fire(name, cfg)
	wesnoth.wml_actions[name](wesnoth.tovconfig(cfg or {}))
end
