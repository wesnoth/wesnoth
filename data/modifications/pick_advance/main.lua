-- << pick_advance/main.lua

local on_event = wesnoth.require "on_event"
local F = wesnoth.require "functional"
local T = wml.tag
local _ = wesnoth.textdomain "wesnoth"

wesnoth.wml_actions.set_menu_item {
	id = "pickadvance",
	description = _ "Plan Advancement",
	T.show_if {
		T.lua {
			code = "return pickadvance.menu_available()"
		},
	},
	T.command {
		T.lua {
			code = "pickadvance.pick_advance()"
		}
	}
}

-- replace any non-alphanumeric characters with an underscore
local function clean_type_func(unit_type)
	return string.gsub(unit_type, "[^a-zA-Z0-9]", "_")
end

-- splits a comma delimited string of unit types
-- returns a table of unit types that aren't blank, "null", and that exist
local function split_comma_units(string_to_split)
	return F.filter(
		stringx.split(string_to_split or ""),
		function(s) return s ~= "" and s ~= "null" and wesnoth.unit_types[s] end
	)
end

-- returns a table of the original unit types
--         a comma delimited string containing the same values
local function original_advances(unit)
	local clean_type = clean_type_func(unit.type)
	local variable = unit.variables["pickadvance_orig_" .. clean_type] or ""
	return split_comma_units(variable), clean_type_func(variable)
end

-- replace the unit's current advancements with the new set of units via object/effect
local function set_advances(unit, array)
	unit:add_modification("object", {
		pickadvance = true,
		take_only_once = false,
		T.effect {
			apply_to = "new_advancement",
			replace = true,
			types = array
		}
	})
end

-- for table "arr" containing sets of [index,unit_type]
-- return table containing sets of [unit_type,true]
local function array_to_set(arr)
	local result = {}
	for _, v in ipairs(arr) do
		result[v] = true
	end
	return result
end

-- works as anti-cheat and fixes tricky bugs in [male]/[female]/undead variation overrides
local function filter_overrides(unit, overrides)
	local possible_advances_array = original_advances(unit)
	local possible_advances = array_to_set(possible_advances_array)
	local filtered = F.filter(overrides, function(e) return possible_advances[e] end)
	return #filtered > 0 and filtered or possible_advances_array
end

-- returns a table with the unit's original advancements
--         the unit's currently overridden advancement or nil if not set
--         the unit's currently overridden advancement or nil if not set, but set by some other mechanism from the current game
local function get_advance_info(unit)
	local type_advances, orig_options_sanitized = original_advances(unit)
	local game_override_key = "pickadvance_side" .. unit.side .. "_" .. orig_options_sanitized
	local game_override = wml.variables[game_override_key]
	local function correct(override)
		return override and #override > 0 and #override < #type_advances and override or nil
	end

	return {
		type_advances = type_advances,
		unit_override = correct(unit.advances_to),
		game_override = correct(split_comma_units(game_override)),
	}
end

