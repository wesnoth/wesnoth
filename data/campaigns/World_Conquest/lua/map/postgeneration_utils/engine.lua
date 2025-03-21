-- helper functions for lua map generation.

f = wesnoth.map.filter_tags

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


--- Similar to multiple set_terrain calls (see below), but it first runs all the
--- filters, and then changes the terrain so that the eariler changes have no
--- effect on the later filters.
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

--- Filter based terrain modification of the global map object.
--- @param a a table suppoting the following keys:
--- a[1] (or a.terrain)       : The terrain code that we want to set terain to.
---                             This can be a comma-seperated list, then a terrain is randomly
---                             chosen from that list (a independent draw for each tile that
---                             maches the filter)
--- a[2] (or a.filter)        : the filter.
--- a.locs          (optional): a list of locations to change. If present only locations
---                             match the filter that are also in that list are considered.
--- a.layer         (optional): change only the background or the foreground of a terrain
--- a.nlocs         (optional): change at most a.nlocs tiles.
--- a.per_thousand  (optional): give every matching tile a change of (a.per_thousand / 1000) to be changed.
--- a.precentage    (optional): give every matching tile a change of (a.precentage / 100) to be changed.
--- a.fraction      (optional): give every matching tile a change of (1 / a.fraction) to be changed.
--- a.fraction_rand (optional): pick a random element from fraction_rand, then same as a.fraction.
--- a.exact         (optional): When a.per_thousand, a.percentage, or a.fraction is used, change exact
---                             number_of_matches  * (a.per_thousand / 1000) files instead of
---                             drawing the chance for each tile to be changed independenly.
--- Example:
--- set_terrain { "Ww",
---   f.terrain("Wo"),
---   per_thousand = 200,
---   exact = true,
--- }
--- Changes Exactly one fifth of the "Wo" terrain to "Ww"
function set_terrain(a)
	set_terrain_simul({a})
end

function set_map_name(str)
	scenario_data.map_name = str
end
