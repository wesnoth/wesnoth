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

---@alias location_set_operation fun(x:integer, y:integer, value:any):boolean|nil
---@alias location_set_resolver fun(x:integer, y:integer, old:any, new:any):any

---A set of locations, with an optional associated value for each one.
---@class location_set : { [location]: any }
---@field private values table<integer, any>
---@operator bnot:location_set
---@operator band:location_set
---@operator bor:location_set
---@operator bxor:location_set
---@operator sub:location_set
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

if wesnoth.current then
	function locset_meta:__bnot(other)
		return self:invert(wesnoth.current.map)
	end
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

---Test if the set is empty
---@return boolean
function methods:empty()
	return (not next(self.values))
end

---Count the number of locations in the set
---@return integer
function methods:size()
	local sz = 0
	for p,v in pairs(self.values) do
		sz = sz + 1
	end
	return sz
end

---Remove all locations from the set
function methods:clear()
	self.values = {}
end

---Look up a location in the set
---@overload fun(set:location_set, x:integer, y:integer):any
---@overload fun(set:location_set, loc:location):any
function methods:get(...)
	local loc = wesnoth.map.read_location(...)
	if loc ~= nil then
		return self.values[index(loc.x, loc.y)]
	end
	return nil
end

---Add a location to the set
---@overload fun(set:location_set, x:integer, y:integer, value?:any)
---@overload fun(set:location_set, loc:location, value?:any)
function methods:insert(...)
	local loc, n = wesnoth.map.read_location(...)
	if loc ~= nil then
		local v = select(n + 1, ...)
		self.values[index(loc.x, loc.y)] = v or true
	end
end

---Remove a location from the set
---@overload fun(self:location_set, x:integer, y:integer)
---@overload fun(self:location_set, loc:location|unit)
function methods:remove(...)
	local loc = wesnoth.map.read_location(...)
	if loc ~= nil then
		self.values[index(loc.x, loc.y)] = nil
	end
end

---Create a copy of the set
---This is a shallow copy - if the values are tables they will still
---be referenced by the original set
---@return location_set
function methods:clone()
	local new = location_set.create()
	for p,v in pairs(self.values) do
		new.values[p] = v
	end
	return new
end

---Take the union of two sets, replacing duplicate values
---@param s location_set The other set to merge in
function methods:union(s)
	local values = self.values
	for p,v in pairs(s.values) do
		values[p] = v
	end
end

---Take the union of two sets, merging duplicate values
---@param s location_set The other set to merge in
---@param f location_set_resolver A function which is used to resolve conflicts.
---It will be called for every element of the right-hand set.
function methods:union_merge(s, f)
	local values = self.values
	for p,v in pairs(s.values) do
		local x, y = revindex(p)
		values[p] = f(x, y, values[p], v)
	end
end

---Take the intersection of two sets, by removing any elements in the left set that
---are missing from the right set.
---@param s location_set The other set to intersect with
function methods:inter(s)
	local values = self.values
	local nvalues = {}
	for p,v in pairs(s.values) do
		nvalues[p] = values[p]
	end
	self.values = nvalues
end

---Take the union of two sets, merging duplicate values
---@param s location_set The other set to merge in
---@param f location_set_resolver A function which is used to resolve conflicts.
---It will be called for every element common to both sets.
function methods:inter_merge(s, f)
	local values = s.values
	local nvalues = {}
	for p,v in pairs(self.values) do
		local x, y = revindex(p)
		nvalues[p] = f(x, y, v, values[p])
	end
	self.values = nvalues
end

---Take the relative complement of two location sets.
---This removes any elements from the left set that are also in the right set.
---@param s location_set
function methods:diff(s)
	local values = self.values
	for p,v in pairs(s.values) do
		values[p] = nil
	end
end

---Take the symmetric difference of two location sets.
---This removes any elements that are common to both sets,
---while adding new elements to the left set if they only exist in the right.
---@param s location_set
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

