-- Maritime
local images = {
	dock_ship = "misc/blank-hex.png~BLIT(units/transport/boat.png~CROP(0,15,72,57))",
	dock_ship_2 = "misc/blank-hex.png~BLIT(units/transport/boat.png~FL()~CROP(0,15,72,57))"
}

function get_possible_maritime_bridge()
	return {
		{
			type = "Bsb|",
			locs = map:find(f.all(
				f.terrain("Ww"),
				f.adjacent(f.terrain("Chw"), "s,n", nil),
				f.adjacent(f.terrain("Ch,Kh"), "s,n", nil),
				f.adjacent(f.terrain("*^B*"), nil, 0)
			))
		},
		{
			type = "Bsb\\",
			locs = map:find(f.all(
				f.terrain("Ww"),
				f.adjacent(f.terrain("Chw"), "se,nw", nil),
				f.adjacent(f.terrain("Ch,Kh"), "se,nw", nil),
				f.adjacent(f.terrain("*^B*"), nil, 0)
			))
		},
		{
			type = "Bsb/",
			locs = map:find(f.all(
				f.terrain("Ww"),
				f.adjacent(f.terrain("Chw"), "sw,ne", nil),
				f.adjacent(f.terrain("Ch,Kh"), "sw,ne", nil),
				f.adjacent(f.terrain("*^B*"), nil, 0)
			))
		}
	}

end

