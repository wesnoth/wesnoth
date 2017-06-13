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
