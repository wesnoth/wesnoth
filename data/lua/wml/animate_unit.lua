local helper = wesnoth.require "helper"
local T = helper.set_wml_tag_metatable{}

local function add_animation(anim, cfg)
	cfg = helper.shallow_parsed(cfg)
	local flag = cfg.flag or helper.wml_error("[animate_unit] is missing flag")
	local filter = helper.get_child(cfg, "filter")
	local unit
	if filter then
		unit = wesnoth.get_units{
			limit = 1,
			T["and"](filter)
		}[1]
	else
		unit = wesnoth.get_unit(
			wesnoth.current.event_context.x1,
			wesnoth.current.event_context.y1
		)
	end

	if unit and not wesnoth.is_fogged(wesnoth.current.side, unit.x, unit.y) then
		local primary = helper.get_child(cfg, "primary_attack")
		local secondary = helper.get_child(cfg, "secondary_attack")
		-- We don't have access to the secondary unit at this point.
		-- Thus, for the secondary attack, we just create a dummy attack
		-- which exactly matches the filter.
		-- TODO: Is there a way to fix this? Does it even need fixing?
		if cfg.flag == "death" or cfg.flag == "victory" then
			-- death and victory animations have their weapons swapped
			-- The primary weapon in a death animation is the weapon that killed the unit.
			-- In other words, it's the weapon of the attacker, the secondary unit.
			-- The secondary weapon is the weapon of the defender, the primary unit.
			-- In a victory animation, these are reversed, so the primary weapon is
			-- the weapon of the defender, who is now the secondary unit.
			-- Similarly, the secondary weapon in a victory animation is the weapon
			-- of the attacker, who is now the primary unit.
			if primary then
				primary = wesnoth.create_weapon(primary)
			end
			if secondary then
				secondary = helper.find_attack(unit, secondary)
			end
		else
			if primary then
				primary = helper.find_attack(unit, primary)
			end
			if secondary then
				secondary = wesnoth.create_weapon(secondary)
			end
		end

		local hits = cfg.hits
		if hits == true or hits == nil then
			hits = 'hit'
		elseif hits == false then
			hits = 'miss'
		end

		local color = {0xff, 0xff, 0xff}
		if cfg.red or cfg.green or cfg.blue then
			-- This tonumber() or 0 is to ensure they're all definitely numbers
			-- It works because tonumber() returns nil if its argument is not a number
			color = {
				tonumber(cfg.red) or 0,
				tonumber(cfg.green) or 0,
				tonumber(cfg.blue) or 0
			}
		end

		-- TODO: The last argument is currently unused
		-- (should make the game not scroll if view locked or prefs disables it)
		wesnoth.scroll_to_tile(unit.x, unit.y, true, false, true, false)

		local facing = helper.get_child(cfg, "facing")
		if facing then
			local facing_loc = wesnoth.get_locations(facing)[1]
			if facing_loc then
				local dir = wesnoth.map.get_relative_dir(unit.x, unit.y, facing_loc[1], facing_loc[2])
				if dir == "" then
					dir = "nw"
				end
				unit.facing = dir
				facing = wesnoth.map.get_direction(unit.x, unit.y, dir)
			else
				facing = nil
			end
		end

		local text = cfg.text
		if cfg.female_text and unit.gender == 'female' then
			text = cfg.female_text
		elseif cfg.male_text and unit.gender == 'male' then
			text = cfg.male_text
		end

		anim:add(unit, flag, hits, {
			target = facing,
			value = {tonumber(cfg.value) or 0, tonumber(cfg.value_second) or 0},
			with_bars = not not cfg.with_bars,
			text = text,
			color = color,
			primary = primary,
			secondary = secondary
		})
	end

	for c in helper.child_range(cfg, "animate") do
		add_animation(anim, c)
	end
end

function wesnoth.wml_actions.animate_unit(cfg)
	local anim = wesnoth.create_animator()
	add_animation(anim, cfg)
	anim:run()
end