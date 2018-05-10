local helper = wesnoth.require "helper"

function wesnoth.wml_actions.set_variable(cfg)
	local name = cfg.name or helper.wml_error "trying to set a variable with an empty name"

	if cfg.value ~= nil then -- check for nil because user may try to set a variable as false
		wesnoth.set_variable(name, cfg.value)
	end

	if cfg.literal ~= nil then
		wesnoth.set_variable(name, wml.shallow_literal(cfg).literal)
	end

	if cfg.to_variable then
		wesnoth.set_variable(name, wml.variables[cfg.to_variable])
	end

	if cfg.suffix then
		wesnoth.set_variable(name, (wml.variables[name] or '') .. (cfg.suffix or ''))
	end

	if cfg.prefix then
		wesnoth.set_variable(name, (cfg.prefix or '') .. (wml.variables[name] or ''))
	end

	if cfg.add then
		wesnoth.set_variable(name, (tonumber(wml.variables[name]) or 0) + (tonumber(cfg.add) or 0))
	end

	if cfg.sub then
		wesnoth.set_variable(name, (tonumber(wml.variables[name]) or 0) - (tonumber(cfg.sub) or 0))
	end

	if cfg.multiply then
		wesnoth.set_variable(name, (tonumber(wml.variables[name]) or 0) * (tonumber(cfg.multiply) or 0))
	end

	if cfg.divide then
		local divide = tonumber(cfg.divide) or 0
		if divide == 0 then helper.wml_error("division by zero on variable " .. name) end
		wesnoth.set_variable(name, (tonumber(wml.variables[name]) or 0) / divide)
	end

	if cfg.modulo then
		local modulo = tonumber(cfg.modulo) or 0
		if modulo == 0 then helper.wml_error("division by zero on variable " .. name) end
		wesnoth.set_variable(name, (tonumber(wml.variables[name]) or 0) % modulo)
	end

	if cfg.abs then
		wesnoth.set_variable(name, math.abs(tonumber(wml.variables[name]) or 0))
	end

	if cfg.root then
		local root = tonumber(cfg.root)
		local root_fcn
		if cfg.root == "square" then
			root = 2
			root_fcn = math.sqrt
		else
			if cfg.root == "cube" then
				root = 3
			end
			root_fcn = function(n) return n ^ (1 / root) end
		end

		local radicand = tonumber(wml.variables[name]) or 0
		if radicand < 0 and root % 2 == 0 then
			if root == 2 then
				helper.wml_error("square root of negative number on variable " .. name)
			else
				helper.wml_error(string.format("%dth root of negative number on variable %s", root, name))
			end
		end

		wesnoth.set_variable(name, root_fcn(radicand))
	end

	if cfg.power then
		wesnoth.set_variable(name, (tonumber(wml.variables[name]) or 0) ^ (tonumber(cfg.power) or 0))
	end

	if cfg.round then
		local var = tonumber(wml.variables[name] or 0)
		local round_val = cfg.round
		if round_val == "ceil" then
			wesnoth.set_variable(name, math.ceil(var))
		elseif round_val == "floor" then
			wesnoth.set_variable(name, math.floor(var))
		elseif round_val == "trunc" then
			-- Storing to a variable first because modf returns two values,
			-- and I'm not sure if set_variable will complain about the extra parameter
			local new_val = math.modf(var)
			wesnoth.set_variable(name, new_val)
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
		wesnoth.set_variable(name, fcn(wml.variables[name]))
	end

	local join_child = wml.get_child(cfg, "join")
	if join_child then
		local array_name = join_child.variable or helper.wml_error "missing variable= attribute in [join]"
		local separator = join_child.separator
		local key_name = join_child.key or "value"
		local remove_empty = join_child.remove_empty

		local string_to_join = ''

		for i, element in ipairs(wml.array_access.get(array_name)) do
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
