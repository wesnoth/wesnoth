
----------------------------------------------------------
---- A utility class, to calculate distances of each  ----
---- tile to one given tile                           ----
----------------------------------------------------------


local Distmap = {}

function Distmap:create(w, h)
	local o = {}
	setmetatable(o, self)
	self.__index = self
	o.w = w
	o.h = h
	o.data = {}
	for i = 1,w *h do
		o.data[#o.data + 1] = false
	end
	return o
end

function Distmap:loc_to_index(loc)
	return loc[1] + 1 + loc[2] * self.w
end

function Distmap:is_on_map(loc)
	local x, y= loc[1], loc[2]
	return x >= 0 and y >= 0 and y < self.h and x < self.w
end

function Distmap:get(loc)
	return self.data[self:loc_to_index(loc)]
end


local adjacent_offset = {
	--odd x
	{ {0,-1}, {1,-1}, {1,0}, {0,1}, {-1,0}, {-1,-1} },
	--even x
	{ {0,-1}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0} }
}

function Distmap:adjacent_tiles(loc, filter)
	local x, y = loc[1], loc[2]
	local offsets = adjacent_offset[2 - (x % 2)]
	local res = {}
	for i, offset in ipairs(offsets) do
		local x2, y2 = x + offset[1], y + offset[2]
		if self:is_on_map({x2, y2}) and ((filter == nil) or filter(x2, y2)) then
			res[#res + 1] = { x2, y2}
		end
	end
	return res
end

function Distmap:calculate_distances(locs, upto, filter)
	wesnoth.log("info", "calculate_distances " .. upto .. " " .. #locs)
	local todo = locs
	local data = self.data
	for i,loc in ipairs(todo) do
		data[self:loc_to_index(loc)] = 0
	end
	while #todo ~= 0 do
		local loc = todo[1]
		local loc_i = self:loc_to_index(loc)
		local dist = self.data[loc_i] + 1
		for i2, loc2 in ipairs(self:adjacent_tiles(loc, filter)) do
			local loc2_i = self:loc_to_index(loc2)
			if (data[loc2_i] or 999) > dist then
				data[loc2_i] = dist
				todo[#todo + 1] = loc2
			end
		end
		table.remove(todo, 1)
	end
end

function Distmap:std_print(loc)
	local data = functional.map(self.data, function(v)
		return tostring(v or "nil")
	end)
	for i =1, self.h do
		std_print(table.concat(data, "\t,\t", 1 + (i-1)* self.w, i * self.w ))
	end
end

return Distmap
