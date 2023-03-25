-- helper functions for lua map generation.

f = {
	terrain =  function(terrain)
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
	adjacent =  function(f, ad, count)
		return { "adjacent",  f, adjacent = ad, count = count }
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

function get_locations(t)
	local filter = wesnoth.map.filter(t.filter, t.filter_extra or {})
	return map:find(filter, t.locs)
end

function set_terrain_impl(data)
	local locs = {}
	local nlocs_total = 0
	for i = 1, #data do
		if data[i].filter then
			local f = wesnoth.map.filter(data[i].filter, data[i].known or {})
			locs[i] = map:find(f, data[i].locs)
		else
			locs[i] = data[i].locs
		end
		nlocs_total = nlocs_total + #locs[1]
	end
	local nlocs_changed = 0
	for i = 1, #data do
		local d = data[i]
		local chance = d.per_thousand
		local terrains = d.terrain
		local layer = d.layer or 'both'
		local num_tiles = d.nlocs and math.min(#locs[i], d.nlocs) or #locs[i]
		if d.exact then
			num_tiles = math.ceil(num_tiles * chance / 1000)
			chance = 1000
			mathx.shuffle(locs[i])
		end
		for j = 1, num_tiles do
			local loc = locs[i][j]
			if chance >= 1000 or chance >= mathx.random(1000) then
				map[loc] = wesnoth.map['replace_' .. layer](mathx.random_choice(terrains))
				nlocs_changed = nlocs_changed + 1
			end
		end
	end
end

function set_terrain_simul(cfg)
	local data = {}
	for i, r in ipairs(cfg) do
		r_new = {
			filter = r[2] or r.filter,
			terrain = r[1] or r.terrain,
			locs = r.locs,
			layer = r.layer,
			exact = r.exact ~= false,
			per_thousand = 1000,
			nlocs = r.nlocs,
			known = r.known or r.filter_extra
		}
		if r.percentage then
			r_new.per_thousand = r.percentage * 10
		elseif r.per_thousand then
			r_new.per_thousand = r.per_thousand;
		elseif r.fraction then
			r_new.per_thousand = math.ceil(1000 / r.fraction);
		elseif r.fraction_rand then
			r_new.per_thousand = math.ceil(1000 / mathx.random_choice(r.fraction_rand));
		end
		table.insert(data, r_new)
	end
	set_terrain_impl(data)
end

function set_terrain(a)
	set_terrain_simul({a})
end

function set_map_name(str)
	scenario_data.scenario.name = str
end
