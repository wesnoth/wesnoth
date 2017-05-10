local location_set = {}

local function index(x, y)
	-- the 2000 bias ensure that the correct x is recovered for negative y
	return x * 16384 + y + 2000
end

local invscale = 1 / 16384
local function revindex(p)
	local x = math.floor(p * invscale)
	return x, p - x * 16384 - 2000
end

local methods = {}
local locset_meta = { __index = methods }

function methods:empty()
	return (not next(self.values))
end

function methods:size()
	local sz = 0
	for p,v in pairs(self.values) do
		sz = sz + 1
	end
	return sz
end

function methods:clear()
	self.values = {}
end

function methods:get(x, y)
	return self.values[index(x, y)]
end

function methods:insert(x, y, v)
	self.values[index(x, y)] = v or true
end

function methods:remove(x, y)
	self.values[index(x, y)] = nil
end

function methods:union(s)
	local values = self.values
	for p,v in pairs(s.values) do
		values[p] = v
	end
end

function methods:union_merge(s, f)
	local values = self.values
	for p,v in pairs(s.values) do
		local x, y = revindex(p)
		values[p] = f(x, y, values[p], v)
	end
end

function methods:inter(s)
	local values = self.values
	local nvalues = {}
	for p,v in pairs(s.values) do
		nvalues[p] = values[p]
	end
	self.values = nvalues
end

function methods:inter_merge(s, f)
	local values = s.values
	local nvalues = {}
	for p,v in pairs(self.values) do
		local x, y = revindex(p)
		nvalues[p] = f(x, y, v, values[p])
	end
	self.values = nvalues
end

function methods:filter(f)
	local nvalues = {}
	for p,v in pairs(self.values) do
		local x, y = revindex(p)
		if f(x, y, v) then nvalues[p] = v end
	end
	return setmetatable({ values = nvalues }, locset_meta)
end

function methods:iter(f)
	if f == nil then
		local locs = self
		return coroutine.wrap(function()
			locs:iter(coroutine.yield)
		end)
	end
	for p,v in pairs(self.values) do
		local x, y = revindex(p)
		f(x, y, v)
	end
end

function methods:stable_iter(f)
	local indices = {}
	for p,v in pairs(self.values) do
		table.insert(indices, p)
	end
	if f == nil then
		local locs = self
		return coroutine.wrap(function()
			locs:stable_iter(coroutine.yield)
		end)
	end
	table.sort(indices)
	for i,p in ipairs(indices) do
		local x, y = revindex(p)
		f(x, y, self.values[p])
	end
end

function methods:of_pairs(t)
	local values = self.values

	for i,v in ipairs(t) do
		local value_table = {}
		local x_index
		local y_index
		if v.x and v.y then
			x_index = "x"
			y_index = "y"
		else
			x_index = 1
			y_index = 2
		end
		for k,val in pairs(v) do
			if k ~= x_index and k ~= y_index then
				value_table[k] = val
			end
		end
		if next(value_table) then
			values[index(v[x_index], v[y_index])] = value_table
		else
			values[index(v[x_index], v[y_index])] = true
		end
	end
end

function methods:of_wml_var(name)
	local values = self.values
	for i = 0, wesnoth.get_variable(name .. ".length") - 1 do
		local t = wesnoth.get_variable(string.format("%s[%d]", name, i))
		local x, y = t.x, t.y
		t.x, t.y = nil, nil
		values[index(x, y)] = next(t) and t or true
	end
end

function methods:of_triples(t)
    -- Create a location set from a table of 3-element tables
    -- Elements 1 and 2 are x,y coordinates, #3 is value to be inserted
    for k,v in pairs(t) do
        self:insert(v[1], v[2], v[3])
    end
end

function methods:to_pairs()
	local res = {}
	self:iter(function(x, y) table.insert(res, { x, y }) end)
	return res
end

function methods:to_stable_pairs()
	local res = {}
	self:stable_iter(function(x, y) table.insert(res, { x, y }) end)
	return res
end

function methods:to_wml_var(name)
	local i = 0
	wesnoth.set_variable(name)
	self:stable_iter(function(x, y, v)
		if type(v) == 'table' then
			wesnoth.set_variable(string.format("%s[%d]", name, i), v)
		end
		wesnoth.set_variable(string.format("%s[%d].x", name, i), x)
		wesnoth.set_variable(string.format("%s[%d].y", name, i), y)
		i = i + 1
	end)
end

function methods:to_triples()
    local res = {}
    self:iter(function(x, y, v) table.insert(res, { x, y, v }) end)
    return res
end

function location_set.create()
	if wesnoth.get_map_size then
		-- If called from the mapgen kernel, there's no map
		local w,h,b = wesnoth.get_map_size()
		assert(h + 2 * b < 9000)
	end
	return setmetatable({ values = {} }, locset_meta)
end

function location_set.of_pairs(t)
	local s = location_set.create()
	s:of_pairs(t)
	return s
end

function location_set.of_wml_var(name)
	local s = location_set.create()
	s:of_wml_var(name)
	return s
end

function location_set.of_triples(t)
    local s = location_set.create()
    s:of_triples(t)
    return s
end

return location_set
