function append_array_to_wml(wml, key, array)
	for i = 1, #array do
		table.insert(wml, wml.tag[key](array[i]))
	end
	return wml
end

function wesnoth.wml_actions.set_variables(cfg, variables)
	local name = cfg.name or wml.error "trying to set a variable with an empty name"
	if not wml.valid_var(name) then
		wml.error(stringx.vformat("Cannot do [set_variables] with invalid destination variable: $varname with $settings", {
			varname = name,
			settings = wml.tostring(wml.literal(cfg))
		}))
	end

	local data = {}
	if cfg.to_variable then
		if not wml.valid_var(cfg.to_variable) then
			wml.error(stringx.vformat("Cannot do [set_variables] with invalid to_variable variable: $varname with $settings", {
				varname = cfg.to_variable,
				settings = wml.tostring(wml.literal(cfg))
			}))
		end
		-- cfg.to_variable is allowed to refer to container variable (for example array element) instead of array, but wml.array_access.get does not support that
		local variables_proxy = variables or wml.variables
		if variables_proxy[cfg.to_variable .. ".length"] then
			data = wml.array_access.get(cfg.to_variable, variables)
		else
			data = {variables_proxy[cfg.to_variable]}
		end
	else
		for i,child in ipairs(cfg) do
			if child.tag == "value" then
				table.insert(data, wml.parsed(child.contents))
			elseif child.tag == "literal" then
				table.insert(data, wml.literal(child.contents))
			elseif child.tag == "split" then
				local to_split = child.contents.list
				local separator = child.contents.separator
				local key_name = child.contents.key or "value"
				local remove_empty = child.contents.remove_empty
				if separator then
					if #separator > 1 then
						wml.error(stringx.vformat("[set_variables] [split] separator only supports 1 character, multiple passed: $separator with $settings", {
							separator = separator,
							settings = wml.tostring(wml.literal(cfg))
						}))
					end
					for j, split in ipairs(stringx.split(to_split, separator, {remove_empty = remove_empty, strip_spaces = true})) do
						table.insert(data, {[key_name] = split})
					end
				else
					for j, char in ipairs(to_split) do
						table.insert(data, {[key_name] = char})
					end
				end
			end
		end
	end
	local mode = cfg.mode or "replace"
	local realvar, idx = name:match('^(.*)%[(%d+)%]$')
	if not realvar or not idx then
		realvar = name
		idx = nil
	end
	local var_array = wml.array_access.get(realvar, variables)
	if mode == "append" then
        -- append just means "insert at end"
        idx = #var_array
        mode = "insert"
    end
	if mode == "merge" then
		-- replace data with the result of the merge, then use "replace" logic to change var_array
		if idx then
			local data_merged = {}
			for i = 1, #data do
				data_merged = wml.merge(data_merged, data[i].contents, "append")
			end
			data = { wml.merge(var_array[idx + 1] or {}, data_merged, "merge") }
		else
			local var_wml = append_array_to_wml({}, "value", var_array)
			local data_wml = append_array_to_wml({}, "value", data)
			data = wml.child_array(wml.merge(var_wml, data_wml), "value")
		end
		mode = "replace"
    end
    if mode == "replace" then
        -- implement replace as "delete then insert"
		if idx then
			table.remove(var_array, math.min(#var_array + 1, idx + 1))
		else
			var_array = {}
		end
        mode = "insert"
    end
    if mode == "insert" then
        idx = idx or 0
        while(#arr < idx)
            arr[#arr + 1] = {}
        end
        for i = 1, #data do
            table.insert(var_array, idx + i, data[i])
        end
    else
        wml.error("unknown mode for [set_variables]: " .. cfg.mode)
    end
	wml.array_access.set(realvar, var_array, variables)
end
