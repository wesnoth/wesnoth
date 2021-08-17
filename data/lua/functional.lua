-- This file implements equivalents of various higher-order WFL functions

local functional = {}

function functional.filter(input, condition)
    local filtered_table = {}

    for _,v in ipairs(input) do
        if condition(v) then
            table.insert(filtered_table, v)
        end
    end

    return filtered_table
end

function functional.filter_map(input, condition)
    local filtered_table = {}

    for k,v in pairs(input) do
        if condition(k, v) then
            filtered_table[k] = v
        end
    end

    return filtered_table
end

function functional.find(input, condition)
	for _,v in ipairs(input) do
		if condition(v) then
			return v
		end
	end
end

function functional.find_map(input, condition)
	for k,v in pairs(input) do
		if condition(k,v) then
			return k, v
		end
	end
end

function functional.choose(input, value)
    -- Equivalent of choose() function in Formula AI
    -- Returns element of a table with the largest @value (a function)
    -- Also returns the max value and the index

    local max_value, best_input, best_key = -math.huge
    for k,v in ipairs(input) do
		local v2 = value(v)
        if v2 > max_value then
            max_value, best_input, best_key = v2, v, k
        end
    end

    return best_input, max_value, best_key
end

function functional.choose_map(input, value)
    -- Equivalent of choose() function in Formula AI
    -- Returns element of a table with the largest @value (a function)
    -- Also returns the max value and the index

    local max_value, best_input, best_key = -math.huge
    for k,v in pairs(input) do
		local v2 = value(k, v)
        if v2 > max_value then
            max_value, best_input, best_key = v2, v, k
        end
    end

    return {key = best_key, value = best_input}, max_value
end

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

function functional.reduce(input, operator, identity)
	if type(operator) == 'string' then
		operator = known_operators[operator]
	end
	local result = identity or 0
	for _, v in ipairs(input) do
		result = operator(result, v)
	end
	return result
end

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

-- Technically not a higher-order function, but whatever...
function functional.zip(input)
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
