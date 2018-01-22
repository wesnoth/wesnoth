local helper = wesnoth.require "helper"

function wesnoth.wml_conditionals.proceed_to_next_scenario(cfg)
	local endlevel_data = wesnoth.get_end_level_data()

	if not endlevel_data then
		return false
	else
		return endlevel_data.proceed_to_next_level
	end
end

function wesnoth.wml_conditionals.lua(cfg)
	cfg = helper.shallow_literal(cfg)
	local bytecode, message = load(cfg.code or "")

	if not bytecode then
		error("~lua:" .. message, 0)
	else
		return bytecode(helper.get_child(cfg, "args"))
	end
end
