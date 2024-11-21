
function wesnoth.wml_actions.set_variable(cfg, variables)
	local name = cfg.name or wml.error "trying to set a variable with an empty name"
	if variables == nil then variables = wml.variables end

	if cfg.value ~= nil then -- check for nil because user may try to set a variable as false
		variables[name] = cfg.value
	end

	if cfg.literal ~= nil then
		variables[name] = wml.shallow_literal(cfg).literal
	end

	if cfg.to_variable then
		variables[name] = variables[cfg.to_variable]
	end

	if cfg.suffix then
		variables[name] = (variables[name] or '') .. (cfg.suffix or '')
	end

	if cfg.prefix then
		variables[name] = (cfg.prefix or '') .. (variables[name] or '')
	end

	if cfg.add then
		variables[name] = (tonumber(variables[name]) or 0) + (tonumber(cfg.add) or 0)
	end

	if cfg.sub then
		variables[name] = (tonumber(variables[name]) or 0) - (tonumber(cfg.sub) or 0)
	end

	if cfg.multiply then
		variables[name] = (tonumber(variables[name]) or 0) * (tonumber(cfg.multiply) or 0)
	end

	if cfg.divide then
		local divide = tonumber(cfg.divide) or 0
		if divide == 0 then wml.error("division by zero on variable " .. name) end
		variables[name] = (tonumber(variables[name]) or 0) / divide
	end

	if cfg.modulo then
		local modulo = tonumber(cfg.modulo) or 0
		if modulo == 0 then wml.error("division by zero on variable " .. name) end
		variables[name] = (tonumber(variables[name]) or 0) % modulo
	end

	if cfg.abs then
		variables[name] = math.abs(tonumber(variables[name]) or 0)
	end

	if cfg.reverse then
		if type(variables[name]) == 'string' then
			variables[name] = string.reverse(variables[name])
		elseif type(variables[name]) == 'number' or getmetatable(variables[name]) == 'translatable string' then
			variables[name] = string.reverse(tostring(variables[name]))
		else
			wml.error(string.format('Cannot reverse value %s', tostring(variables[name])))
		end
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

		local radicand = tonumber(variables[name]) or 0
		if radicand < 0 and root % 2 == 0 then
			if root == 2 then
				wml.error("square root of negative number on variable " .. name)
			else
				wml.error(string.format("%dth root of negative number on variable %s", root, name))
			end
		end

		variables[name] = root_fcn(radicand)
	end

	if cfg.power then
		variables[name] = (tonumber(variables[name]) or 0) ^ (tonumber(cfg.power) or 0)
	end

	if cfg.round then
		local var = tonumber(variables[name] or 0)
		local round_val = cfg.round
		if round_val == "ceil" then
			variables[name] = math.ceil(var)
		elseif round_val == "floor" then
			variables[name] = math.floor(var)
		elseif round_val == "trunc" then
			-- Storing to a variable first because modf returns two values,
			-- and I'm not sure if set_variable will complain about the extra parameter
			local new_val = math.modf(var)
			variables[name] = new_val
		else
			local decimals = math.modf(tonumber(round_val) or 0)
			local value = var * (10 ^ decimals)
			value = mathx.round(value)
			value = value * (10 ^ -decimals)
			variables[name] = value
		end
	end

	-- unlike the other math operations, ipart and fpart do not act on
	-- the value already contained in the variable
	-- but on the value assigned to the respective key
	if cfg.ipart then
		local ivalue = math.modf(tonumber(cfg.ipart) or 0)
		variables[name] = ivalue
	end

	if cfg.fpart then
		local ivalue, fvalue = math.modf(tonumber(cfg.fpart) or 0)
		variables[name] = fvalue
	end

	-- similarly, min and max operate on the list assigned to the variable
	-- and do not consider value already contained in the variable
	if cfg.min then
		local values = cfg.min:split()
		for i = 1, #values do
			values[i] = tonumber(values[i])
		end
		variables[name] = math.min(table.unpack(values))
	end

	if cfg.max then
		local values = cfg.max:split()
		for i = 1, #values do
			values[i] = tonumber(values[i])
		end
		variables[name] = math.max(table.unpack(values))
	end

	if cfg.string_length ~= nil then
		variables[name] = string.len(tostring(cfg.string_length))
	end

	if cfg.time then
		if cfg.time == "stamp" then
			variables[name] = wesnoth.ms_since_init()
		end
	end

	if cfg.rand then
		variables[name] = mathx.random_choice(tostring(cfg.rand))
	end

	if cfg.formula then
		local fcn = wesnoth.compile_formula(cfg.formula)
		variables[name] = fcn({value = variables[name]})
	end

	local join_child = wml.get_child(cfg, "join")
	if join_child then
		local array_name = join_child.variable or wml.error "missing variable= attribute in [join]"
		local separator = join_child.separator
		local key_name = join_child.key or "value"
		local remove_empty = join_child.remove_empty

		local string_to_join = ''

		for i, element in ipairs(wml.array_variables[array_name]) do
			if element[key_name] ~= nil or (not remove_empty) then
				if #string_to_join > 0 then
					string_to_join = string_to_join .. separator
				end
				local elem = element[key_name]
				if type(elem) == 'boolean' then
					-- Use yes/no instead of true/false for booleans
					elem = elem and 'yes' or 'no'
				elseif getmetatable(elem) ~= 'translatable string' then
					-- Not entirely sure if this branch is necessary, since it probably only triggers for numbers
					-- It certainly can't hurt, though.
					elem = tostring(elem)
				end
				string_to_join = string_to_join .. elem
			end
		end

		variables[name] = string_to_join
	end
end
