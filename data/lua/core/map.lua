--[========[Map module]========]
print("Loading map module...")

function wesnoth.map.split_terrain_code(code)
	return table.unpack(code:split('^', {remove_empty = false}))
end

function wesnoth.map.read_location(...)
	local x, y = ...
	if x == nil then return nil, 0 end
	if y == nil or type(x) == 'table' or type(x) == 'userdata' then
		if type(x.x) == 'number' and type(x.y) == 'number' then
			x, y = x.x, x.y
		elseif type(x[1]) == 'number' and type(x[2]) == 'number' then
			x, y = table.unpack(x)
		else
			return nil, 0
		end
		return {x = x, y = y}, 1
	elseif type(x) == 'number' and type(y) == 'number' then
		return {x = x, y = y}, 2
	end
	return nil, 0
end

if wesnoth.kernel_type() ~= "Application Lua Kernel" then
	-- possible terrain string inputs:
	-- A        A^       A^B      ^        ^B
	-- implied mode:
	-- both     base     both     overlay  overlay
	function wesnoth.map.replace_base(code)
		local base, overlay = wesnoth.map.split_terrain_code(code)
		if base == nil then -- ^ or ^B
			-- There's no base to replace with, so do nothing
			return ''
		else
			-- Use the specified base but ignore the overlay
			return base .. '^'
		end
	end

	function wesnoth.map.replace_overlay(code)
		local base, overlay = wesnoth.map.split_terrain_code(code)
		if overlay == nil or overlay == '' then -- A or A^
			-- No overlay was specified, so we want to clear the overlay without touching the base
			return '^'
		else
			-- An overlay was specified, so use that and ignore the base
			return '^' .. overlay
		end
	end

	function wesnoth.map.replace_both(code)
		local base, overlay = wesnoth.map.split_terrain_code(code)
		if base == '' then -- ^ or ^B
			-- There's no way to find a base to replace with in this case.
			-- Could use the existing base, but that's not really replacing both, is it?
			error('replace_both: no base terrain specified')
		elseif overlay == '' then -- A^
			-- This would normally mean replace base while preserving overlay,
			-- but we actually want to replace base and clear overlay.
			return base
		else
			-- It's already going to replace both, so return unchanged
			return code
		end
	end

	function wesnoth.map.iter_adjacent(map, ...)
		local where, n = wesnoth.map.read_location(...)
		if n == 0 then error('wesnoth.map.iter_adjacent: missing location') end
		local with_borders = select(n + 1, ...)
		local adj = {wesnoth.map.get_adjacent_hexes(where)}
		local i = 0
		return function()
			while i < #adj do
				i = i + 1
				local u, v = adj[i].x, adj[i].y
				if map:on_board(u, v, with_borders) then
					return u, v
				end
			end
			return nil
		end
	end
end

