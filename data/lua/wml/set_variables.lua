
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
		data = wml.array_access.get(cfg.to_variable, variables)
	else
		for i,child in ipairs(cfg) do
			if child.tag == "value" then
				table.insert(data, wml.parsed(child.contents))
			elseif child.tag == "literal" then
				table.insert(data, wml.literal(child.contents))
			elseif child.tag == "split" then
				local to_split = child.contents.list
				local separator = child.contents.separator
				local key_name = child.contents.key
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
	local explicit_idx = true
	if not realvar or not idx then
		realvar = name
		idx = 0
		explicit_idx = false
	end
	if mode == "merge" or mode == "append" or mode == "replace" then
		local merge_with = wml.array_access.get(realvar, variables)
		-- Convert the arrays to WML tables
		for i = 1, #merge_with do
			merge_with[i] = wml.tag.value(merge_with[i])
		end
		for i = 1, #data do
			data[i] = wml.tag.value(data[i])
		end
		-- If specific index was specified, we start the operation at that index.
		-- Thus, split the array into two parts such as that index is the first element of the second part
		local head = {}
		if explicit_idx then
			-- Note: idx is a WML index, so it starts at 0, not 1
			idx = idx + 1
			for i = 1, idx - 1 do
				head[i] = merge_with[i]
			end
			for i = idx, #merge_with do
				merge_with[i - idx + 1] = merge_with[i]
				merge_with[i] = nil
			end
			if mode == "merge" then
				-- For merge mode, all the values are merged together before being merged into the specific element
				local data_merged = {}
				for i = 1, #data do
					data_merged = wml.merge(data_merged, data[i].contents, "append")
				end
				data = {wml.tag.value(data_merged)}
			elseif mode == "replace" then
				-- For replace mode, any elements after the explicit index are pushed up but otherwise left untouched
				for i = 2, #merge_with do
					table.insert(data, merge_with[i])
				end
			end
		end
		local merged = wml.merge(merge_with, data, mode)
		-- If we started at a specific index, add back everything that came before and after
		for i = 1, #head do
			table.insert(merged, i, head[i])
		end
		wml.array_access.set(realvar, wml.child_array(merged, 'value'), variables)
	elseif mode == "insert" then
		local insert_into = wml.array_access.get(realvar, variables)
		for i = 1, #data do
			table.insert(insert_into, idx + i, data[i])
		end
		wml.array_access.set(realvar, insert_into, variables);
	else
		wml.error("unknown mode for [set_variables]: " .. cfg.mode)
	end
end
