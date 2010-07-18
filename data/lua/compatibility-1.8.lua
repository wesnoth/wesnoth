-- This file provides an implementation of all the Lua functions removed
-- from the engine between 1.8 and 1.10.

function wesnoth.get_side(i)
	return wesnoth.sides[i]
end

function wesnoth.get_side_count(i)
	return #wesnoth.sides
end
