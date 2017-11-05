for i, side in ipairs(wesnoth.get_sides({})) do
	wesnoth.set_variable("p" .. tostring(i) .. "_faction", side.faction)
end
