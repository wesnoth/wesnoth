local helper = wesnoth.require "helper"

function wesnoth.wml_actions.set_variable(cfg)
	local name = cfg.name or helper.wml_error "trying to set a variable with an empty name"

	if cfg.value ~= nil then -- check for nil because user may try to set a variable as false
		wesnoth.set_variable(name, cfg.value)
	end

	if cfg.literal ~= nil then
		wesnoth.set_variable(name, helper.shallow_literal(cfg).literal)
	end

	if cfg.to_variable then
		wesnoth.set_variable(name, wesnoth.get_variable(cfg.to_variable))
	end

	if cfg.add then
		wesnoth.set_variable(name, (tonumber(wesnoth.get_variable(name)) or 0) + (tonumber(cfg.add) or 0))
	end

	if cfg.sub then
		wesnoth.set_variable(name, (tonumber(wesnoth.get_variable(name)) or 0) - (tonumber(cfg.sub) or 0))
	end

	if cfg.multiply then
		wesnoth.set_variable(name, (tonumber(wesnoth.get_variable(name)) or 0) * (tonumber(cfg.multiply) or 0))
	end

	if cfg.divide then
		local divide = tonumber(cfg.divide) or 0
		if divide == 0 then helper.wml_error("division by zero on variable " .. name) end
		wesnoth.set_variable(name, (tonumber(wesnoth.get_variable(name)) or 0) / divide)
	end

	if cfg.modulo then
		local modulo = tonumber(cfg.modulo) or 0
		if modulo == 0 then helper.wml_error("division by zero on variable " .. name) end
		wesnoth.set_variable(name, (tonumber(wesnoth.get_variable(name)) or 0) % modulo)
	end

	if cfg.abs then
		wesnoth.set_variable(name, math.abs(tonumber(wesnoth.get_variable(name)) or 0))
	end

	if cfg.root then
		if cfg.root == "square" then
			local radicand = tonumber(wesnoth.get_variable(name)) or 0
			if radicand < 0 then helper.wml_error("square root of negative number on variable " .. name) end
			wesnoth.set_variable(name, math.sqrt(radicand))
		end
	end

	if cfg.power then
		wesnoth.set_variable(name, (tonumber(wesnoth.get_variable(name)) or 0) ^ (tonumber(cfg.power) or 0))
	end

	if cfg.round then
		local var = tonumber(wesnoth.get_variable(name) or 0)
		local round_val = cfg.round
		if round_val == "ceil" then
			wesnoth.set_variable(name, math.ceil(var))
		elseif round_val == "floor" then
			wesnoth.set_variable(name, math.floor(var))
		else
			local decimals = math.modf(tonumber(round_val) or 0)
			local value = var * (10 ^ decimals)
			value = helper.round(value)
			value = value * (10 ^ -decimals)
			wesnoth.set_variable(name, value)
		end
	end

	-- unlike the other math operations, ipart and fpart do not act on
	-- the value already contained in the variable
	-- but on the value assigned to the respective key
	if cfg.ipart then
		local ivalue = math.modf(tonumber(cfg.ipart) or 0)
		wesnoth.set_variable(name, ivalue)
	end

	if cfg.fpart then
		local ivalue, fvalue = math.modf(tonumber(cfg.fpart) or 0)
		wesnoth.set_variable(name, fvalue)
	end

	if cfg.string_length ~= nil then
		wesnoth.set_variable(name, string.len(tostring(cfg.string_length)))
	end

	if cfg.time then
		if cfg.time == "stamp" then
			wesnoth.set_variable(name, wesnoth.get_time_stamp())
		end
	end

	if cfg.rand then
		wesnoth.set_variable(name, helper.rand(tostring(cfg.rand)))
	end

	if cfg.formula then
		local fcn = wesnoth.compile_formula(cfg.formula)
		wesnoth.set_variable(name, fcn(wesnoth.get_variable(name)))
	end

	local join_child = helper.get_child(cfg, "join")
	if join_child then
		local array_name = join_child.variable or helper.wml_error "missing variable= attribute in [join]"
		local separator = join_child.separator
		local key_name = join_child.key or "value"
		local remove_empty = join_child.remove_empty

		local string_to_join = ''

		for i, element in ipairs(helper.get_variable_array(array_name)) do
			if element[key_name] ~= nil or (not remove_empty) then
				if #string_to_join > 0 then
					string_to_join = string_to_join .. separator
				end
				string_to_join = string_to_join .. element[key_name]
			end
		end

		wesnoth.set_variable(name, string_to_join)
	end
end
