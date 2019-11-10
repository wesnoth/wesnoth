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
}

local known_tags = make_set {
	"object",
	"advancement",
	"trait",
	"filter",
	"status",
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

-- gets map and recalllist units.
local function get_all_units(filter)
	local res = wesnoth.get_units(filter)
	if (not filter.x or filter.x == "recall") and (not filter.y or filter.y == "recall") then
		--append recall units to result.
		for i, u in ipairs(wesnoth.get_recall_units(filter)) do
			res[#res + 1] = u
		end
	end
	return res
end

local function simple_modify_unit(cfg)
	local filter = wml.get_child(cfg, "filter") or helper.wml_error "[modify_unit] missing required [filter] tag"
	-- todo: investigate the follwoing attrtibutes:
	--       id, ellipse, recall_cost, alpha, flying,
	--       hidden, halo, description, overlays, unrenamable
	-- and tags: [status] [variables]
	local simple_attributes = {
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
		"canrecruit"
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
		---------- SIMPLE ATTRIBUTES  ----------
		for i, name in ipairs(simple_attributes) do
			if cfg[name] ~= nil then
				u[name] = cfg[name]
			end
		end

		---------- TAGS ----------
		for i, t in ipairs(wml.shallow_parsed(cfg)) do
			local tagname, tagcontent = t[1], t[2]
			if tagname == "object" or tagname == "trait" or tagname == "advancement" then
				if tagcontent.delayed_variable_substitution then
					tagcontent = wml.literal(tagcontent)
				else
					tagcontent = wml.parsed(tagcontent)
				end
				u:add_modification(tagname, tagcontent);
			end
			if tagname == "status" then
				for i, v in pairs(tagcontent) do
					u.status[i] = v
				end
			end
		end

		-- handle 'type' last.
		if cfg.type == "" then
			u.experience = u.max_experience
		elseif cfg.type then
			u.experience = 0
			u:transform(cfg.type)
		end

		-- always do an advancement here (not only when experience/max_experience/type was modified)
		-- for compatability with old code.
		u:advance()
	end

	local this_unit = utils.start_var_scope("this_unit")
	for i, u in ipairs(get_all_units(filter)) do
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
				unit = wesnoth.create_unit(unit)
				wesnoth.add_modification(unit, current_tag, mod)
				unit = unit.__cfg;
				wml.variables[unit_path] = unit
			elseif current_tag == "effect" then
				local mod = current_table[2]
				local apply_to = mod.apply_to
				if wesnoth.effects[apply_to] then
					local unit = wml.variables[unit_path]
					unit = wesnoth.create_unit(unit)
					wesnoth.effects[apply_to](unit, mod)
					unit = unit.__cfg;
					wml.variables[unit_path] = unit
				else
					helper.wml_error("[modify_unit] had invalid [effect]apply_to value")
				end
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
