function wesnoth.wml_conditionals.proceed_to_next_scenario(cfg)
	local endlevel_data = wesnoth.scenario.end_level_data

	if not endlevel_data then
		return false
	else
		return endlevel_data.proceed_to_next_level
	end
end

function wesnoth.wml_conditionals.lua(cfg)
	cfg = wml.shallow_literal(cfg)
	local bytecode, message = load(cfg.code or "", cfg.name or nil)

	if not bytecode then
		error("~lua:" .. message, 0)
	else
		return bytecode(wml.get_child(cfg, "args"))
	end
end

-- Add formula= to [variable]
-- Doesn't work for array variables though
local old_variable = wesnoth.wml_conditionals.variable
function wesnoth.wml_conditionals.variable(cfg)
	if cfg.formula then
		local value = wml.variables[cfg.name]
		if cfg.as_type == 'unit' then
			value = wesnoth.units.create(value)
		elseif cfg.as_type == 'weapon' then
			value = wesnoth.units.create_weapon(value)
		end
		local result = wesnoth.eval_formula(cfg.formula, {value = value})
		-- WFL considers 0 as false; Lua doesn't
		if result == 0 then return false end
		return result
	elseif cfg.blank then
		if type(cfg.blank) ~= 'boolean' then wml.error('[variables]blank= must be a boolean') end
		local value = wml.variables[cfg.name]
		return cfg.blank == (value == nil)
	else
		return old_variable(cfg)
	end
end

-- Similar to [variable], but works on container variables
function wesnoth.wml_conditionals.variables(cfg)
	-- Step 1: load the variable we're comparing
	local value, is_single
	if cfg.name:match('%[%d+%]$') then
		value = {wml.tag.value(wml.variables[cfg.name])}
		is_single = true
	else
		value = wml.array_access.get(cfg.name)
		is_single = false
		-- This turns the array into a valid WML table
		for i = 1,#value do
			value[i] = wml.tag.value(value[i])
		end
	end
	if not wml.valid(value) then return false end
	for _,t in ipairs(cfg) do
		if t.tag == 'equals' then
			-- The entire variable contents matches exactly
			local other
			if is_single then
				other = {wml.tag.value(t.contents)}
			else
				other = wml.child_array(cfg, 'value')
				-- This turns the array into a valid WML table
				for i = 1,#other do
					other[i] = wml.tag.value(other[i])
				end
			end
			if #value ~= #other then return false end
			if not wml.equal(value, other) then return false end
		elseif t.tag == 'contains' then
			-- There exists an element of the array that matches exactly
			local found = false
			for i = 1, #value do
				if wml.equal(value[i].contents, t.contents) then
					found = true
					break
				end
			end
			if not found then return false end
		elseif t.tag == 'filter_wml' then
			-- There exists an element that matches the filter
			local found = false
			for i = 1, #value do
				if wml.matches_filter(value[i].contents, t.contents) then
					found = true
					break
				end
			end
			if not found then return false end
		end
	end
	return true
end

function wesnoth.wml_conditionals.has_achievement(cfg)
	return wesnoth.achievements.has(cfg.content_for, cfg.id);
end

function wesnoth.wml_conditionals.has_sub_achievement(cfg)
	return wesnoth.achievements.has_sub_achievement(cfg.content_for, cfg.id, cfg.sub_id);
end

function wesnoth.wml_conditionals.have_side(cfg)
	local sides = wesnoth.sides.find(cfg)
	if cfg.count then
		if #sides == cfg.count then return true else return false end
	else
		if sides[1] then return true else return false end
	end
end
