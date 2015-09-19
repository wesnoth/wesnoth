
local helper = wesnoth.require "lua/helper.lua"
local utils = wesnoth.require "lua/wml-utils.lua"
local T = helper.set_wml_tag_metatable {}
local wml_actions = wesnoth.wml_actions

local used_items = {}

function wml_actions.object(cfg)
	local context = wesnoth.current.event_context

	-- If this item has already been used
	local obj_id = utils.check_key(cfg.id, "id", "object", true)
	if obj_id and used_items[obj_id] then return end

	local unit, command_type, text

	if helper.get_child(cfg, "filter") then
		unit = wesnoth.get_units(filter)[1]
	else
		unit = wesnoth.get_unit(contxt.x, context.y)
	end

	-- If a unit matches the filter, proceed
	if unit then
		text = tostring(cfg.description or "")
		command_type = "then"

		local dvs = cfg.delayed_variable_substitution
		local add = cfg.no_write ~= true
		if dvs then
			wesnoth.add_modification(unit, "object", helper.literal(cfg), add)
		else
			wesnoth.add_modification(unit, "object", helper.parsed(cfg), add)
		end

		wesnoth.select_hex(unit.x, unit.y)

		-- Mark this item as used up
		if obj_id then used_items[obj_id] = true end
	else
		text = tostring(cfg.cannot_use_message or "")
		command_type = "else"
	end

	-- Default to silent if object has no description
	local silent = cfg.silent
	if silent == nil then silent = (text:len() == 0) end

	if not silent then
		wml_actions.redraw{}
		local name = tostring(cfg.name or "")
		wesnoth.show_popup_dialog(name, text, cfg.image)
	end

	for cmd in helper.child_range(cfg, command_type) do
		utils.handle_event_commands(cmd)
	end
end

local old_on_load = wesnoth.game_events.on_load
function wesnoth.game_events.on_load(cfg)
	for i = 1,#cfg do
		if cfg[i][1] == "used_items" then
			-- Not quite sure if this will work
			-- Might need to loop through and copy each ID separately
			used_items = cfg[i][2]
			table.remove(cfg, i)
			break
		end
	end
	old_on_load(cfg)
end

local old_on_save = wesnoth.game_events.on_save
function wesnoth.game_events.on_save()
	local cfg = old_on_save()
	table.insert(cfg, T.used_items(used_items) )
	return cfg
end

function wesnoth.wml_conditionals.found_item(cfg)
	return used_items[utils.check_key(cfg.id, "id", "found_item", true)]
end
