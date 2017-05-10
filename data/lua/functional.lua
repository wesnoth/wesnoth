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

    local max_value, best_input, best_key = -9e99
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

    local max_value, best_input, best_key = -9e99
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
		if type(k) == 'number' then
			table.insert(mapped_table, formula(v))
		else
			mapped_table[k] = formula(v, k)
		end
	end
	return mapped_table
end

function functional.reduce(input, operator, identity)
	if #input == 0 then return identity end
	local value = operator(identity or 0, input[1])
	if #input == 1 then return value end
	for i = 2, #input do
		value = operator(value, input[i])
	end
	return value
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

return functional