-- true if there's a unit at the selected hex
--      the unit has advancements
--      the unit is on a local human controlled side
--      the unit has multiple options in either its original set of advancements or current set of advancements
function pickadvance.menu_available()
	local unit = wesnoth.units.get(wml.variables.x1, wml.variables.y1)
	return unit and
		#unit.advances_to > 0
		and wesnoth.sides[unit.side].is_local and wesnoth.sides[unit.side].controller == "human"
		and (#original_advances(unit) > 1 or #unit.advances_to > 1)
end

-- if the unit doesn't have a set of original advancements present, remove any existing "pickadvance" object
-- set the unit's original advancements in its variables
-- and then set the unit's advancement to either a game-provided override or its default advancements
local function initialize_unit(unit)
	local clean_type = clean_type_func(unit.type)
	if unit.variables["pickadvance_orig_" .. clean_type] == nil then
		unit:remove_modifications{
			pickadvance = true
		}
		unit.variables["pickadvance_orig_" .. clean_type] = table.concat(unit.advances_to, ",")
		local advance_info = get_advance_info(unit)
		local desired = advance_info.game_override or unit.advances_to
		desired = filter_overrides(unit, desired)
		set_advances(unit, desired)
	end
end

-- let the player select the unit's advancement via dialog
function pickadvance.pick_advance(unit)
	unit = unit or wesnoth.units.get(wml.variables.x1, wml.variables.y1)
	initialize_unit(unit)
	local _, orig_options_sanitized = original_advances(unit)
	local dialog_result = wesnoth.synchronize_choice(function()
		local local_result = pickadvance.show_dialog_unsynchronized(get_advance_info(unit), unit)
		return local_result
	end, function() return { is_ai = true } end)
	if dialog_result.ignore or dialog_result.is_ai then
		return
	end
	dialog_result.unit_override = split_comma_units(dialog_result.unit_override)
	dialog_result.game_override = split_comma_units(dialog_result.game_override)
	dialog_result.unit_override = filter_overrides(unit, dialog_result.unit_override)
	dialog_result.game_override = filter_overrides(unit, dialog_result.game_override)
	if dialog_result.is_unit_override then
		set_advances(unit, dialog_result.unit_override)
	end
	if dialog_result.is_game_override then
		local key = "pickadvance_side" .. unit.side .. "_" .. orig_options_sanitized
		wml.variables[key] = table.concat(dialog_result.game_override, ",")
	end
end

-- make unit advancement tree viewable in the ingame help
local known_units = {}
local function make_unit_known(unit)  -- can be both unit or unit type
	local type = unit.type or unit.id
	if known_units[type] then return end
	known_units[type] = true
	wesnoth.add_known_unit(type)
	for _, advance in ipairs(unit.advances_to) do
		make_unit_known(wesnoth.unit_types[advance])
	end
end

-- initialize a unit for picking an advancement
-- make its advancements viewable
-- force picking an advancement if it has multiple and the force option was specified
local function initialize_unit_x1y1(ctx)
	local unit = wesnoth.units.get(ctx.x1, ctx.y1)
	if not wesnoth.sides[unit.side].__cfg.allow_player then return end
	initialize_unit(unit)
	make_unit_known(unit)
	if #unit.advances_to > 1 and wml.variables.pickadvance_force_choice and unit.side == wesnoth.current.side then
		pickadvance.pick_advance(unit)
	end
end

-- return true if the side can be played and has either a recruit list set or non-leader units
local function humans_can_recruit()
	for _, side in ipairs(wesnoth.sides) do
		local units = wesnoth.units.find_on_map { side = side.side, canrecruit = false }
		if side.__cfg.allow_player and (#side.recruit ~= 0 or #units > 0) then
			return true
		end
	end
end
-- return true if any keeps exist
local function map_has_keeps()
	for x, y in wesnoth.current.map:iter() do
		local terr = wesnoth.current.map[{x, y}]
		local info = wesnoth.terrain_types[terr]
		if info.keep then
			return true
		end
	end
end

-- on start determine whether choosing an advancement is force for each unit
on_event("start", function()
	local map_has_recruits = humans_can_recruit() and map_has_keeps()
	wml.variables.pickadvance_force_choice = wml.variables.pickadvance_force_choice or not map_has_recruits
end)

-- set "fresh_turn" for the moveto event at the start of each side turn
local fresh_turn = false
on_event("turn refresh", function()
	fresh_turn = true
end)

-- the first time a unit moves at the start of each side's turn, check if there are any new units that need to be forced to make an advancement choice
on_event("moveto", function()
	if fresh_turn then
		fresh_turn = false
		if not wesnoth.sides[wesnoth.current.side].__cfg.allow_player then return end
		for _, unit in ipairs(wesnoth.units.find_on_map { side = wesnoth.current.side }) do
			if #unit.advances_to > 1 and wml.variables.pickadvance_force_choice and wesnoth.current.turn > 1 then
				pickadvance.pick_advance(unit)
				if #unit.advances_to > 1 then
					local len = #unit.advances_to
					local rand = wesnoth.random(len)
					unit.advances_to = { unit.advances_to[rand] }
				end
			else
				initialize_unit(unit)
			end
		end
	end
end)

-- initialize units on recruit and after advancing, forcing another advancement choice if required
on_event("recruit", initialize_unit_x1y1)
on_event("post advance", initialize_unit_x1y1)


-- >>