if wesnoth.kernel_type() == "Game Lua Kernel" then
	local hex_mt = {__metatable = 'terrain hex reference'}

	function hex_mt.__index(self, key)
		if key == 'fogged' then
			return self:fogged_for(wesnoth.current.side)
		elseif key == 'shrouded' then
			return self:shrouded_for(wesnoth.current.side)
		elseif key == 'team_label' then
			local label = self:label_for(wesnoth.current.side)
			if label then return label.text end
			return nil
		elseif key == 'global_label' then
			local label = self:label_for(nil)
			if label then return label.text end
			return nil
		elseif key == 'label' then
			return self.team_label or self.global_label
		elseif key == 'terrain' then
			return wesnoth.current.map[self]
		elseif key == 'base_terrain' then
			return self.terrain:split('^')[1]
		elseif key == 'overlay_terrain' then
			return self.terrain:split('^', {remove_empty=false})[2]
		elseif key == 'info' then
			return wesnoth.get_terrain_info(wesnoth.current.map[self])
		elseif key == 'time_of_day' then
			return wesnoth.schedule.get_time_of_day(self)
		elseif key == 'illuminated_time' then
			return wesnoth.schedule.get_illumination(self)
		elseif key == 1 then
			return self.x
		elseif key == 2 then
			return self.y
		elseif #key > 0 and key[0] ~= '_' then
			return hex_mt[key]
		end
	end

	function hex_mt.__newindex(self, key, val)
		if key == 'fogged' then
			self:set_fogged(wesnoth.current.side, val)
		elseif key == 'shrouded' then
			self:set_shrouded(wesnoth.current.side, val)
		elseif key == 'team_label' or key == 'global_label' or key == 'label' then
			local cfg
			if type(val) == 'string' or (type(val) == 'userdata' and getmetatable(val) == 'translatable string') then
				cfg = {x = self.x, y = self.y, text = val}
			else
				cfg = wml.parsed(val)
				cfg.x, cfg.y = self.x, self.y
			end
			if key == 'team_label' then
				cfg.side = wesnoth.current.side
				cfg.team_name = wesnoth.sides[wesnoth.current.side].team_name
			elseif key == 'global_label' then
				cfg.side = 0
				cfg.team_name = nil
			elseif cfg.side == nil and cfg.team_name == nil then
				-- If side or team name explicitly specified, use that, otherwise use current side and no team
				cfg.side = wesnoth.current.side
				cfg.team_name = nil
			end
			wesnoth.map.add_label(cfg)
		elseif key == 'terrain' then
			wesnoth.current.map[self] = val
		elseif key == 'base_terrain' then
			wesnoth.current.map[self] = wesnoth.map.replace_base(val)
		elseif key == 'overlay_terrain' then
			wesnoth.current.map[self] = wesnoth.map.replace_overlay(val)
		elseif key == 1 then
			self.x = val
		elseif key == 2 then
			self.y = val
		elseif key == 'info' or key == 'time_of_day' or key == 'illuminated_time' then
			error(string.format('hex.%s is read-only', key), 1)
		else
			-- If it's not a known key, just set it
			rawset(self, key, val)
		end
	end

	function hex_mt:fogged_for(side)
		return wesnoth.sides.is_fogged(side, self)
	end

	function hex_mt:shrouded_for(side)
		return wesnoth.sides.is_shrouded(side, self)
	end

	function hex_mt:set_shrouded(side, val)
		if val then
			wesnoth.sides.place_shroud(side, {self})
		else
			wesnoth.sides.remove_shroud(side, {self})
		end
	end

	function hex_mt:set_fogged(side, val)
		if val then
			wesnoth.sides.place_fog(side, {self})
		else
			wesnoth.sides.remove_fog(side, {self})
		end
	end

	function hex_mt:label_for(who)
		return wesnoth.map.get_label(self.x, self.y, who)
	end

	function hex_mt:matches(filter)
		return wesnoth.map.matches(self.x, self.y, filter)
	end

	-- Backwards compatibility - length is always 2
	hex_mt.__len = wesnoth.deprecate_api('#location', 'nil', 3, '1.17', function() return 2 end, 'Using the length of a location as a validity test is no longer supported. You should represent an invalid location by nil instead.')

	function wesnoth.map.get(x, y)
		local loc, n = wesnoth.map.read_location(x, y)
		if n == 0 then error('Missing or invalid coordinate') end
		return setmetatable(loc, hex_mt)
	end

	local find_locations = wesnoth.map.find
	function wesnoth.map.find(cfg, ref_unit)
		local hexes = find_locations(cfg, ref_unit)
		for i = 1, #hexes do
			hexes[i] = wesnoth.map.get(hexes[i][1], hexes[i][2])
		end
		return hexes
	end

	wesnoth.terrain_mask = wesnoth.deprecate_api('wesnoth.terrain_mask', 'wesnoth.current.map:terrain_mask', 1, nil, function(...)
		wesnoth.current.map:terrain_mask(...)
	end)
	wesnoth.get_terrain = wesnoth.deprecate_api('wesnoth.get_terrain', 'wesnoth.current.map[loc]', 1, nil, function(x, y)
		local loc = wesnoth.map.read_location(x, y)
		if loc == nil then error('get_terrain: expected location') end
		return wesnoth.current.map[loc]
	end)
	wesnoth.set_terrain = wesnoth.deprecate_api('wesnoth.set_terrain', 'wesnoth.current.map[loc]=', 1, nil, function(...)
		local loc, n = wesnoth.map.read_location(...)
		if n == 0 then error('set_terrain: expected location') end
		local new_ter, mode, replace_if_failed = select(n + 1, ...)
		if new_ter == '' or type(new_ter) ~= 'string' then error('set_terrain: expected terrain string') end
		if replace_if_failed then
			mode = mode or 'both'
			new_ter = wesnoth.map.replace_if_failed(new_ter, mode, true)
		elseif mode == 'both' or mode == 'base' or mode == 'overlay' then
			new_ter = wesnoth.map['replace_' .. mode](new_ter)
		elseif mode ~= nil then
			error('set_terrain: invalid mode')
		end
		wesnoth.current.map[loc] = new_ter
	end)
	wesnoth.get_map_size = wesnoth.deprecate_api('wesnoth.get_map_size', 'wesnoth.current.map.playable_width,playable_height,border_size', 1, nil, function()
		local m = wesnoth.current.map
		return m.playable_width, m.playable_height, m.border_size
	end)
	wesnoth.special_locations = wesnoth.deprecate_api('wesnoth.special_locations', 'wesnoth.current.map.special_locations', 1, nil, setmetatable({}, {
		__index = function(_, k) return wesnoth.current.map.special_locations[k] end,
		__newindex = function(_, k, v) wesnoth.current.map.special_locations[k] = v end,
		__len = function(_)
			local n = 0
			for k,v in pairs(wesnoth.current.map.special_locations) do
				n = n + 1
			end
			return n
		end,
		__pairs = function(_) return pairs(wesnoth.current.map.special_locations) end,
	}), 'Note: the length operator has been removed')

	wesnoth.get_village_owner = wesnoth.deprecate_api('wesnoth.get_village_owner', 'wesnoth.map.get_owner', 1, nil, wesnoth.map.get_owner)
	wesnoth.set_village_owner = wesnoth.deprecate_api('wesnoth.set_village_owner', 'wesnoth.map.set_owner', 1, nil, wesnoth.map.set_owner)
	wesnoth.label = wesnoth.deprecate_api('wesnoth.label', 'wesnoth.map.add_label', 1, nil, wesnoth.map.add_label)
	wesnoth.add_time_area = wesnoth.deprecate_api('wesnoth.add_time_area', 'wesnoth.map.place_area', 1, nil, wesnoth.map.place_area)
	wesnoth.remove_time_area = wesnoth.deprecate_api('wesnoth.remove_time_area', 'wesnoth.map.remove_area', 1, nil, wesnoth.map.remove_area)
	wesnoth.get_locations = wesnoth.deprecate_api('wesnoth.get_locations', 'wesnoth.map.find', 1, nil, wesnoth.map.find)
	wesnoth.get_villages = wesnoth.deprecate_api('wesnoth.get_locations', 'wesnoth.map.find', 1, nil, function(cfg)
		return wesnoth.map.find{gives_income = true, wml.tag["and"](cfg)}
	end)
	wesnoth.match_location = wesnoth.deprecate_api('wesnoth.match_location', 'wesnoth.map.matches', 1, nil, wesnoth.map.matches)
	wesnoth.get_terrain_info = wesnoth.deprecate_api('wesnoth.get_terrain_info', 'wesnoth.terrain_types', 1, nil, function(t) return wesnoth.terrain_types[t] end)
