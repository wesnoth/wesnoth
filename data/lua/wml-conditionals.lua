function wesnoth.wml_conditionals.proceed_to_next_scenario(cfg)
	local endlevel_data = wesnoth.get_end_level_data()

	if not endlevel_data then
		return false
	else
		return endlevel_data.proceed_to_next_level
	end
end

function wesnoth.wml_conditionals.lua(cfg)
	cfg = wml.shallow_literal(cfg)
	local bytecode, message = load(cfg.code or "")

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
		local result = wesnoth.eval_formula(cfg.formula, {value = value})
		-- WFL considers 0 as false; Lua doesn't
		if result == 0 then return false end
		return result
	else
		return old_variable(cfg)
	end
end
