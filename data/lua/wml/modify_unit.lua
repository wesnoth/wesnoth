local helper = wesnoth.require "helper"
local utils = wesnoth.require "wml-utils"
local wml_actions = wesnoth.wml_actions

function wml_actions.modify_unit(cfg)
	local unit_variable = "LUA_modify_unit"

	local replace_mode = cfg.mode == "replace"

	local function handle_attributes(cfg, unit_path, toplevel)
		for current_key, current_value in pairs(helper.shallow_parsed(cfg)) do
			if type(current_value) ~= "table" and (not toplevel or (current_key ~= "type" and current_key ~= "mode")) then
				wesnoth.set_variable(string.format("%s.%s", unit_path, current_key), current_value)
			end
		end
	end

	local function handle_child(cfg, unit_path)
		local children_handled = {}
		local cfg = helper.shallow_parsed(cfg)
		handle_attributes(cfg, unit_path)

		for current_index, current_table in ipairs(cfg) do
			local current_tag = current_table[1]
			local tag_index = children_handled[current_tag] or 0
			handle_child(current_table[2], string.format("%s.%s[%u]",
				unit_path, current_tag, tag_index))
			children_handled[current_tag] = tag_index + 1
		end
	end

	local filter = helper.get_child(cfg, "filter") or helper.wml_error "[modify_unit] missing required [filter] tag"
	local function handle_unit(unit_num)
		local children_handled = {}
		local unit_path = string.format("%s[%u]", unit_variable, unit_num)
		local this_unit = wesnoth.get_variable(unit_path)
		wesnoth.set_variable("this_unit", this_unit)
		handle_attributes(cfg, unit_path, true)

		for current_index, current_table in ipairs(helper.shallow_parsed(cfg)) do
			local current_tag = current_table[1]
			if current_tag == "filter" then
				-- nothing
			elseif current_tag == "object" or current_tag == "trait" or current_tag == "advancement" then
				local mod = current_table[2]
				if mod.delayed_variable_substitution then
					mod = helper.literal(mod)
				else
					mod = helper.parsed(mod)
				end
				local unit = wesnoth.get_variable(unit_path)
				unit = wesnoth.create_unit(unit)
				wesnoth.add_modification(unit, current_tag, mod)
				unit = unit.__cfg;
				wesnoth.set_variable(unit_path, unit)
			elseif current_tag == "effect" then
				local mod = current_table[2]
				local apply_to = mod.apply_to
				if wesnoth.effects[apply_to] then
					local unit = wesnoth.get_variable(unit_path)
					unit = wesnoth.create_unit(unit)
					wesnoth.effects[apply_to](unit, mod)
					unit = unit.__cfg;
					wesnoth.set_variable(unit_path, unit)
				else
					helper.wml_error("[modify_unit] had invalid [effect]apply_to value")
				end
			else
				if replace_mode then
					wesnoth.set_variable(string.format("%s.%s", unit_path, current_tag), {})
				end
				local tag_index = children_handled[current_tag] or 0
				handle_child(current_table[2], string.format("%s.%s[%u]",
					unit_path, current_tag, tag_index))
				children_handled[current_tag] = tag_index + 1
			end
		end

		if cfg.type then
			if cfg.type ~= "" then wesnoth.set_variable(unit_path .. ".advances_to", cfg.type) end
			wesnoth.set_variable(unit_path .. ".experience", wesnoth.get_variable(unit_path .. ".max_experience"))
		end
		wml_actions.kill({ id = this_unit.id, animate = false })
		wml_actions.unstore_unit { variable = unit_path }
	end

	wml_actions.store_unit { {"filter", filter}, variable = unit_variable }
	local max_index = wesnoth.get_variable(unit_variable .. ".length") - 1

	local this_unit = utils.start_var_scope("this_unit")
	for current_unit = 0, max_index do
		handle_unit(current_unit)
	end
	utils.end_var_scope("this_unit", this_unit)

	wesnoth.set_variable(unit_variable)
end