-- postgeneration files contain macros only used in that one. Macros used in several postgenerations are in utilities files.
-- lack of homogenic system in postgeneration rutines is quite intended, to avoid pattern perception in long run from players
-- scenarios need call postgeneration in steps to syncronize with generate bonus, enemy castle expansion and enemy themed events.

function wct_dirt_beachs(rng_range)
	set_terrain { "Ds^Esd",
		f.all(
			f.terrain("Ds"),
			f.adjacent(f.terrain("Ww*,Wo*"))
		),
		fraction_rand = rng_range,
	}
	
end

-- pack of generic tweaks to default generator
function world_conquest_tek_map_rebuild(cave, reef)
	wct_reduce_wall_clusters(cave)
	wct_fill_lava_chasms()
	wct_volcanos()
	wct_expand_snow()
	-- add underwater ground
	set_terrain { "Ww",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.terrain("!,Wo"), nil, "2-6")
		),
		exact = false,
		percentage = reef,
	}
	
	-- add reefs
	set_terrain { "Wwr",
		f.any(
			f.all(
				f.terrain("Wo"),
				f.adjacent(f.terrain("!,Wo"), nil, "2-6")
			),
			f.all(
				f.terrain("Ww"),
				f.radius(3, f.terrain("Wo"))
			)
		),
		exact = false,
		percentage = reef,
	}

	-- replace hills for mushrooms
	-- base amount in map surface
	local r = mathx.random_choice(tostring(total_tiles // 500) .. ".." .. tostring(total_tiles // 250))
	-- just to be sure.
	r = tonumber(r)
	set_terrain { "Hh^Tf",
		f.all(
			f.terrain("Hh,Hh^F*"),
			f.adjacent(f.terrain("A*^*,Ms^*,Ha^*"), nil, 0),
			f.radius(2, f.terrain("Mm,Md,Xu,Mm^Xm,Uu,Uh,Uu^Tf,Ql,Qxu,Mv"))
		),
		nlocs = r,
	}
end

function wct_fill_lava_chasms()
	set_terrain { "Ql",
		f.all(
			f.terrain("Qxu^*"),
			f.radius(999, f.terrain("Ql^*"), f.terrain("Ql^*,Qxu^*"))
		),
		layer = "base",
	}
	
end

function world_conquest_tek_map_dirt(mushrooms)
	-- muddy swamps almost surrounded by sand
	local terrain_to_change = wct_store_possible_muddy_swamps()
	while #terrain_to_change > 0 do
		for i, v in ipairs(terrain_to_change) do
			map[v] = "Sm"
		end
		terrain_to_change = wct_store_possible_muddy_swamps()
	end
	
	wct_randomize_snowed_forest()
	wct_volcanos_dirt()
	-- volcanos dry mountains
	set_terrain { "Md",
		f.all(
			f.terrain("Mm^*"),
			f.radius(2, f.terrain("Mv"))
		),
		layer = "base",
	}
	
	-- lava dry mountains
	set_terrain { "Md",
		f.all(
			f.terrain("Mm*^*"),
			f.radius(1, f.terrain("Ql"))
		),
		layer = "base",
	}
	
	-- some impassible become mushrooms
	set_terrain { mushrooms,
		f.all(
			f.terrain("Mm^Xm,Md^Xm"),
			f.adjacent(f.terrain("Ql,Mv"), nil, 0)
		),
		fraction_rand = "9..11",
	}
	
	wct_dirt_beachs("9..11")
end

function wct_change_map_water(t)
	set_terrain { "Ww" .. t,
		f.terrain("Ww^*"),
		layer = "base",
	}
	set_terrain { "Wwr" .. t,
		f.terrain("Wwr"),
	}
	set_terrain { "Wo" .. t,
		f.terrain("Wo"),
	}
	
end

function wct_map_decorative_docks()
	set_terrain { "Ww^Bw/",
		f.all(
			f.terrain("Ww"),
			f.adjacent(f.all(
				f.terrain("Ds,C*,K*"),
				f.adjacent(f.terrain("G*^V*,D*^V*,H*^V*,C*,K*"))
			), "ne,sw", nil),
			f.adjacent(f.terrain("Ww,Wo,Wwr"), "ne,sw", nil)
		),
		exact = false,
		percentage = 17,
	}
	set_terrain { "Ww^Bw\\",
		f.all(
			f.terrain("Ww"),
			f.adjacent(f.all(
				f.terrain("Ds,C*,K*"),
				f.adjacent(f.terrain("G*^V*,D*^V*,H*^V*,C*,K*"))
			), "nw,se", nil),
			f.adjacent(f.terrain("Ww,Wo,Wwr"), "nw,se", nil),
			f.adjacent(f.terrain("*^B*"), nil, 0)
		),
		exact = false,
		percentage = 18,
	}
	
	-- chance of trash near docks
	set_terrain { "Ds,Ds,Ds,Ds,Ds,Ds^Edt",
		f.all(
			f.terrain("Ds"),
			f.adjacent(f.terrain("*^B*")),
			f.adjacent(f.terrain("C*^*,K*^*,*^V*"))
		),
	}
	set_terrain { "Ds,Ds,Ds,Ds,Ds,Ds,Ds,Ds,Ds,Ds^Edt",
		f.all(
			f.terrain("Ds"),
			f.adjacent(f.terrain("Ds^Edt"))
		),
	}
	
end

function wct_store_possible_flowers(terrain)
	return map:find(f.all(
		f.terrain("Gs,Gg"),
		f.adjacent(f.terrain(terrain)),
		f.adjacent(f.terrain("D*^*,S*^*,Hd"), nil, 0)
	))
	
	
end

function wct_store_possible_map4_castle(value)
	return  map:find(f.all(
		f.terrain("H*^F*"),
		f.adjacent(f.terrain("H*^F*"), nil, 6)
	))
end

function wct_possible_map4_castle(terrain, value)
	local terrain_to_change = wct_store_possible_map4_castle(value)
	while #terrain_to_change > 0 and mathx.random(value + 1) == 1 do
		local loc = terrain_to_change[mathx.random(#terrain_to_change)]
		map[loc] = terrain
		terrain_to_change = wct_store_possible_map4_castle(value)
	end
end

function wct_store_possible_dwarven_castle()
	return map:find(f.all(
		f.terrain("Uh"),
		f.adjacent(f.terrain("Uh"), nil, "5-6")
	))
end

function wct_store_possible_roads(village)
	return map:find(f.all(
		f.terrain("Gg,Gs"),
		f.adjacent(f.all(
			f.terrain(village),
			f.adjacent(f.terrain("R*,Ch*,Kh*"), nil, 0)
		)),
		f.adjacent(f.terrain("R*,Kh*,Ch*"))
	))
	
end

function wct_road_to_village(road, village)
	-- build roads of 1 hex to conect villages
	local terrain_to_change = wct_store_possible_roads(village)
	while #terrain_to_change > 0 do
		local loc = terrain_to_change[mathx.random(#terrain_to_change)]
		map[loc] = road
		terrain_to_change = wct_store_possible_roads(village)
	end
end

-- builds roads using terrain @a terrain
-- for each step we call @get_next 
function wct_iterate_roads_to(get_next, radius, terrain)
	print_time("wct_iterate_roads_to start")
	for r = radius, 1, -1 do
		local locs = get_next(r)
		while #locs > 0 do
			local loc = locs[mathx.random(#locs)]
			map[loc] = terrain
			locs = get_next(r)
		end
	end
	print_time("wct_iterate_roads_to end")
end

-- creates roads, faster than wct_iterate_roads_to
-- creates for each tile that mactches a f_src a road
-- to a tile that macthes @a f_dest of possible,
-- roads have a maximum length of @a radius and can only be
-- placed on tiles that match @a f_dest
function wct_iterate_roads_to_2(f_validpath, f_src, f_dest, terrain_road, radius)
	local src_tiles = map:find(f_src)
	local dest_tiles = map:find(f_dest)
	local filter_path = wesnoth.map.filter(f_validpath)
	local map = _G.map

	local function filter_path_function(x, y)
		local xy_list = { {x,y} }
		local res = #map:find(filter_path, xy_list) > 0
		return res
	end
	-- calculate for each tile its distance to any of the tiles in dest_tiles.
	local distmap = Distmap:create(map.width, map.height)
	distmap:calculate_distances(dest_tiles, radius, filter_path_function)

	for i_src, loc_src in ipairs(src_tiles) do
		-- calculate how far loc_src is away from one of the tiles in dest_tiles.
		local dist = nil
		for i_ad, loc_ad in ipairs(distmap:adjacent_tiles(loc_src)) do
			local dist_ad = distmap:get(loc_ad)
			if (not dist) or (dist_ad and dist_ad < dist) then
				dist = dist_ad
			end
		end
		-- the distant of loc_src to dest_tiles is one higher than the distance of the closest neighbor
		dist = (dist or 999) + 1
		if dist <= radius then
			local path = { }
			local loc = loc_src
			while dist > 1 do
				local next_locs = {}
				for i_ad, loc_ad in ipairs(distmap:adjacent_tiles(loc)) do
					local dist_ad = distmap:get(loc_ad)
					if (dist_ad or 999) < dist then
						next_locs[#next_locs + 1] = loc_ad
					end
					if dist_ad and map[loc_ad] == terrain_road then
						--we merged with another path. 
						goto path_found
					end
				end
				path[#path + 1] = next_locs[mathx.random(#next_locs)]
				loc = path[#path]
				dist = distmap:get(loc)
			end
			::path_found::
			wesnoth.log("debug", "generated path: " .. debug_wml(path))
			for i, ploc in ipairs(path) do
				map[ploc] = terrain_road
			end
		end
	end
end

function wct_iterate_roads_to_ex(t)	
	print_time("wct_iterate_roads_to_ex start")
	wct_iterate_roads_to_2(t.f_validpath, t.f_src, t.f_dest, t.terrain_road, t.radius)
	print_time("wct_iterate_roads_to_ex end")
end
function wct_break_walls(wall, terrain)
	
	local terrain_to_change = wct_store_broken_wall_candidates(wall)
	while #terrain_to_change > 0 do
		local loc = terrain_to_change[mathx.random(#terrain_to_change)]
		map[loc] = mathx.random_choice(terrain)
		terrain_to_change = wct_store_broken_wall_candidates(wall)
	end
end

function wct_store_broken_wall_candidates(wall)
	return map:find(f.all(
		f.terrain(wall),
		f.adjacent(f.terrain("M*^Xm,X*"), nil, "2-6"),
		f.adjacent(f.terrain("Mv"), nil, 0)
	))
	
end

function wct_store_possible_muddy_swamps()
	return map:find(f.all(
		f.terrain("Ss"),
		f.adjacent(f.terrain("D*^*,Hd,Sm,Rd"), nil, "5-6")
	))
	
end

function wct_map_reduce_castle_expanding_recruit(castle, terrain)
	-- terrain @a castle is a castle terrain used for map and not for keeps
	-- so we have to maek sure it doesnt appear near keeps.
	set_terrain { terrain,
		f.all(
			f.adjacent(f.terrain("Ch,Cha,Kh*^*")),
			f.terrain(castle)
		),
	}
	
end

function wct_map_cave_path_to(terrain)
	set_terrain { terrain,
		f.terrain("Ur"),
	}
	
end
