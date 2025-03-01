--[========[Additional mathematical functions]========]
print("Loading mathx module...")

---Pick a random choice from a list of values
---@param possible_values string|table Either a comma-separated list of values (which can contain integer ranges like 2-7)
---or an array of possible values (which can also contain integer ranges as subtables with {lo, hi} elements)
---@param random_func? fun(a:integer,b:integer):number
---@return number|string|nil
function mathx.random_choice(possible_values, random_func)
	random_func = random_func or mathx.random
	assert(type(possible_values) == "table" or type(possible_values) == "string",
		string.format("mathx.random_choice expects a string or table as parameter, got %s instead",
		type(possible_values)))

	local items = {}
	local num_choices = 0

	if type(possible_values) == "string" then
		-- split on commas
		for _,word in ipairs(possible_values:quoted_split()) do
			-- does the word contain two dots? If yes, that's a range
			local dots_start, dots_end = word:find("%.%.")
			if dots_start then
				-- split on the dots if so and cast to numbers
				local low = tonumber(word:sub(1, dots_start-1))
				local high = tonumber(word:sub(dots_end+1))
				-- perhaps someone passed a string as part of the range, intercept the issue
				if not (low and high) then
					wesnoth.interface.add_chat_message("Malformed range: " .. word)
					table.insert(items, word)
					num_choices = num_choices + 1
				else
					if low > high then
						-- low is greater than high, swap them
						low, high = high, low
					end

					-- if both ends represent the same number, then just use that number
					if low == high then
						table.insert(items, low)
						num_choices = num_choices + 1
					else
						-- insert a table representing the range
						table.insert(items, {low, high})
						-- how many items does the range contain? Increase difference by 1 because we include both ends
						num_choices = num_choices + (high - low) + 1
					end
				end
			else
				-- handle as a string
				table.insert(items, word)
				num_choices = num_choices + 1
			end
		end
	else
		num_choices = #possible_values
		items = possible_values
		-- We need to parse ranges separately anyway
		for i, val in ipairs(possible_values) do
			if type(val) == "table" then
				assert(#val == 2 and type(val[1]) == "number" and type(val[2]) == "number", "Malformed range for mathx.random_choice")
				if val[1] > val[2] then
					val = {val[2], val[1]}
				end
				num_choices = num_choices + (val[2] - val[1])
			end
		end
	end

	local idx = random_func(1, num_choices)

	for i, item in ipairs(items) do
		if type(item) == "table" then -- that's a range
			local elems = item[2] - item[1] + 1 -- amount of elements in the range, both ends included
			if elems >= idx then
				return item[1] + elems - idx
			else
				idx = idx - elems
			end
		else -- that's a single element
			idx = idx - 1
			if idx == 0 then
				return item
			end
		end
	end

	return nil
end

---Randomize the order of an array
---@param t any[]
---@param random_func? fun(a:number,b:number):number
function mathx.shuffle(t, random_func)
	random_func = random_func or mathx.random
	-- since tables are passed by reference, this is an in-place shuffle
	-- it uses the Fisher-Yates algorithm, also known as Knuth shuffle
	assert(type(t) == "table", string.format("mathx.shuffle expects a table as parameter, got %s instead", type(t)))
	local length = #t
	for index = length, 2, -1 do
		local random = random_func(1, index)
		t[index], t[random] = t[random], t[index]
	end
end

---Compute a linear interpolation
---@param lo number
---@param hi number
---@param alpha number
---@return number
function mathx.lerp(lo, hi, alpha)
	return lo + alpha * (hi - lo)
end

---Choose an element from a list based on a ratio.
---@generic T
---@param list T[]
---@param alpha number
---@return T
function mathx.lerp_index(list, alpha)
	if #list == 0 then return nil end
	return list[mathx.round(mathx.lerp(1, #list, alpha))]
end

---Clamp a number into a specified range
---@param val number
---@param lo number
---@param hi number
---@return number
function mathx.clamp(val, lo, hi)
	return math.min(hi, math.max(lo, val))
end

wesnoth.random = wesnoth.deprecate_api('wesnoth.random', 'mathx.random', 1, nil, mathx.random)
