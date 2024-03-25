-- This file implements equivalents of various higher-order WFL functions

local functional = {}

---Filter an array for elements matching a certain condition
---@generic T
---@param input T[]
---@param condition fun(val:T):boolean
---@return T[]
function functional.filter(input, condition)
	local filtered_table = {}

	for _,v in ipairs(input) do
		if condition(v) then
			table.insert(filtered_table, v)
		end
	end

	return filtered_table
end

---Filter a map for elements matching a certain condition
---@generic K
---@generic V
---@param input table<K, V>
---@param condition fun(key:K, val:V):boolean
---@return table<K, V>
function functional.filter_map(input, condition)
	local filtered_table = {}

	for k,v in pairs(input) do
		if condition(k, v) then
			filtered_table[k] = v
		end
	end

	return filtered_table
end

---Search an array for an element matching a certain condition
---@generic T
---@param input T[]
---@param condition fun(val:T):boolean
---@return T?
function functional.find(input, condition)
	for _,v in ipairs(input) do
		if condition(v) then
			return v
		end
	end
end

---Search a map for a key-value pair matching a certain condition
---@generic K
---@generic V
---@param input table<K, V>
---@param condition fun(key:K, val:V):boolean
---@return K?
---@return V?
function functional.find_map(input, condition)
	for k,v in pairs(input) do
		if condition(k,v) then
			return k, v
		end
	end
end

---Find the element of an array with the largest value
---@generic T
---@param input T[]
---@param value fun(val:T):number
---@return T
---@return number
---@return integer
function functional.choose(input, value)
	-- Equivalent of choose() function in Formula AI
	-- Returns element of a table with the largest @value (a function)
	-- Also returns the max value and the index
	if value == nil then
		value = function(v) return v end
	elseif type(value) ~= 'function' then
		local key = value
		value = function(v) return v[key] end
	end

	local max_value, best_input, best_key = -math.huge, nil, nil
	for k,v in ipairs(input) do
		local v2 = value(v)
		if v2 > max_value then
			max_value, best_input, best_key = v2, v, k
		end
	end

	return best_input, max_value, best_key
end

---Find the key-value pair in a map with the largest value
---@generic K
---@generic V
---@param input table<K, V>
---@param value fun(key:K, val:V):number
---@return {[1]:K, [2]:V}
---@return number
function functional.choose_map(input, value)
	-- Equivalent of choose() function in Formula AI
	-- Returns element of a table with the largest @value (a function)
	-- Also returns the max value and the index
	if value == nil then
		value = function(k, v) return v end
	elseif type(value) ~= 'function' then
		local key = value
		value = function(k, v) return v[key] end
	end

	local max_value, best_input, best_key = -math.huge, nil, nil
	for k,v in pairs(input) do
		local v2 = value(k, v)
		if v2 > max_value then
			max_value, best_input, best_key = v2, v, k
		end
	end

	return {key = best_key, value = best_input}, max_value
end

---Map the elements of an array according to an operation
---@generic T1
---@generic T2
---@param input T1[]
---@param formula fun(val:T1):T2
---@return T2[]
function functional.map_array(input, formula)
	local mapped_table = {}
	for n,v in ipairs(input) do
		table.insert(mapped_table, formula(v))
	end
	return mapped_table
end

---Map the values of a dictionary according to an operation
---@generic K
---@generic V1
---@generic V2
---@param input table<K, V1>
---@param formula fun(key:K,val:V1):V2
---@return table<K, V2>
function functional.map(input, formula)
	local mapped_table = {}
	for k,v in pairs(input) do
		mapped_table[k] = formula(v, k)
	end
	return mapped_table
end

local known_operators = {
	['+'] = function(a, b) return a + b end,
	['-'] = function(a, b) return a - b end,
	['*'] = function(a, b) return a * b end,
	['/'] = function(a, b) return a / b end,
	['%'] = function(a, b) return a % b end,
	['^'] = function(a, b) return a ^ b end,
	['//'] = function(a, b) return a // b end,
	['&'] = function(a, b) return a & b end,
	['|'] = function(a, b) return a | b end,
	['~'] = function(a, b) return a ~ b end,
	['<<'] = function(a, b) return a << b end,
	['>>'] = function(a, b) return a >> b end,
	['..'] = function(a, b) return a .. b end,
	['=='] = function(a, b) return a == b end,
	['~='] = function(a, b) return a ~= b end,
	['<'] = function(a, b) return a < b end,
	['>'] = function(a, b) return a > b end,
	['<='] = function(a, b) return a <= b end,
	['>='] = function(a, b) return a >= b end,
	['and'] = function(a, b) return a and b end,
	['or'] = function(a, b) return a or b end,
}

-- Reduce the elements of input array into a single value. operator is called as
--- 'operator(accumulator, element)' for every element in t. If a 3rd argument
--- is provided, even as nil, it will be used as the accumulator when
--- calling operator on the first element. If there is no 3rd argument, the
--- first operator call will be on the first two elements. If there is no 3rd
--- argument and the array is empty, return nil. operator may be a function or a
--- binary Lua operator as a string.
---@generic T
---@param input T[]
---@param operator string|fun(a:T, b:T):T
---@param ... T The initial value of the accumulator, typically the identity element.
---@return T
function functional.reduce(input, operator, ...)
	local f <const> = known_operators[operator] or operator

	local function loop(init, i)
		local value <const> = input[i]
		if value == nil then
			return init
		end
		return loop(f(init, value), i + 1)
	end

	if select('#', ...) == 0 then
		return loop(input[1], 2)
	end
	return loop(select(1, ...), 1)
end

---Take elements of an array until the condition fails
---@generic T
---@param input T[]
---@param condition fun(val:T):boolean
---@return T[]
function functional.take_while(input, condition)
	local truncated_table = {}
	for _,v in ipairs(input) do
		if not condition(v) then
			break
		end
		table.insert(truncated_table, v)
	end
	return truncated_table
end

---Given an array of arrays, produce a new array of arrays where the first has every first element the second has every second element, etc
---@param input any[][]
---@return any[][]
function functional.zip(input)
	-- Technically not a higher-order function, but whatever...
	local output = {}
	local _, n = functional.choose(input, function(list) return #list end)
	for i = 1, n do
		local elem = {}
		for j, list in ipairs(input) do
			elem[j] = list[i]
		end
		table.insert(output, elem)
	end
	return output
end

return functional
