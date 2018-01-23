local helper = wesnoth.require "helper"
local utils = wesnoth.require "wml-utils"
local wml_actions = wesnoth.wml_actions
local T = helper.set_wml_tag_metatable {}

function wml_actions.harm_unit(cfg)
	local filter = helper.get_child(cfg, "filter") or helper.wml_error("[harm_unit] missing required [filter] tag")
	-- we need to use shallow_literal field, to avoid raising an error if $this_unit (not yet assigned) is used
	if not helper.shallow_literal(cfg).amount then helper.wml_error("[harm_unit] has missing required amount= attribute") end
	local variable = cfg.variable -- kept out of the way to avoid problems
	local _ = wesnoth.textdomain "wesnoth"
	-- #textdomain wesnoth
	local harmer

	local function toboolean( value ) -- helper for animate fields
		-- units will be animated upon leveling or killing, even
		-- with special attacker and defender values
		if value then return true
		else return false end
	end

	local this_unit = utils.start_var_scope("this_unit")

	for index, unit_to_harm in ipairs(wesnoth.get_units(filter)) do
		if unit_to_harm.valid then
			-- block to support $this_unit
			wesnoth.set_variable ( "this_unit" ) -- clearing this_unit
			wesnoth.set_variable("this_unit", unit_to_harm.__cfg) -- cfg field needed
			local amount = tonumber(cfg.amount)
			local animate = cfg.animate -- attacker and defender are special values
			local delay = cfg.delay or 500
			local kill = cfg.kill
			local fire_event = cfg.fire_event
			local primary_attack = helper.get_child(cfg, "primary_attack")
			local secondary_attack = helper.get_child(cfg, "secondary_attack")
			local harmer_filter = helper.get_child(cfg, "filter_second")
			local experience = cfg.experience
			local resistance_multiplier = tonumber(cfg.resistance_multiplier) or 1
			if harmer_filter then harmer = wesnoth.get_units(harmer_filter)[1] end
			-- end of block to support $this_unit

			if animate then
				if animate ~= "defender" and harmer and harmer.valid then
					wesnoth.scroll_to_tile(harmer.x, harmer.y, true)
					wml_actions.animate_unit {
						flag = "attack",
						hits = true,
						with_bars = true,
						T.filter { id = harmer.id },
						T.primary_attack ( primary_attack ),
						T.secondary_attack ( secondary_attack ),
						T.facing { x = unit_to_harm.x, y = unit_to_harm.y },
					}
				end
				wesnoth.scroll_to_tile(unit_to_harm.x, unit_to_harm.y, true)
			end

			-- the two functions below are taken straight from the C++ engine,
			-- utils.cpp and actions.cpp, with a few unuseful parts removed
			-- may be moved in helper.lua in 1.11
			local function round_damage( base_damage, bonus, divisor )
				local rounding
				if base_damage == 0 then return 0
				else
					if bonus < divisor or divisor == 1 then
						rounding = divisor / 2 - 0
					else
						rounding = divisor / 2 - 1
					end
					return math.max( 1, math.floor( ( base_damage * bonus + rounding ) / divisor ) )
				end
			end

			local function calculate_damage( base_damage, alignment, tod_bonus, resistance, modifier )
				local damage_multiplier = 100
				if alignment == "lawful" then
					damage_multiplier = damage_multiplier + tod_bonus
				elseif alignment == "chaotic" then
					damage_multiplier = damage_multiplier - tod_bonus
				elseif alignment == "liminal" then
					damage_multiplier = damage_multiplier - math.abs( tod_bonus )
				else -- neutral, do nothing
				end
				local resistance_modified = resistance * modifier
				damage_multiplier = damage_multiplier * resistance_modified
				local damage = round_damage( base_damage, damage_multiplier, 10000 ) -- if harmer.status.slowed, this may be 20000 ?
				return damage
			end

			local damage = calculate_damage(
				amount,
				cfg.alignment or "neutral",
				wesnoth.get_time_of_day( { unit_to_harm.x, unit_to_harm.y, true } ).lawful_bonus,
				wesnoth.unit_resistance( unit_to_harm, cfg.damage_type or "dummy" ),
				resistance_multiplier
			)

			if unit_to_harm.hitpoints <= damage then
				if kill == false then damage = unit_to_harm.hitpoints - 1
				else damage = unit_to_harm.hitpoints
				end
			end

			unit_to_harm.hitpoints = unit_to_harm.hitpoints - damage
			local text = string.format("%d%s", damage, "\n")
			local add_tab = false
			local gender = unit_to_harm.__cfg.gender

			local function set_status(name, male_string, female_string, sound)
				if not cfg[name] or unit_to_harm.status[name] then return end
				if gender == "female" then
					text = string.format("%s%s%s", text, tostring(female_string), "\n")
				else
					text = string.format("%s%s%s", text, tostring(male_string), "\n")
				end

				unit_to_harm.status[name] = true
				add_tab = true

				if animate and sound then -- for unhealable, that has no sound
					wesnoth.play_sound (sound)
				end
			end

			if not unit_to_harm.status.unpoisonable then
				set_status("poisoned", _"poisoned", _"female^poisoned", "poison.ogg")
			end
			set_status("slowed", _"slowed", _"female^slowed", "slowed.wav")
			set_status("petrified", _"petrified", _"female^petrified", "petrified.ogg")
			set_status("unhealable", _"unhealable", _"female^unhealable")

			-- Extract unit and put it back to update animation if status was changed
			wesnoth.extract_unit(unit_to_harm)
			wesnoth.put_unit(unit_to_harm)

			if add_tab then
				text = string.format("%s%s", "\t", text)
			end

			if animate and animate ~= "attacker" then
				if harmer and harmer.valid then
					wml_actions.animate_unit {
						flag = "defend",
						hits = true,
						with_bars = true,
						T.filter { id = unit_to_harm.id },
						T.primary_attack ( primary_attack ),
						T.secondary_attack ( secondary_attack ),
						T.facing { x = harmer.x, y = harmer.y },
					}
				else
					wml_actions.animate_unit {
						flag = "defend",
						hits = true,
						with_bars = true,
						T.filter { id = unit_to_harm.id },
						T.primary_attack ( primary_attack ),
						T.secondary_attack ( secondary_attack ),
					}
				end
			end

			wesnoth.float_label( unit_to_harm.x, unit_to_harm.y, string.format( "<span foreground='red'>%s</span>", text ) )

			local function calc_xp( level ) -- to calculate the experience in case of kill
				if level == 0 then return 4
				else return level * 8 end
			end

			if experience ~= false and harmer and harmer.valid
				and wesnoth.is_enemy( unit_to_harm.side, harmer.side )
			then
				if kill ~= false and unit_to_harm.hitpoints <= 0 then
					harmer.experience = harmer.experience + calc_xp( unit_to_harm.__cfg.level )
				else
					unit_to_harm.experience = unit_to_harm.experience + harmer.__cfg.level
					harmer.experience = harmer.experience + unit_to_harm.__cfg.level
				end
			end

			if kill ~= false and unit_to_harm.hitpoints <= 0 then
				wml_actions.kill { id = unit_to_harm.id, animate = toboolean( animate ), fire_event = fire_event, harmer and T.secondary_unit { id = harmer.id } }
			end

			if animate then
				wesnoth.delay(delay)
			end

			if variable then
				wesnoth.set_variable(string.format("%s[%d]", variable, index - 1), { harm_amount = damage })
			end

			-- both units may no longer be alive at this point, so double check
			if experience ~= false and unit_to_harm and unit_to_harm.valid then
				unit_to_harm:advance(toboolean(animate), fire_event ~= false)
			end

			if experience ~= false and harmer and harmer.valid then
				harmer:advance(toboolean(animate), fire_event ~= false)
			end
		end

		wml_actions.redraw {}
	end

	wesnoth.set_variable ( "this_unit" ) -- clearing this_unit
	utils.end_var_scope("this_unit", this_unit)
end