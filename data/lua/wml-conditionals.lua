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
	else
		return old_variable(cfg)
	end
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
