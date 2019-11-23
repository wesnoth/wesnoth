for i, side in ipairs(wesnoth.sides.find({})) do
	wml.variables["p" .. tostring(i) .. "_faction"] = side.faction
end