function wct_maritime_bridges()

	local pb = get_possible_maritime_bridge()
	while #pb[1].locs > 0 or #pb[2].locs > 0 or #pb[3].locs > 0 do
		pb = functional.filter(pb, function(t) return #t.locs >0 end)
		local sel = pb[mathx.random(#pb)]
		local loc = sel.locs[mathx.random(#sel.locs)]
		map[loc] = "Ww^" .. sel.type
		pb = get_possible_maritime_bridge()
	end
end

function roads_to_dock(radius)
	wct_iterate_roads_to_ex {
		terrain_road = "Rp",
		f_validpath = f.terrain("!,W*^*"),
		f_src = f.terrain("Iwr^Vl"),
		f_dest = f.terrain("Ch*,Kh*^*,Re"),
		radius = radius
	}
	--old implementation: wct_iterate_roads_to(wct_roads_to_dock, 3, "Rp")
end

function wct_roads_to_dock(radius)
	return map:find(f.all(
		f.terrain("!,W*^*"),
		f.adjacent(f.all(
			f.terrain("Iwr^Vl,Rp"),
			f.adjacent(f.all(
				f.terrain("Rp"),
				f.radius(radius, f.terrain("Ch*,Kh*^*,Re"), f.terrain("!,W*^*"))
			), nil, 0),
			f.none(
				f.radius(radius, f.terrain("Ch*,Kh*^*,Re"), f.terrain("!,W*^*"))
			)
		)),
		f.radius(radius, f.terrain("Ch*,Kh*^*,Re"), f.terrain("!,W*^*"))
	))
end

function roads_to_river(radius)
	wct_iterate_roads_to_ex {
		terrain_road = "Rp",
		f_validpath = f.terrain("!,W*^*"),
		f_src = f.terrain("*^Vhc"),
		f_dest = f.terrain("Ch*,Kh*^*,Re"),
		radius = radius
	}
	--old implementation: wct_iterate_roads_to(wct_roads_to_river, 3, "Rp")
end

-- todo: the old code used
-- `wct_iterate_roads_to(wct_roads_to_river, 3, "Rp")`
-- but the new code uses roads_to_river(4) i guess
-- wct_iterate_roads_to_ex defeind radiosu differently thatn
-- wct_iterate_roads_to ?
-- anyway leavong this function in as a sekeltong on how
-- wct_iterate_roads_to worked. in particular if we want to convert the remaining cases to
-- wct_iterate_roads_to_ex
function wct_roads_to_river(radius)

	local f_valid_path_tiles = f.terrain("!,W*^*")
	local f_dest = f.terrain("Ch*,Kh*^*,Re")
	local f_src = f.terrain("*^Vhc")
	local f_path_taken = f.terrain("Rp")

	return map:find(f.all(
		f_valid_path_tiles,
		-- adjacent to open ends or startign points.
		f.adjacent(f.all(
			-- tiles in your source or tiles that we alrady started building a road on.
			f.any(f_src, f_path_taken),
			-- exculde road tiles that are not open ends.
			f.adjacent(f.all(
				f_path_taken,
				f.radius(radius, f_dest, f_valid_path_tiles)
			), nil, 0),
			-- exculde open ends tha are alreaqdy close enough to the target for this building step.
			f.none(
				f.radius(radius, f_dest, f_valid_path_tiles)
			)
		)),
		-- tiles where we can actually get coloser to the target-
		f.radius(radius, f_dest, f_valid_path_tiles)
	))
end

function world_conquest_tek_map_decoration_6b()
	-- rich terrain around rivers
	set_terrain { "*^Vhc",
		f.all(
			f.terrain("H*^V*"),
			f.adjacent(f.terrain("Ww"))
		),
		layer = "overlay",
	}
	set_terrain { "Rp^Vhc",
		f.all(
			f.terrain("G*^V*"),
			f.adjacent(f.terrain("Ww"))
		),
	}
	set_terrain { "Gg",
		f.all(
			f.terrain("G*^*"),
			f.adjacent(f.terrain("Ww"))
		),
		layer = "base",
	}
	set_terrain { "Gg",
		f.all(
			f.terrain("Gs^*,Gd^*"),
			f.radius(2, f.terrain("Ww"))
		),
		fraction = 3,
		layer = "base",
	}
	set_terrain { "Gg",
		f.all(
			f.terrain("Gs^*,Gd^*"),
			f.radius(3, f.terrain("Ww"))
		),
		fraction = 3,
		layer = "base",
	}
	set_terrain { "Gs^*",
	--set_terrain { "Gs*^*",
		f.all(
			f.terrain("Gd*^*"),
			f.radius(3, f.terrain("Ww"))
		),
		layer = "base",
	}

	-- generate big docks villages
	set_terrain { "Iwr^Vl",
		f.all(
			f.adjacent(f.terrain("W*^*"), nil, "1-5"),
			f.adjacent(f.terrain("Wog,Wwg")),
			f.terrain("*^V*"),
			f.radius(4, f.terrain("Ch,Kh*^*"))
		),
	}

	roads_to_dock(4)
	roads_to_river(4)

	if #map:find(f.terrain("Iwr^Vl")) == 0 then
		local locs = map:find(f.all(
			f.terrain("*^V*"),
			f.adjacent(f.terrain("W*^*"), nil, "2-5"),
			f.adjacent(f.terrain("Wog,Wwg"))
		))
		loc = locs[mathx.random(#locs)];
		map[loc] = "Iwr^Vl"
	end

	set_terrain { "Wwg,Iwr,Wwg^Bw\\,Wwg^Bw\\,Wwg^Bw\\,Wwg^Bw\\",
		f.all(
			f.terrain("Wog,Wwg"),
			f.adjacent(f.terrain("Wog,Wwg"), "se,nw", nil),
			f.adjacent(f.terrain("Iwr^Vl"), "se,nw", nil)
		),
	}
	set_terrain { "Wwg,Iwr,Wwg^Bw/,Wwg^Bw/,Wwg^Bw/,Wwg^Bw/",
		f.all(
			f.terrain("Wog,Wwg"),
			f.adjacent(f.terrain("Wog,Wwg"), "sw,ne", nil),
			f.adjacent(f.terrain("Iwr^Vl"), "sw,ne", nil)
		),
	}
	set_terrain { "Wwg,Iwr,Wwg^Bw|,Wwg^Bw|,Wwg^Bw|,Wwg^Bw|",
		f.all(
			f.terrain("Wog,Wwg"),
			f.adjacent(f.terrain("Wog,Wwg"), "s,n", nil),
			f.adjacent(f.terrain("Iwr^Vl"), "s,n", nil)
		),
	}
	set_terrain { "Wwg,Wwg^Bw\\",
		f.all(
			f.terrain("Wog,Wwg"),
			f.adjacent(f.terrain("Wog,Wwg"), "se,nw", nil),
			f.adjacent(f.terrain("Iwr"), "se,nw", nil)
		),
	}
	set_terrain { "Wwg,Wwg^Bw/",
		f.all(
			f.terrain("Wog,Wwg"),
			f.adjacent(f.terrain("Wog,Wwg"), "sw,ne", nil),
			f.adjacent(f.terrain("Iwr"), "sw,ne", nil)
		),
	}
	set_terrain { "Wwg,Wwg^Bw|",
		f.all(
			f.terrain("Wog,Wwg"),
			f.adjacent(f.terrain("Wog,Wwg"), "s,n", nil),
			f.adjacent(f.terrain("Iwr"), "s,n", nil)
		),
	}
	local locs = map:find(f.terrain("Iwr"))
	for ship_i, ship_loc in ipairs(locs) do
		if mathx.random(2) == 1 then
			table.insert(prestart_event, wml.tag.item {
				x = ship_loc[1],
				y = ship_loc[2],
				image = images.dock_ship,
				name = "wc2_dock_ship",
			})
		else
			table.insert(prestart_event, wml.tag.item {
				x = ship_loc[1],
				y = ship_loc[2],
				image = images.dock_ship_2,
				name = "wc2_dock_ship",
			})
		end
	end
	set_terrain { "Ds^Edt",
		f.all(
			f.terrain("Ds"),
			f.adjacent(f.terrain("Iwr"))
		),
	}
	set_terrain { "Ds^Edt,Ds",
		f.all(
			f.terrain("Ds"),
			f.adjacent(f.terrain("Iwr"))
		),
	}
	set_terrain { "Ds^Edt",
		f.all(
			f.terrain("W*"),
			f.adjacent(f.terrain("Iwr")),
			f.adjacent(f.terrain("W*^*"), nil, 0)
		),
	}

	-- some villages tweaks
	set_terrain { "*^Vd",
		f.terrain("M*^V*"),
		fraction = 2,
		layer = "overlay",
	}
	set_terrain { "*^Vo",
		f.terrain("H*^Vhh"),
		fraction = 2,
		layer = "overlay",
	}
	set_terrain { "*^Ve",
		f.all(
			f.terrain("G*^Vh"),
			f.adjacent(f.terrain("G*^F*"))
		),
		layer = "overlay",
	}
	set_terrain { "*^Vht",
		f.all(
			f.terrain("G*^Vh"),
			f.adjacent(f.terrain("R*^*,C*^*,K*^*"), nil, 0),
			f.adjacent(f.terrain("Ss"), nil, "2-6")
		),
		layer = "overlay",
	}

	-- fix badlooking dunes
	set_terrain { "Hhd",
		f.all(
			f.terrain("Hd"),
			f.adjacent(f.terrain("D*^*,Rd^*"), nil, 0)
		),
	}

	-- expnad dock road
	set_terrain { "Rp",
		f.all(
			f.terrain("G*,Ds"),
			f.adjacent(f.terrain("*^Vl"))
		),
	}

	-- contruction dirt near beach roads
	set_terrain { "Hd,Ds^Dr",
		f.all(
			f.terrain("Ds"),
			f.adjacent(f.terrain("W*^*,G*^*,S*^*"), nil, 0),
			f.adjacent(f.terrain("Rp"))
		),
	}

	-- rebuild some swamps far from rivers
	set_terrain { "Gs,Ds,Ds",
		f.all(
			f.terrain("Ss"),
			f.none(
				f.radius(6, f.terrain("Ww,Wwf"))
			)
		),
		fraction = 8,
	}
	set_terrain { "Gs^Fp,Hh^Fp,Hh,Mm,Gs^Fp,Ss^Tf,Ss^Tf,Ss^Tf",
		f.all(
			f.terrain("Ss"),
			f.none(
				f.radius(6, f.terrain("Ww,Wwf"))
			)
		),
		fraction = 8,
	}

	-- some mushrooms on hills near river or caves
	set_terrain { "Hh^Tf",
		f.all(
			f.terrain("Hh,Hh^F*"),
			f.radius(5, f.terrain("Ww,Wwf,U*^*"))
		),
		fraction = 14,
	}

	-- reefs
	set_terrain { "Wwrg",
		f.all(
			f.terrain("Wog"),
			f.adjacent(f.terrain("Wwg")),
			f.none(
				f.radius(7, f.terrain("*^Vl"))
			)
		),
		fraction = 6,
	}

	-- chance of expand rivers into sea
	local r = tonumber(mathx.random_choice("0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,2,2,3"))
	for i = 1 , r do
		local terrain_to_change = map:find(f.all(
			f.terrain("Wog,Wwg,Wwrg"),
			f.adjacent(f.terrain("Ww,Wwf,Wo"))
		))
		set_terrain { "Ww",
			f.terrain("Wwg"),
			locs = terrain_to_change
		}
		set_terrain { "Wo",
			f.terrain("Wog"),
			locs = terrain_to_change
		}
		set_terrain { "Wwr",
			f.terrain("Wwrg"),
			locs = terrain_to_change
		}

	end
	wct_map_reduce_castle_expanding_recruit("Chw", "Wwf")
	-- soft castle towards river defense
	set_terrain { "Chw",
		f.all(
			f.terrain("Ww"),
			f.adjacent(f.terrain("C*,K*"), nil, 0),
			f.adjacent(f.terrain("W*^*"), nil, "2-6"),
			f.adjacent(f.all(
				f.terrain("Ww"),
				f.adjacent(f.terrain("Ch,Kh"))
			))
		),
		exact = false,
		percentage = 83,
	}

	wct_maritime_bridges()
end

function world_conquest_tek_map_repaint_6b()
	wct_reduce_wall_clusters("Uu,Uu^Tf,Uh,Uu^Tf,Uu,Uu^Tf,Uh,Ql,Qxu,Xu,Uu,Ur")
	wct_fill_lava_chasms()
	wct_volcanos()
	world_conquest_tek_map_decoration_6b()
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

	-- dirt beachs far from docks
	set_terrain { "Ds^Esd",
		f.all(
			f.terrain("Ds"),
			f.adjacent(f.terrain("Wwg,Wog")),
			f.none(
				f.radius(6, f.terrain("*^Vl"))
			)
		),
		fraction = 10,
	}

	wct_map_cave_path_to("Rb")
	wct_noise_snow_to("Rb")
end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"Maritime")
	wct_enemy_castle_expansion()
	world_conquest_tek_map_noise_maritime()
	world_conquest_tek_map_repaint_6b()
end
