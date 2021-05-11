
-- The invest dialog spawns items below the keeps, so we have to 
-- make sure the tiles below keeps are passable for all units
function wct_fix_impassible_item_spawn()

	local player_keeps = { }
	for i = 1, _G.scenario_data.nhumanplayers do
		table.insert( player_keeps, map.special_locations[tostring(i)])
	end

	set_terrain { "Mm",
		f.all(
			f.terrain("*^X*,X*^*"),
			f.adjacent(f.find_in("player_keep"), "n", nil)
		),
		filter_extra = { player_keep = player_keeps }
	}
	set_terrain { "Ww",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.find_in("player_keep"), "n", nil)
		),
		filter_extra = { player_keep = player_keeps }
	}
	set_terrain { "Wwt",
		f.all(
			f.terrain("Wot"),
			f.adjacent(f.find_in("player_keep"), "n", nil)
		),
		filter_extra = { player_keep = player_keeps }
	}
	set_terrain { "Wwg",
		f.all(
			f.terrain("Wog"),
			f.adjacent(f.find_in("player_keep"), "n", nil)
		),
		filter_extra = { player_keep = player_keeps }
	}
end


function wct_volcanos()
	set_terrain { "Mv",
		f.all(
			f.terrain("Ql,Qlf"),
			f.adjacent(f.terrain("M*,M*^Xm,X*"), "se,s,sw", 3)
		),
	}
	set_terrain { "Md^Xm",
		f.all(
			f.terrain("X*,M*^Xm"),
			f.adjacent(f.terrain("Mv"), "n,ne,nw", nil)
		),
	}
	set_terrain { "Md",
		f.all(
			f.terrain("Ms,Mm"),
			f.adjacent(f.terrain("Mv"), "n,ne,nw", nil)
		),
	}
	local terrain_to_change = map:find(f.terrain("Mv"))
	--todo figure out whether there is a differnce between many sound_source and on with a hige x,y list.
	for volcano_i, volcano_loc in ipairs(terrain_to_change) do
		table.insert(prestart_event, wml.tag.sound_source {
			id="volcano" .. tostring(volcano_i),
			sounds="rumble.ogg",
			delay=550000,
			chance=1,
			x=volcano_loc[1],
			y=volcano_loc[2],
			full_range=5,
			fade_range=5,
			loop=0,
		})
	end
end

function wct_volcanos_dirt()
	set_terrain { "*^Dr",
		f.all(
			f.terrain("Hh,Hd,Hhd"),
			f.radius(3, f.terrain("Mv"))
		),
		fraction = 3,
		layer = "overlay",
	}
	set_terrain { "Dd^Dc",
		f.all(
			f.terrain("Ds,Dd"),
			f.radius(4, f.terrain("Mv"))
		),
		fraction = 2,
	}
end

function wct_reduce_wall_clusters(cave_terrain)
	set_terrain { cave_terrain,
		f.all(
			f.terrain("Xu"),
			f.adjacent(f.terrain("Xu,M*^Xm"), nil, "3-6")
		),
	}

end

function wct_castle_expansion_side(side_num)
	wesnoth.log("debug", "expanding castle" .. side_num)
	local n_tiles_wanted = scenario_data.nplayers + 1
	local keep_loc = map.special_locations[tostring(side_num)]

	if keep_loc == nil then
		return
	end
	local castle = map:find_in_radius({keep_loc}, 1, wesnoth.map.filter(f.terrain("C*,K*")))
	local keep_area = map:find_in_radius({keep_loc}, 2, wesnoth.map.filter(f.all()))

	local candidates = get_locations {
		filter = f.all(
			f.none(f.terrain("C*,K*")),
			f.adjacent(f.find_in("castle"))
		),
		locs = keep_area,
		filter_extra = { castle = castle }
	}

	local function filter_candidates(t)
		local candidates_fav = get_locations {
			filter = t.filter,
			filter_extra = t.filter_extra,
			locs = candidates,
		}
		if #candidates_fav >= n_tiles_wanted then
			candidates = candidates_fav
		end
	end

	filter_candidates {
		filter = f.none(f.radius(1, f.terrain("Mv")))
	}

	filter_candidates {
		filter = f.none(f.terrain("C*,K*,X*,*^Xm,Ww,Wwt,Wwg,Wo*,Wwr*,*^V*"))
	}

	if #candidates < n_tiles_wanted then
		wesnoth.log("warn", "Too few tiles in castle expansion for side " .. side_num .. ", wanted: " .. n_tiles_wanted .. " but we got only " .. #candidates)
		n_tiles_wanted = #candidates
	end
	mathx.shuffle(candidates)
	for i = 1, n_tiles_wanted do
		map[candidates[i]] = "Ch"
	end
end

function wct_enemy_castle_expansion()
	local n_player_sides = _G.scenario_data.nhumanplayers
	local n_total_sides = _G.scenario_data.nplayers
	for side_num = n_player_sides + 1, n_total_sides do
		wct_castle_expansion_side(side_num)
	end
end

function get_oceanic()
	local f_is_border = f.any(
		f.x("1," .. tostring(map.width - 1)),
		f.y("1," .. tostring(map.height - 1))
	)
	local water_border_tiles = map:find(f.all(f_is_border, f.terrain("Wo*")))
	local filter_radius = wesnoth.map.filter(f.all(
		f.terrain("W*^V*,Wwr*,Ww,Wwg,Wwt,Wo*"),
		--ignore rivers
		f.adjacent(f.terrain("!,W*^*,S*^*,D*^*,Ai"), nil, "0-3")
	))
	return map:find_in_radius(water_border_tiles, 999, filter_radius)
end
