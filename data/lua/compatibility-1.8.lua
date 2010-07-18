-- This file provides an implementation of all the Lua functions removed
-- from the engine between 1.8 and 1.10.

function wesnoth.get_side(i)
	return wesnoth.sides[i]
end

function wesnoth.get_side_count(i)
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
