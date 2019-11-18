local helper = wesnoth.require "helper"
local utils = wesnoth.require "wml-utils"
local wml_actions = wesnoth.wml_actions

-- The [modify_unit] implementation
--  modify_unit has basicially two implementations, the optimized implementation
--  in the first part of this file and the fallback (old) implementation in the
--  second part of this file

local function split_to_array(str)
	-- Split string @str into a table using the delimiter @sep (default: ',')

	local fields = {}
	for c in utils.split(str) do
		fields[#fields+1] = c
	end
	return fields
end

local function make_set(t)
	local res = {}
	for i, v in ipairs(t) do
		res[v] = true
	end
	return res
end

local known_attributes = make_set {
	"mode",
	"x",
	"y",
	"ai_special",
	"goto_x",
	"goto_y",
	"extra_recruit",
	"side",
	"name",
	"role",
	"facing",
	"attacks_left",
	"hitpoints",
	"max_hitpoints",
	"moves",
	"max_moves",
	"experience",
	"max_experience",
	"resting",
	"canrecruit",
	"type",
	"variation",
	"ellipse",
	"halo",
	"recall_cost",
	"description",
	"hidden",
	"unrenamable",
	"profile",
	"zoc",
	"usage",
	"upkeep",
}

local known_tags = make_set {
	"object",
	"advancement",
	"trait",
	"effect",
	"filter",
	"status",
	"set_variable",
	-- todo: "set_variables",
	"clear_variable",
	"filter_recall",
	"variables",
}

local function is_simple(cfg)
	for k, v in pairs(wml.shallow_literal(cfg)) do
		if type(k) == "string" then
			if not known_attributes[k] then
				return false
			end
		else
			if not known_tags[v[1]] then
				return false
			end
		end
	end
	return true
end

local function simple_modify_unit(cfg)
	local filter = wml.get_child(cfg, "filter") or helper.wml_error "[modify_unit] missing required [filter] tag"
	-- todo: investigate the following attrtibutes:
	--       id, alpha, flying, overlays
	local simple_attributes = {
		"ellipse",
		"halo",
		"recall_cost",
		"description",
		"hidden",
		"side",
		"name",
		"role",
		"facing",
		"attacks_left",
		"hitpoints",
		"max_hitpoints",
		"moves",
		"max_moves",
		"experience",
		"max_experience",
		"resting",
		"canrecruit",
		"zoc",
		"usage",
		"upkeep",
	}

	local function handle_unit(u)
		---------- ATTRIBUTES THAT NEED SPECIAL HANDLING ----------
		if cfg.x or cfg.y then
			u.loc = { cfg.x or u.x , cfg.y or u.y }
		end
		if cfg.goto_x or cfg.goto_y then
			u["goto"] = { cfg.goto_x or u["goto"][1] , cfg.goto_y or u["goto"][2] }
		end
		if cfg.extra_recruit then
			u.extra_recruit = split_to_array(cfg.extra_recruit)
		end
		if cfg.ai_special == "guardian" then
			u.status.guardian = true
		end
		if cfg.profile ~= nil then
			u.portrait = profile
		end
		if cfg.unrenamable ~= nil then
			u.renamable = not cfg.unrenamable
		end
		---------- SIMPLE ATTRIBUTES  ----------
		for i, name in ipairs(simple_attributes) do
			if cfg[name] ~= nil then
				u[name] = cfg[name]
			end
		end

		---------- TAGS ----------
		local found_recall, found_variables = false, false
		for i, t in ipairs(wml.shallow_parsed(cfg)) do
			local tagname, tagcontent = t[1], t[2]
			if tagname == "object" or tagname == "trait" or tagname == "advancement" then
				if tagcontent.delayed_variable_substitution then
					tagcontent = wml.literal(tagcontent)
				else
					tagcontent = wml.parsed(tagcontent)
				end
				u:add_modification(tagname, tagcontent);
			elseif tagname == "effect" then
				local apply_to = tagcontent.apply_to
				if wesnoth.effects[apply_to] then
					wesnoth.effects[apply_to](u, tagcontent)
				else
					helper.wml_error("[modify_unit] had invalid [effect]apply_to value")
				end
			elseif tagname == "status" then
				for i, v in pairs(tagcontent) do
					u.status[i] = v
				end
			elseif not found_recall and tagname == "filter_recall" then
				u.recall_filter = wml.merge(u.recall_filter, tagcontent, cfg.mode or "merge")
				found_recall = true -- Ignore all but the first
			elseif not found_variables and tagname == "variables" then
				u.variables.__cfg = wml.merge(u.variables.__cfg, tagcontent, cfg.mode or "merge")
				found_variables = true -- Ignore all but the first
			elseif tagname == "set_variable" then
				wesnoth.wml_actions.set_variable(tagcontent, u.variables)
			elseif tagname == "clear_variable" then
				wesnoth.wml_actions.clear_variable(tagcontent, u.variables)
			end
		end

		-- handle 'type' and 'variation' last.
		if cfg.type == "" then
			u.experience = u.max_experience
		elseif cfg.type or cfg.variation then
			u.experience = 0
			u:transform(cfg.type or u.type, cfg.variation)
		end

		-- always do an advancement here (not only when experience/max_experience/type was modified)
		-- for compatibility with old code.
		-- Skip for recall list units
		if u.valid == 'map' then
			u:advance()
		end
	end

	local this_unit = utils.start_var_scope("this_unit")
	for i, u in ipairs(wesnoth.units.find(filter)) do
		wml.variables["this_unit"] = u.__cfg
		handle_unit(u)
	end
	utils.end_var_scope("this_unit", this_unit)
end

function wml_actions.modify_unit(cfg)
	if is_simple(cfg) then
		simple_modify_unit(cfg)
		return
	end
	local unit_variable = "LUA_modify_unit"

	local replace_mode = cfg.mode == "replace"

	local function handle_attributes(cfg, unit_path, toplevel)
		for current_key, current_value in pairs(wml.shallow_parsed(cfg)) do
			if type(current_value) ~= "table" and (not toplevel or (current_key ~= "type" and current_key ~= "mode")) then
				wml.variables[string.format("%s.%s", unit_path, current_key)] = current_value
			end
		end
	end

	local function handle_child(cfg, unit_path)
		local children_handled = {}
		local cfg = wml.shallow_parsed(cfg)
		handle_attributes(cfg, unit_path)

		for current_index, current_table in ipairs(cfg) do
			local current_tag = current_table[1]
			local tag_index = children_handled[current_tag] or 0
			handle_child(current_table[2], string.format("%s.%s[%u]",
				unit_path, current_tag, tag_index))
			children_handled[current_tag] = tag_index + 1
		end
	end

	local filter = wml.get_child(cfg, "filter") or helper.wml_error "[modify_unit] missing required [filter] tag"
	local function handle_unit(unit_num)
		local children_handled = {}
		local unit_path = string.format("%s[%u]", unit_variable, unit_num)
		local this_unit = wml.variables[unit_path]
		wml.variables["this_unit"] = this_unit
		handle_attributes(cfg, unit_path, true)

		for current_index, current_table in ipairs(wml.shallow_parsed(cfg)) do
			local current_tag = current_table[1]
			if current_tag == "filter" then
				-- nothing
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
					helper.wml_error("[modify_unit] had invalid [effect]apply_to value")
				end
			elseif current_tag == "set_variable" then
				local unit = wesnoth.units.create(wml.variables[unit_path])
				wesnoth.wml_actions.set_variable(current_table[2], unit.variables)
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
		end

		if cfg.type then
			if cfg.type ~= "" then wml.variables[unit_path .. ".advances_to"] = cfg.type end
			wml.variables[unit_path .. ".experience"] = wml.variables[unit_path .. ".max_experience"]
		end
		wml_actions.kill({ id = this_unit.id, animate = false })
		wml_actions.unstore_unit { variable = unit_path }
	end

	wml_actions.store_unit { {"filter", filter}, variable = unit_variable }
	local max_index = wml.variables[unit_variable .. ".length"] - 1

	local this_unit = utils.start_var_scope("this_unit")
	for current_unit = 0, max_index do
		handle_unit(current_unit)
	end
	utils.end_var_scope("this_unit", this_unit)

	wml.variables[unit_variable] = nil
end