end

if wesnoth.kernel_type() == "Mapgen Lua Kernel" then
	wesnoth.map.filter_tags = {
		terrain = function(terrain)
			return { "terrain", terrain }
		end,
		all =  function(...)
			return { "all", ... }
		end,
		any =  function(...)
			return { "any", ... }
		end,
		none =  function(...)
			return { "none", ... }
		end,
		notall =  function(...)
			return { "notall", ... }
		end,
		adjacent =  function(f, adj, count)
			return { "adjacent",  f, adjacent = adj, count = count }
		end,
		find_in =  function(terrain)
			return { "find_in", terrain }
		end,
		radius =  function(r, f, f_r)
			return { "radius", r, f, filter_radius = f_r}
		end,
		x =  function(terrain)
			return { "x", terrain }
		end,
		y =  function(terrain)
			return { "y", terrain }
		end,
		is_loc = function(loc)
			return f.all(f.x(loc[1]), f.y(loc[2]))
		end
	}

	-- More map module stuff
	wesnoth.create_filter = wesnoth.deprecate_api('wesnoth.create_filter', 'wesnoth.map.filter', 1, nil, wesnoth.map.filter)
	wesnoth.create_map = wesnoth.deprecate_api('wesnoth.create_map', 'wesnoth.map.create', 1, nil, wesnoth.map.create)
	wesnoth.default_generate_height_map = wesnoth.deprecate_api('wesnoth.default_generate_height_map', 'wesnoth.map.generate_height_map', 1, nil, wesnoth.map.generate_height_map)
	wesnoth.generate_default_map = wesnoth.deprecate_api('wesnoth.generate_default_map', 'wesnoth.map.generate', 1, nil, wesnoth.map.generate)
	-- These were originally only on the map metatable, so the deprecated versions also need to be in the map module
	wesnoth.map.get_locations = wesnoth.deprecate_api('map:get_locations', 'map:find', 1, nil, wesnoth.map.find)
	wesnoth.map.get_tiles_radius = wesnoth.deprecate_api('map:get_tiles_radius', 'map:find_in_radius', 1, nil, function(map, locs, filter, radius)
		return wesnoth.map.find_in_radius(map, locs, radius, filter)
	end, 'The filter is now the last parameter, instead of the radius')
end

wesnoth.map.tiles_adjacent = wesnoth.deprecate_api('wesnoth.map.tiles_adjacent', 'wesnoth.map.are_hexes_adjacent', 1, nil, wesnoth.map.are_hexes_adjacent)
wesnoth.map.get_adjacent_tiles = wesnoth.deprecate_api('wesnoth.map.get_adjacent_tiles', 'wesnoth.map.get_adjacent_hexes', 1, nil, wesnoth.map.get_adjacent_hexes)
