for i, side in ipairs(wesnoth.get_sides({})) do
	wml.variables["p" .. tostring(i) .. "_faction"] = side.faction
end
