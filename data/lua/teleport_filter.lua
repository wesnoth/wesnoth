function teleport_source_filter(x, y)
	if wesnoth.get_village_owner(x, y) ~= wesnoth.get_variable("teleport_unit.side") then
		return false
	end
	local blocking_unit = wesnoth.get_unit(x, y)
	return (not blocking_unit) or blocking_unit.id == wesnoth.get_variable("teleport_unit.id")
end

function teleport_target_filter(x, y)
	if wesnoth.get_village_owner(x, y) ~= wesnoth.get_variable("teleport_unit.side") then
		return false
	end
	local blocking_unit = wesnoth.get_unit(x, y)
	return not blocking_unit
end

function backstab_defender_filter(defender)

	local attacker_x = wesnoth.get_variable("other_unit.x")
	local attacker_y = wesnoth.get_variable("other_unit.y")
	
	local defender_x = defender.x
	local defender_y = defender.y
	
	local adjacent = {wesnoth.map.get_adjacent_tiles(defender_x, defender_y)}

	local attacker_pos_index = nil
	for i,v in ipairs(adjacent) do
		if v[1] == attacker_x and v[2] == attacker_y then
			attacker_pos_index = i
		end
	end
	
	if attacker_pos_index == nil then
		-- Attack not from adjacent location
		return false
	end
	
	local opposite_pos_index = (((attacker_pos_index + 3) - 1) % 6) + 1
	local opposite_unit = wesnoth.get_unit(adjacent[opposite_pos_index][1], adjacent[opposite_pos_index][2])

	if not opposite_unit then
		-- No opposite unit
		return false
	end

	if opposite_unit.status.petrified then
		return false
	end
	
	return wesnoth.is_enemy(opposite_unit.side, defender.side)
end

for i = 1, 5 do
	_G["leadership_receiver_filter_" .. i] = function(receiver)
		return receiver.level == wesnoth.get_variable("other_unit.level") - i
	end
end
