function connected_components(locs)
	local l_set = {}
	local color_i = 1
	local w = 1000

	local function loc_to_index(loc)
		return loc[1] + 1 + loc[2] * w
	end

	for i, v in ipairs(locs) do
		l_set[loc_to_index(v)] = true
	end

	for i, loc in pairs(locs) do
		local loc_i = loc_to_index(loc)
		if l_set[loc_i] == true then
			local todo = { loc }
			l_set[loc_i] = color_i
			while #todo ~= 0 do
				for i, loc_ad in ipairs({wesnoth.map.get_adjacent_hexes(todo[1][1], todo[1][2])}) do
					local loc_ad_i = loc_to_index(loc_ad)
					if l_set[loc_ad_i] then
						if l_set[loc_ad_i] == true then
							l_set[loc_ad_i] = color_i
							todo[#todo + 1] = loc_ad
						else
							assert(l_set[loc_ad_i] == color_i)
						end
					end
				end
				table.remove(todo, 1)
			end
			color_i = color_i + 1
		end
	end
	local res = {}
	for i = 1, color_i - 1 do
		res[i] = {}
	end
	for i, loc in ipairs(locs) do
		local t = res[l_set[loc_to_index(loc)]]
		t[#t + 1] = loc
	end
	return res
end


function repaint(map_data)
	--fixme: is the the correyt directory?
	local heights = wesnoth.dofile("./../postgeneration_utils/wild_zones.lua")

	-- store and remove villages
	local villages = map:find(f.terrain("*^Vh"))
	set_terrain { "*",
		f.terrain("*^Vh"),
		layer = "overlay",
	}
	wild_store_cave_zone(map_data)

	wild_zones_store(heights)

	-- fix ocean water, add reefs
	set_terrain { "Wwrg",
		f.all(
			f.terrain("Wog"),
			f.adjacent(f.terrain("!,Wog")),
			f.adjacent(f.terrain("W*^V*"), nil, 0)
		),
		percentage = 5,
		exact = false,
	}

	if mathx.random(4) == 1 then
		set_terrain { "Ww",
			f.terrain("Wwg^*"),
			layer = "base",
		}
		set_terrain { "Wwr",
			f.terrain("Wwrg"),
		}
		set_terrain { "Wo",
			f.terrain("Wog"),
		}

	end

	wild_zones_replace(heights)

	-- remove roads
	set_terrain { "Ur",
		f.all(
			f.terrain("Rr,Rrc"),
			f.adjacent(f.terrain("U*^*,Q*^*"), nil, "2-6")
		),
	}
	set_terrain { "Gg",
		f.terrain("Rrc"),
	}
	set_terrain { "Gs",
		f.terrain("Rr"),
	}

	-- cave walls
	set_terrain { "Xu",
		f.all(
			f.adjacent(f.terrain("U*^*,Q*^*,X*"), nil, "3-6"),
			f.terrain("*^Xm")
		),
	}
	set_terrain { "Xu",
		f.all(
			f.terrain("*^Xm"),
			f.adjacent(f.terrain("U*^*,Q*^*,X*"), nil, 2)
		),
		percentage = 50,
		exact = false,
	}
	set_terrain { "Xu",
		f.all(
			f.terrain("Uu,Uh,Uu^Tf,Uh^Tf"),
			f.adjacent(f.terrain("G*^*,H*^*")),
			f.adjacent(f.terrain("*^Xm"), nil, 0)
		),
		percentage = 50,
		exact = false,
	}

	-- wood castles
	set_terrain { "Ce",
		f.terrain("Ch"),
	}


	wct_fill_lava_chasms()
	wct_volcanos()
	wct_volcanos_dirt()

	-- restore villages
	set_terrain { "*^Vo",
		f.all(
			f.find_in("villages"),
			f.none(
				f.terrain("Mv")
			)
		),
		filter_extra = { villages = villages },
		layer = "overlay",
	}
	set_terrain { "*^Voa",
		f.terrain("A*^Vo,Ha^Vo,Ms^Vo"),
		layer = "overlay",
	}
	set_terrain { "Uh^Vud",
		f.terrain("Uh^Vo"),
	}
	set_terrain { "*^Vhs",
		f.terrain("S*^Vo"),
		layer = "overlay",
	}
	set_terrain { "Ur^Vu",
		f.terrain("Q*^Vo,X*^Vo"),
	}
	set_terrain { "*^Vd",
		f.terrain("Dd^Vo,Hd^Vo,Hhd^Vo,Md^Vo"),
		layer = "overlay",
	}
	set_terrain { "*^Ve",
		f.any(
			f.all(
				f.terrain("Gg^Vo,Hh^Vo"),
				f.adjacent(f.terrain("*^F*"), nil, "2-6")
			),
			f.all(
				f.terrain("Gll^V*"),
				f.adjacent(f.terrain("Gll^Tf"))
			)
		),
		layer = "overlay",
	}
	set_terrain { "Wwf^Vht",
		f.all(
			f.adjacent(f.terrain("A*^*,Ha^*,Ms^*"), nil, 0),
			f.terrain("Wwf^Vo")
		),
	}
	set_terrain { "Ss^Vhs",
		f.terrain("Wwf^Vo"),
	}
	set_terrain { "Ww^Vm",
		f.terrain("W*^Vo"),
	}
	set_terrain { "*^Vo,*^Vd,*^Vc,*^Vct",
		f.terrain("Gs^Vo,Ds^Vo"),
		layer = "overlay",
		fraction = 1,
	}
	set_terrain { "Uu^Vo,Uu^Vu,Uu^Vud,Uu^Vud,Uu^Vud",
		f.terrain("Uu^Vo"),
	}
	set_terrain { "Mm^Vo,Mm^Vu,Mm^Vd",
		f.terrain("Mm^Vo"),
	}
	set_terrain { "Hh^Vo,Hh^Vd",
		f.all(
			f.terrain("Hh^Vo"),
			f.adjacent(f.terrain("D*^*,Gs^*,Md^*"))
		),
	}

end

function wild_zones_store(heights)
	for i_height, height in ipairs(heights) do
		for i_temp, temp in ipairs(height) do
			-- oldname: 'regions'
			temp.all_locs = map:find(f.terrain(temp.terrain))
			-- oldname: 'zone[$zone_i].loc'
			temp.zones = connected_components(temp.all_locs)
		end
	end
end

function handle_single_zone(locs, commands)
	--insert tag
	for i, command in ipairs(commands) do
		if type(command) == "table" then
			local args = shallow_copy(command)
			args.locs = locs
			set_terrain(args)
		elseif type(command) == "function" then
			command(locs)
		end
	end
end

function wild_zones_replace(heights)
	for i_height, height in ipairs(heights) do
		for i_temp, temp in ipairs(height) do
			handle_single_zone(temp.all_locs, temp[1].default)
			for zone_i, zone in ipairs(temp.zones) do
				local wild_dice = mathx.random(100)
				for chance_i, chance in ipairs(temp[1].chances) do
					if wild_dice <= chance.value then
						handle_single_zone(zone, chance.command)
						goto end_zone
					else
						wild_dice = wild_dice - chance.value
					end
				end
				::end_zone::
			end
		end
	end
end

-- to place right image in bonus points
function wild_store_cave_zone(map_data)
	map_data.road_in_cave = map:find(f.terrain("X*^*"))
end

function wild_store_roads_in_cave_zone(map_data)
	map_data.road_in_cave = map:find(f.terrain("R*,Ur"), map_data.road_in_cave)
end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"Wild")
	local map_data = {}
	wct_enemy_castle_expansion()
	repaint(map_data)
	wild_store_roads_in_cave_zone(map_data)
	-- WORLD_CONQUEST_TEK_BONUS_POINTS uses map_data.road_in_cave
	world_conquest_tek_bonus_points("wild")
end
