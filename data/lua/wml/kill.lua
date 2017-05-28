local helper = wesnoth.require "helper"
local location_set = wesnoth.require "location_set"

local kill_recursion_preventer = location_set.create()

function wesnoth.wml_actions.kill(cfg)
	local number_killed = 0
	local secondary_unit = helper.get_child(cfg, "secondary_unit")
	local killer_loc = {0, 0}
	if secondary_unit then
		secondary_unit = wesnoth.get_units(secondary_unit)[1]
		if cfg.fire_event then
			if secondary_unit then
				killer_loc = {secondary_unit.loc}
			else
				wesnoth.log("warn", "failed to match [secondary_unit] in [kill] with a single on-board unit")
			end
		end
	end
	local dead_men_walking = wesnoth.get_units(cfg)
	for i,unit in ipairs(dead_men_walking) do
		local death_loc = {x = tonumber(unit.x) or 0, y = tonumber(unit.y) or 0}
		if not secondary_unit then killer_loc = death_loc end
		local can_fire = false

		local recursion = (kill_recursion_preventer:get(death_loc.x, death_loc.y) or 0) + 1
		if cfg.fire_event then
			kill_recursion_preventer:insert(death_loc.x, death_loc.y, recursion)
			can_fire = true
			if death_loc.x == wesnoth.current.event_context.x1 and death_loc.y == wesnoth.current.event_context.y1 then
				if wesnoth.current.event_context.name == "die" or wesnoth.current.event_context.name == "last breath" then
					if recursion >= 10 then
						can_fire = false;
						wesnoth.log("error", "tried to fire 'die' or 'last breath' event on unit from the unit's 'die' or 'last breath' event with first_time_only=no!")
					end
				end
			end
		end
		if can_fire then
			wesnoth.fire_event("last breath", death_loc, killer_loc)
		end
		if cfg.animate then
			wesnoth.scroll_to_tile(death_loc)
			local anim = wesnoth.create_animator()
			local primary = helper.get_child(cfg, "primary_attack")
			local secondary = helper.get_child(cfg, "secondary_attack")
			-- Yes, we get the primary attack from the secondary unit and vice versa
			-- The primary attack in a death animation is the weapon that caused the death
			-- In other words, the attacker's weapon. The attacker is the secondary unit.
			-- In the victory animation, this is simply swapped.
			if primary then
				if secondary_unit then
					primary = helper.find_attack(secondary_unit, primary)
				else
					primary = wesnoth.create_weapon(primary)
				end
				wesnoth.log('err', "Primary weapon:\n" .. wml.tostring(primary.__cfg))
			end
			if secondary then
				if primary_unit then
					secondary = helper.find_attack(unit, secondary)
				else
					secondary = wesnoth.create_weapon(secondary)
				end
				wesnoth.log('err', "Secondary weapon:\n" .. wml.tostring(secondary.__cfg))
			end
			anim:add(unit, "death", "kill", {primary = primary, secondary = secondary})
			if secondary_unit then
				anim:add(secondary_unit, "victory", "kill", {primary = secondary, secondary = primary})
			end
			anim:run()
		end
		wesnoth.wml_actions.redraw{}

		if can_fire then
			wesnoth.fire_event("die", death_loc, killer_loc)
		end
		if cfg.fire_event then
			if recursion <= 1 then
				kill_recursion_preventer:remove(death_loc.x, death_loc.y)
			else
				kill_recursion_preventer:insert(death_loc.x, death_loc.y, recursion)
			end
		end
		-- Test that it's valid (and still on the map) first, in case the event erased (or extracted) it.
		if unit.valid == "map" then unit:erase() end

		number_killed = number_killed + 1
	end

	if (cfg.x == "recall" or cfg.x == nil) and (cfg.y == "recall" or cfg.y == nil) then
		local dead_men_sleeping = wesnoth.get_recall_units(cfg)
		for i,unit in ipairs(dead_men_sleeping) do
			unit:erase()
		end
		number_killed = number_killed + #dead_men_sleeping
	end

	return number_killed
end