---Take the absolute complement of the location set
---The resulting set has no values associated with each location.
---@param width integer The width of the map
---@param height integer The height of the map
---@param border_size integer The border size of the map
---@return location_set
---@overload fun(map:terrain_map):location_set
function methods:invert(width, height, border_size)
	if type(width) == 'number' and type(height) == 'number' then
		border_size = border_size or 0
	elseif type(width) == 'userdata' and getmetatable(width) == 'terrain map' then
		---@type terrain_map
		local map = width ---@diagnostic disable-line : assign-type-mismatch
		width = map.playable_width
		height = map.playable_height
		border_size = map.border_size
	else
		error('Invalid arguments to location_set:invert - expected a map or map dimensions', 2)
	end
	local new = location_set.create()
	for x = 1 - border_size, width + border_size do
		for y = 1 - border_size, height + border_size do
			if not self:get(x, y) then
				new:insert(x, y)
			end
		end
	end
	return new
end

---Filter the set for elements that satisfy a given condition.
---Returns a new set containing only the matching values.
---@param f location_set_operation The condition to test
---@return location_set
function methods:filter(f)
	local nvalues = {}
	for p,v in pairs(self.values) do
		local x, y = revindex(p)
		if f(x, y, v) then nvalues[p] = v end
	end
	return setmetatable({ values = nvalues }, locset_meta)
end

---Iterate over the location set.
---If passed no arguments, it can be used in a range-for loop.
---@param f location_set_operation
---@overload fun():fun()
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

---Iterate over the location set in a sorted order.
---If passed no arguments, it can be used in a range-for loop.
---@param f location_set_operation
---@overload fun():fun()
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

---Add a series of locations to the set.
---If the location tables have extra information in them, it will be used as the value.
---@param t location[]
function methods:of_pairs(t)
	local values = self.values

	local function has_key(v, k)
		if type(v) == 'userdata' then
			return v[k] ~= nil
		elseif type(v) == 'table' then
			return rawget(v, k) ~= nil
		end
	end

	for i,v in ipairs(t) do
		local value_table = {}
		local x_index
		local y_index
		if has_key(v, 'x') and has_key(v, 'y') then
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

---Add locations stored in a WML variable.
---If the location tables have extra information in them, it will be used as the value.
---@param name string
function methods:of_wml_var(name)
	local values = self.values
	for i = 0, wml.variables[name .. ".length"] - 1 do
		local t = wml.variables[string.format("%s[%d]", name, i)]
		local x, y = t.x, t.y
		t.x, t.y = nil, nil
		values[index(x, y)] = next(t) and t or true
	end
end

---Add a series of locations to the set.
---The third element of each location is used as the value.
---@param t location_triple[]
function methods:of_triples(t)
	-- Create a location set from a table of 3-element tables
	-- Elements 1 and 2 are x,y coordinates, #3 is value to be inserted
	for k,v in pairs(t) do
		if #v == 0 then
			self:insert(v.x, v.y, v.value)
		else
			self:insert(v[1], v[2], v[3])
		end
	end
end

--- Add values from a table of location->element mappings
--- Keys can be of the form {x,y} or {x=x,y=y}, or a location-like object such as a unit
---@param t table<location, any>
function methods:of_map(t)
	for k,v in pairs(t) do
		local loc = wesnoth.read_location(k)
		self:insert(loc.x, loc.y, v)
	end
end

---Add a series of locations from a shroud data string.
---Each location indicated by a 1 will be added to the set.
---@param data string
function methods:of_shroud_data(data)
	self:of_pairs(wesnoth.map.parse_bitmap(data))
end

---Convert the set to an array of locations.
---The value will not be stored in the output array.
---@return location[]
function methods:to_pairs()
	local res = {}
	self:iter(function(x, y)
		table.insert(res, wesnoth.named_tuple({ x, y }, {'x', 'y'}))
	end)
	return res
end

---Convert the set to an array of locations in a sorted order.
---The value will not be stored in the output array.
---@return location[]
function methods:to_stable_pairs()
	local res = {}
	self:stable_iter(function(x, y)
		table.insert(res, wesnoth.named_tuple({ x, y }, {'x', 'y'}))
	end)
	return res
