
local utils = wesnoth.require "wml-utils"
local T = wml.tag
local wml_actions = wesnoth.wml_actions

local used_items = {}

function wml_actions.object(cfg)
	local context = wesnoth.current.event_context

	-- If this item has already been used
	local unique = cfg.take_only_once
	if unique == nil then unique = true end
	local obj_id = utils.check_key(cfg.id, "id", "object", true)
	if obj_id and unique and used_items[obj_id] then return end

	local unit, command_type, text

	local filter = wml.get_child(cfg, "filter")
	if filter then
		unit = wesnoth.units.find_on_map(filter)[1]
	else
		unit = wesnoth.units.get(context.x1, context.y1)
	end

	-- If a unit matches the filter, proceed
	if unit then
		text = tostring(cfg.description or "")
	else
		text = tostring(cfg.cannot_use_message or "")
		command_type = "else"
	end

	-- Default to silent if object has no description
	local silent = cfg.silent
	if silent == nil then silent = (text:len() == 0) end

	if unit then
		command_type = "then"

		if cfg.no_write ~= nil then
			wesnoth.log("wml", "[object]no_write=yes is deprecated in favour of placing [effect] tags in [modify_unit]")
		end

		local dvs = cfg.delayed_variable_substitution
		local add = cfg.no_write ~= true
		if dvs then
			unit:add_modification("object", wml.literal(cfg), add)
		else
			unit:add_modification("object", wml.parsed(cfg), add)
		end

		if not silent then
			unit:select(false)
			wesnoth.interface.highlight_hex(unit.x, unit.y)
		end

		-- Mark this item as used up
		if obj_id and unique then used_items[obj_id] = true end
	end

	if not silent then
		wml_actions.redraw{}
		local name = tostring(cfg.name or "")
		gui.show_popup(name, text, cfg.image)
	end

	for cmd in wml.child_range(cfg, command_type) do
		local action = utils.handle_event_commands(cmd, "conditional")
		if action ~= "none" then break end
	end
end

function wesnoth.persistent_tags.used_items.read(cfg)
	used_items = cfg
end

function wesnoth.persistent_tags.used_items.write(add)
	add(used_items)
end

function wml_actions.remove_object(cfg)
	local obj_id = cfg.object_id
	for _,unit in ipairs(wesnoth.units.find_on_map(cfg)) do
		unit:remove_modifications({id = obj_id})
	end
end

function wesnoth.wml_conditionals.found_item(cfg)
	return used_items[utils.check_key(cfg.id, "id", "found_item", true)]
end
