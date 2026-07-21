local utils = wesnoth.require "wml-utils"
local wml_actions = wesnoth.wml_actions

-- The [modify_unit] implementation.
-- Implementation detail: every affected unit is passed through [unstore_unit], so gets
-- that tag's implementation's handling of special cases such as x,y=recall,recall.

function wml_actions.modify_unit(cfg)
	local unit_variable = "LUA_modify_unit"

	local replace_mode = cfg.mode == "replace"

	local function handle_attributes(child_cfg, unit_path, toplevel)
		for current_key, current_value in pairs(wml.shallow_parsed(child_cfg)) do
			if type(current_value) ~= "table" and (not toplevel or (current_key ~= "type" and current_key ~= "mode")) then
				wml.variables[string.format("%s.%s", unit_path, current_key)] = current_value
			end
		end
	end

	local function handle_child(child_cfg, unit_path)
		local children_handled = {}
		child_cfg = wml.shallow_parsed(child_cfg)
		handle_attributes(child_cfg, unit_path)

		for current_index, current_table in ipairs(child_cfg) do
			local current_tag = current_table[1]
			local tag_index = children_handled[current_tag] or 0
			handle_child(current_table[2], string.format("%s.%s[%u]",
				unit_path, current_tag, tag_index))
			children_handled[current_tag] = tag_index + 1
		end
	end

	local filter = wml.get_child(cfg, "filter") or wml.error "[modify_unit] missing required [filter] tag"
	local function handle_unit(unit_num)
		local children_handled = {}
		local unit_path = string.format("%s[%u]", unit_variable, unit_num)
		local this_unit = wml.variables[unit_path]
		wml.variables["this_unit"] = this_unit
		handle_attributes(cfg, unit_path, true)

		for current_index, current_table in ipairs(wml.shallow_parsed(cfg)) do
			local current_tag = current_table[1]
			if current_tag == "filter" then
				goto skip
			elseif current_tag == "object" or current_tag == "trait" or current_tag == "advancement" then
				local mod = current_table[2]
				if mod.delayed_variable_substitution then
					mod = wml.literal(mod)
				else
					mod = wml.parsed(mod)
				end
				local unit = wml.variables[unit_path]
				unit = wesnoth.units.create(unit)
				unit:add_modification(current_tag, mod)
				unit = unit.__cfg;
				wml.variables[unit_path] = unit
			elseif current_tag == "effect" then
				local mod = current_table[2]
				local apply_to = mod.apply_to
				if wesnoth.effects[apply_to] then
					local unit = wml.variables[unit_path]
					unit = wesnoth.units.create(unit)
					wesnoth.effects[apply_to](unit, mod)
					unit = unit.__cfg;
					wml.variables[unit_path] = unit
				else
					wml.error("[modify_unit] had invalid [effect]apply_to value")
				end
			elseif current_tag == "set_variable" then
				local unit = wesnoth.units.create(wml.variables[unit_path])
				wesnoth.wml_actions.set_variable(current_table[2], unit.variables)
				wml.variables[unit_path] = unit.__cfg
			elseif current_tag == "set_variables" then
				local unit = wesnoth.units.create(wml.variables[unit_path])
				wesnoth.wml_actions.set_variables(current_table[2], unit.variables)
				wml.variables[unit_path] = unit.__cfg
			elseif current_tag == "clear_variable" then
				local unit = wesnoth.units.create(wml.variables[unit_path])
				wesnoth.wml_actions.clear_variable(current_table[2], unit.variables)
				wml.variables[unit_path] = unit.__cfg
			else
				if replace_mode then
					wml.variables[string.format("%s.%s", unit_path, current_tag)] = {}
				end
				local tag_index = children_handled[current_tag] or 0
				handle_child(current_table[2], string.format("%s.%s[%u]",
					unit_path, current_tag, tag_index))
				children_handled[current_tag] = tag_index + 1
			end
			::skip::
		end

		if cfg.type then
			if cfg.type ~= "" then wml.variables[unit_path .. ".advances_to"] = cfg.type end
			wml.variables[unit_path .. ".experience"] = wml.variables[unit_path .. ".max_experience"]
		end
		wml_actions.kill({ id = this_unit.id, animate = false })
		wml_actions.unstore_unit { variable = unit_path }
	end

	wml_actions.store_unit { wml.tag.filter(filter), variable = unit_variable }
	local max_index = wml.variables[unit_variable .. ".length"] - 1

	local this_unit <close> = utils.scoped_var("this_unit")
	for current_unit = 0, max_index do
		handle_unit(current_unit)
	end

	wml.variables[unit_variable] = nil
end
