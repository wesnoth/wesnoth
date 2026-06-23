
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
	local explicit_idx = true
	if not realvar or not idx then
		realvar = name
		idx = 0
		explicit_idx = false
	end
	if mode == "merge" or mode == "replace" then
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
		local tail = {}
		if explicit_idx then
			-- Note: idx is a WML index, so it starts at 0, not 1
			idx = idx + 1
			-- merge: add head, merge new element(s) with current element, add tail
			-- replace: add head, add new element(s), add tail
			for i = 1, #merge_with do
				if i < idx then
					head[i] = merge_with[i]
				end
				if i > idx then
					table.insert(tail, merge_with[i])
				end
			end

			if mode == "merge" then
				-- For merge mode, all [value] and [literal] tags are joined together before being merged into the specific element
				local data_merged = {}
				for i = 1, #data do
					data_merged = wml.merge(data_merged, data[i].contents, "append")
				end
				local merge_contents = merge_with[idx] and merge_with[idx].contents or {}
				data_merged = wml.merge(merge_contents, data_merged, mode)
				data = {wml.tag.value(data_merged)}
			end

			-- empty containers should be created when idx is past end of original data
			local padding_needed = idx - #merge_with - 1
			while padding_needed > 0 do
				table.insert(data, 1, wml.tag.value{})
				padding_needed = padding_needed - 1
			end

			-- If we started at a specific index, add back everything that came before and after
			local merged = {}
			for i = 1, #head do
				table.insert(merged, head[i])
			end
			for i = 1, #data do
				table.insert(merged, data[i])
			end
			for i = 1, #tail do
				table.insert(merged, tail[i])
			end
			wml.array_access.set(realvar, wml.child_array(merged, 'value'), variables)
		else
			if mode == "merge" then
				data = wml.merge(merge_with, data, mode)
			end
			wml.array_access.set(realvar, wml.child_array(data, 'value'), variables)
		end
	elseif mode == "append" then
		local insert_into = wml.array_access.get(realvar, variables)
		for i = 1, #data do
			table.insert(insert_into, data[i])
		end
		wml.array_access.set(realvar, insert_into, variables);
	elseif mode == "insert" then
		local insert_into = wml.array_access.get(realvar, variables)
		while #insert_into < 0 + idx do
			table.insert(insert_into, {})
		end
		for i = 1, #data do
			table.insert(insert_into, idx + i, data[i])
		end
		wml.array_access.set(realvar, insert_into, variables);
	else
		wml.error("unknown mode for [set_variables]: " .. cfg.mode)
	end
end
