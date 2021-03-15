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
local locset_meta = {}

function locset_meta:__call(x, y)
	return self:get(x, y)
end

function locset_meta:__index(loc)
	if type(loc) == 'string' then
		return methods[loc]
	elseif loc.x and loc.y then
		return self:get(loc.x, loc.y)
	else
		return self:get(loc[1], loc[2])
	end
end

function locset_meta:__newindex(loc, val)
	local fcn = methods.insert
	if val == nil then fcn = methods.remove end
	if loc.x and loc.y then
		fcn(self, loc.x, loc.y, val)
	else
		fcn(self, loc[1], loc[2], val)
	end
end

function locset_meta:__bor(other)
	local new = self:clone()
	new:union(other)
	return new
end

function locset_meta:__band(other)
	local new = self:clone()
	new:inter(other)
	return new
end

function locset_meta:__bxor(other)
	local new = self:clone()
	new:symm(other)
	return new
end

function locset_meta:__sub(other)
	local new = self:clone()
	new:diff(other)
	return new
end

function locset_meta:__tostring()
	local res = {}
	self:iter(function(x, y, v) res[string.format('(%d,%d)', x, y)] = tostring(v) end)
	return '{' .. stringx.join_map(res, '; ', ' = ') .. '}'
end

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

function methods:get(...)
	local loc = wesnoth.map.read_location(...)
	return self.values[index(loc.x, loc.y)]
end

function methods:insert(...)
	local loc, n = wesnoth.map.read_location(...)
	local v = select(n + 1, ...)
	self.values[index(loc.x, loc.y)] = v or true
end

function methods:remove(...)
	local loc = wesnoth.map.read_location(...)
	self.values[index(loc.x, loc.y)] = nil
end

function methods:clone()
	local new = location_set.create()
	for p,v in pairs(self.values) do
		new.values[p] = v
	end
	return new
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

function methods:diff(s)
	local values = self.values
	for p,v in pairs(s.values) do
		values[p] = nil
	end
end

function methods:symm(s)
	local values = self.values
	for p,v in pairs(s.values) do
		if values[p] then
			values[p] = nil
		else
			values[p] = v
		end
	end
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
	if f == nil then
		local locs = self
		return coroutine.wrap(function()
			locs:stable_iter(coroutine.yield)
		end)
	end
	local indices = {}
	for p,v in pairs(self.values) do
		table.insert(indices, p)
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
	for i = 0, wml.variables[name .. ".length"] - 1 do
		local t = wml.variables[string.format("%s[%d]", name, i)]
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
	wml.variables[name] = nil
	self:stable_iter(function(x, y, v)
		if type(v) == 'table' then
			wml.variables[string.format("%s[%d]", name, i)] = v
		end
		wml.variables[string.format("%s[%d].x", name, i)] = x
		wml.variables[string.format("%s[%d].y", name, i)] = y
		i = i + 1
	end)
end

function methods:to_triples()
    local res = {}
    self:iter(function(x, y, v) table.insert(res, { x, y, v }) end)
    return res
end

function methods:random()
	-- Select a random hex from the hexes in the location set
	-- This seems "inelegant", but I can't come up with another way without creating an extra array
	-- Return -1, -1 if empty
	local r = wesnoth.random(self:size())
	local i, xr, yr = 1, -1, -1
	self:iter( function(x, y, v)
		if (i == r) then xr, yr = x, y end
		i = i + 1
	end)
	return xr, yr
end

function location_set.create()
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