end

---Store the set in a WML variable
---@param name string
---@param mode? "'always_clear'"|"'append'"|"'replace'"
function methods:to_wml_var(name, mode)
	mode = mode or "always_clear"
	local is_explicit_index = name[-1] == "]"
	local i = 0
	-- explicit indexes behave always like "replace"
	if not is_explicit_index then
		if mode == "append" then
			i = wml.variables[name .. ".length"]
		elseif mode ~= "replace" then
			wml.variables[name] = nil
		end
	end
	self:stable_iter(function(x, y, v)
		if wml.valid(v) then
			wml.variables[string.format("%s[%d]", name, i)] = v
		elseif wml.valid{value = v} then
			wml.variables[string.format("%s[%d]", name, i)] = {value = v}
		elseif type(v) ~= 'boolean' then
			warn('Location set value could not be converted to a WML variable:', v)
		end
		wml.variables[string.format("%s[%d].x", name, i)] = x
		wml.variables[string.format("%s[%d].y", name, i)] = y
		i = i + 1
	end)
end

---Convert the set to an array of triples - locations with an extra element for the value.
---@return location_triple[]
function methods:to_triples()
	local res = {}
	self:iter(function(x, y, v)
		table.insert(res, wesnoth.named_tuple({ x, y, v }, {"x", "y", "value"}))
	end)
	return res
end

---Convert the set to a map of location -> value
---@return table<location, any>
function methods:to_map()
	local res = {}
	self:iter(function(x, y, v)
		res[wesnoth.named_tuple({x, y}, {"x", "y"})] = v
	end)
	return res
end

---Convert the set to a shroud data string
--- Each location in the set will be a 1 in the output string.
---@return string
function methods:to_shroud_data()
	return wesnoth.map.make_bitmap(self:to_pairs())
end

---Select a random location from the set
---@return integer #x
---@return integer #y
function methods:random()
	-- Select a random hex from the hexes in the location set
	-- This seems "inelegant", but I can't come up with another way without creating an extra array
	-- Return -1, -1 if empty
	local r = mathx.random(self:size())
	local i, xr, yr = 1, -1, -1
	self:iter( function(x, y, v)
		if (i == r) then xr, yr = x, y end
		i = i + 1
	end)
	return xr, yr
end

---Create a new empty location set.
---@return location_set
function location_set.create()
	return setmetatable({ values = {} }, locset_meta)
end

---Create a location set from raw indexed data.
---The data must be formatted the same way as the values of a location set.
---@param data table<integer, any>
---@return location_set
function location_set.of_raw(data)
	return setmetatable({ values = data }, locset_meta)
end

---Create a set with locations from an array.
---If the location tables have extra information in them, it will be used as the value.
---@param t location[]
---@return location_set
function location_set.of_pairs(t)
	local s = location_set.create()
	s:of_pairs(t)
	return s
end

---Create a set with locations from a WML variable.
---If the location tables have extra information in them, it will be used as the value.
---@param name string
---@return location_set
function location_set.of_wml_var(name)
	local s = location_set.create()
	s:of_wml_var(name)
	return s
end

---Create a set with locations from an array.
---The third element of each location is used as the value.
---@param t location_triple[]
---@return location_set
function location_set.of_triples(t)
	local s = location_set.create()
	s:of_triples(t)
	return s
end

--- Create a set from a table of location->element mappings
--- Keys can be of the form {x,y} or {x=x,y=y}, or a location-like object such as a unit
---@param t table<location, any>
---@return location_set
function location_set.of_map(t)
	local s = location_set.create()
	s:of_map(t)
	return s
end

---Create a set with locations from a shroud data string.
---Each location indicated by a 1 will be added to the set.
---@param data string
---@return location_set
function location_set.of_shroud_data(data)
	local s = location_set.create()
	s:of_shroud_data(data)
	return s
end

return location_set
