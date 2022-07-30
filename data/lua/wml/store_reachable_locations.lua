local location_set = wesnoth.require "location_set"

function wesnoth.wml_actions.store_reachable_locations(cfg)
	local unit_filter = wml.get_child(cfg, "filter") or
		wml.error "[store_reachable_locations] missing required [filter] tag"
	local location_filter = wml.get_child(cfg, "filter_location")
	local range = cfg.range or "movement"
	local moves = cfg.moves or "current"
	local variable = cfg.variable or wml.error "[store_reachable_locations] missing required variable= key"
	local reach_param = { viewing_side = cfg.viewing_side }
	if cfg.viewing_side == 0 then
		wml.error "[store_reachable_locations] invalid viewing_side"
	elseif cfg.viewing_side == nil then
		reach_param.ignore_visibility = true
	end

	local reach = location_set.create()

	if range == "vision" then
		for i,unit in ipairs(wesnoth.units.find_on_map(unit_filter)) do
			local unit_reach = location_set.of_pairs(wesnoth.paths.find_vision_range(unit))
			reach:union(unit_reach)
		end
	else
		for i,unit in ipairs(wesnoth.units.find_on_map(unit_filter)) do
			local unit_reach
			if moves == "max" then
				local saved_moves = unit.moves
				unit.moves = unit.max_moves
				unit_reach = location_set.of_pairs(wesnoth.paths.find_reach(unit, reach_param))
				unit.moves = saved_moves
			else
				unit_reach = location_set.of_pairs(wesnoth.paths.find_reach(unit, reach_param))
			end

			if range == "attack" then
				unit_reach:iter(function(x, y)
					reach:insert(x, y)
					for u,v in wesnoth.current.map:iter_adjacent(x, y) do
						reach:insert(u, v)
					end
				end)
			else
				reach:union(unit_reach)
			end
		end
	end

	if location_filter then
		reach = reach:filter(function(x, y)
			return wesnoth.map.matches(x, y, location_filter)
		end)
	end
	reach:to_wml_var(variable)
end
