local helper = wesnoth.require "helper"

function wesnoth.wml_actions.set_variable(cfg)
	local name = cfg.name or helper.wml_error "trying to set a variable with an empty name"

	if cfg.value ~= nil then -- check for nil because user may try to set a variable as false
		wml.variables[name] = cfg.value
	end

	if cfg.literal ~= nil then
		wml.variables[name] = wml.shallow_literal(cfg).literal
	end

	if cfg.to_variable then
		wml.variables[name] = wml.variables[cfg.to_variable]
	end

	if cfg.suffix then
		wml.variables[name] = (wml.variables[name] or '') .. (cfg.suffix or '')
	end

	if cfg.prefix then
		wml.variables[name] = (cfg.prefix or '') .. (wml.variables[name] or '')
	end

	if cfg.add then
		wml.variables[name] = (tonumber(wml.variables[name]) or 0) + (tonumber(cfg.add) or 0)
	end

	if cfg.sub then
		wml.variables[name] = (tonumber(wml.variables[name]) or 0) - (tonumber(cfg.sub) or 0)
	end

	if cfg.multiply then
		wml.variables[name] = (tonumber(wml.variables[name]) or 0) * (tonumber(cfg.multiply) or 0)
	end

	if cfg.divide then
		local divide = tonumber(cfg.divide) or 0
		if divide == 0 then helper.wml_error("division by zero on variable " .. name) end
		wml.variables[name] = (tonumber(wml.variables[name]) or 0) / divide
	end

	if cfg.modulo then
		local modulo = tonumber(cfg.modulo) or 0
		if modulo == 0 then helper.wml_error("division by zero on variable " .. name) end
		wml.variables[name] = (tonumber(wml.variables[name]) or 0) % modulo
	end

	if cfg.abs then
		wml.variables[name] = math.abs(tonumber(wml.variables[name]) or 0)
	end

	if cfg.root then
		if cfg.root == "square" then
			local radicand = tonumber(wml.variables[name]) or 0
			if radicand < 0 then helper.wml_error("square root of negative number on variable " .. name) end
			wml.variables[name] = math.sqrt(radicand)
		end
	end

	if cfg.power then
		wml.variables[name] = (tonumber(wml.variables[name]) or 0) ^ (tonumber(cfg.power) or 0)
	end

	if cfg.round then
		local var = tonumber(wml.variables[name] or 0)
		local round_val = cfg.round
		if round_val == "ceil" then
			wml.variables[name] = math.ceil(var)
		elseif round_val == "floor" then
			wml.variables[name] = math.floor(var)
		else
			local decimals = math.modf(tonumber(round_val) or 0)
			local value = var * (10 ^ decimals)
			value = helper.round(value)
			value = value * (10 ^ -decimals)
			wml.variables[name] = value
		end
	end

	-- unlike the other math operations, ipart and fpart do not act on
	-- the value already contained in the variable
	-- but on the value assigned to the respective key
	if cfg.ipart then
		local ivalue = math.modf(tonumber(cfg.ipart) or 0)
		wml.variables[name] = ivalue
	end

	if cfg.fpart then
		local ivalue, fvalue = math.modf(tonumber(cfg.fpart) or 0)
		wml.variables[name] = fvalue
	end

	if cfg.string_length ~= nil then
		wml.variables[name] = string.len(tostring(cfg.string_length))
	end

	if cfg.time then
		if cfg.time == "stamp" then
			wml.variables[name] = wesnoth.get_time_stamp()
		end
	end

	if cfg.rand then
		wml.variables[name] = helper.rand(tostring(cfg.rand))
	end

	if cfg.formula then
		local fcn = wesnoth.compile_formula(cfg.formula)
		wml.variables[name] = fcn(wml.variables[name])
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

		wml.variables[name] = string_to_join
	end
end
