-- Industrial
-- TODO: this is somwwhta slow, maybe it because it used wct_iterate_road_to ?
local function wct_conect_factory_rails()
	local rails_conected = map:find(f.all(
		f.terrain("*^Br*"),
		f.adjacent(f.terrain("*^Vhh"))
	))
	while #map:find(wesnoth.map.filter(
		f.all(
			f.terrain("*^Br*"),
			f.adjacent(f.find_in("rails_conected")),
			f.none(
				f.find_in("rails_conected")
			)
		),
		{ rails_conected = rails_conected }
	)) > 0
	do
		set_terrain { "*^Br|",
			f.all(
				f.adjacent(f.find_in("rails_conected"), "n,s", nil),
				f.none(
					f.find_in("rails_conected")
				),
				f.terrain("*^Br*")
			),
			filter_extra = { rails_conected = rails_conected },
			layer = "overlay",
		}
		set_terrain { "*^Br\\",
			f.all(
				f.adjacent(f.find_in("rails_conected"), "nw,se", nil),
				f.none(
					f.find_in("rails_conected")
				),
				f.none(
					f.adjacent(f.terrain("*^Br\\"), "ne,sw", nil)
				),
				f.terrain("*^Br*")
			),
			filter_extra = { rails_conected = rails_conected },
			layer = "overlay",
		}
		set_terrain { "*^Br/",
			f.all(
				f.adjacent(f.find_in("rails_conected"), "ne,sw", nil),
				f.none(
					f.find_in("rails_conected")
				),
				f.none(
					f.adjacent(f.terrain("*^Br/"), "nw,se", nil)
				),
				f.terrain("*^Br*")
			),
			filter_extra = { rails_conected = rails_conected },
			layer = "overlay",
		}
		rails_conected = map:find(wesnoth.map.filter(
			f.all(
				f.terrain("*^Br*"),
				f.radius(1, f.find_in("rails_conected"))
			),
			{ rails_conected = rails_conected }
		))
	end
	rails_conected = nil
	set_terrain { "*^Br|",
		f.all(
			f.terrain("*^Br\\,*^Br/"),
			f.adjacent(f.terrain("*^Br\\,*^Br/"), "n,s", nil)
		),
		layer = "overlay",
	}
	set_terrain { "*^Br/",
		f.all(
			f.terrain("*^Br|"),
			f.adjacent(f.terrain("*^Br|"), "ne", nil),
			f.adjacent(f.terrain("*^Br|"), "n,s", "0-1"),
			f.adjacent(f.terrain("*^Vhh"), "n,s", 0)
		),
		layer = "overlay",
	}
	set_terrain { "*^Br\\",
		f.all(
			f.terrain("*^Br|"),
			f.adjacent(f.terrain("*^Br|"), "nw", nil),
			f.adjacent(f.terrain("*^Br|"), "n,s", "0-1"),
			f.adjacent(f.terrain("*^Vhh"), "n,s", 0)
		),
		layer = "overlay",
	}

end

local function wct_store_possible_dirty_delta()
	return map:find(f.all(
		f.terrain("Wwg,Sm"),
		f.adjacent(f.terrain("Wwg^*,Sm^*"), nil, "5-6")
	))
end

local function wct_dirty_deltas()

	local terrain_to_change = wct_store_possible_dirty_delta()
	while #terrain_to_change > 0 do
		local loc = 1 -- todo: maybe use  terrain_to_change[mathx.random(#terrain_to_change)]
		local ter = mathx.random_choice("Gs,Hh^Tf,Cud,Gs^Tf,Gs,Hh,Ds^Edt,Ds,Hh^Fmf,Gs,Gs^Fmf")
		map[loc] = ter
		terrain_to_change = wct_store_possible_dirty_delta()
	end
end

local function wct_store_possible_ford_delta()
	return map:find(f.all(
		f.terrain("Wwf"),
		f.adjacent(f.terrain("W*^*"), nil, "5-6")
	))

end

local function wct_ford_deltas()

	local terrain_to_change = wct_store_possible_ford_delta()
	while #terrain_to_change > 0 do
		local loc = terrain_to_change[1]-- todo: maybe use errain_to_change[mathx.random(#terrain_to_change)]
		local ter = mathx.random_choice("Gg,Gg^Efm,Mm,Gg^Fet,Gg,Mm,Gg")
		map[loc] = ter
		terrain_to_change = wct_store_possible_ford_delta()
	end
end


local function wct_rails_to_industrial_keep(radius)
	-- "Cud^Br|"
	return map:find(f.all(
		f.terrain("C*"),
		f.adjacent(f.all(
			f.terrain("*^Br*,*^Vhh"),
			f.adjacent(f.all(
				f.terrain("*^Br*,*^Vhh"),
				f.radius(radius, f.terrain("K*^*"), f.terrain("C*,*^Br*,*^Vhh"))
			), nil, 0),
			f.none(
				f.radius(radius, f.terrain("K*^*"), f.terrain("C*,*^Br*,*^Vhh"))
			)
		)),
		f.radius(radius, f.terrain("K*^*"), f.terrain("C*,*^Br*,*^Vhh"))
	))
end

local function wct_roads_to_industrial_village(radius)
	-- "Rb"
	return map:find(f.all(
		f.terrain("!,W*^*,*^V*,*^Bcx*,Urb,C*,K*^*,R*"),
		f.adjacent(f.all(
			f.terrain("*^Ve,*^Vl,Rb"),
			f.adjacent(f.all(
				f.terrain("Rb"),
				f.radius(radius, f.terrain("C*,K*^*,Urb^*,Rr*^*,*^Vhh,*^Vhc"), f.terrain("!,W*^*"))
			), nil, 0),
			f.none(
				f.radius(radius, f.terrain("C*,K*^*,Urb^*,Rr*^*,*^Vhh,*^Vhc"), f.terrain("!,W*^*"))
			)
		)),
		f.radius(radius, f.terrain("C*,K*^*,Urb^*,Rr*^*,*^Vhh,*^Vhc"), f.terrain("!,W*^*"))
	))
end

local function wct_roads_to_industrial_city(radius)
	-- "Rrc"
	return map:find(f.all(
		f.terrain("!,W*^*,*^V*,*^Bcx*,Urb,C*,K*^*,R*"),
		f.adjacent(f.all(
			f.terrain("*^Vhc,Rrc"),
			f.adjacent(f.all(
				f.terrain("Rrc"),
				f.radius(radius, f.terrain("C*,K*^*,Urb^*,Rr^*,*^Vhh"), f.terrain("!,W*^*"))
			), nil, 0),
			f.none(
				f.radius(radius, f.terrain("C*,K*^*,Urb^*,Rr^*,*^Vhh"), f.terrain("!,W*^*"))
			)
		)),
		f.radius(radius, f.terrain("C*,K*^*,Urb^*,Rr^*,*^Vhh"), f.terrain("!,W*^*"))
	))
end

local function wct_roads_to_factory(radius)
	--"Rr"
	return map:find(f.all(
		f.terrain("!,W*^*,*^V*,*^Bcx*,Urb,C*,K*^*,R*"),
		f.adjacent(f.all(
			f.terrain("*^Vhh,Rr"),
			f.adjacent(f.all(
				f.terrain("Rr"),
				f.radius(radius, f.terrain("C*,K*^*,Urb^*"), f.terrain("!,W*^*"))
			), nil, 0),
			f.none(
				f.radius(radius, f.terrain("C*,K*^*,Urb^*"), f.terrain("!,W*^*"))
			)
		)),
		f.radius(radius, f.terrain("C*,K*^*,Urb^*"), f.terrain("!,W*^*"))
	))
end

local function wct_roads_to_river_industry(radius)
	-- "Re"
	return map:find(f.all(
		f.terrain("!,W*^*,*^V*,*^Bcx*,Urb,C*,K*^*,R*"),
		f.adjacent(f.all(
			f.terrain("*^Vud,Re,Rr"),
			f.adjacent(f.all(
				f.terrain("Re,Rr"),
				f.radius(radius, f.terrain("C*,K*^*,Urb^*"), f.terrain("!,W*^*"))
			), nil, 0),
			f.none(
				f.radius(999, f.terrain("K*^*"), f.terrain("Urb^*,*^Bcx*,C*,K*^*,*^V*,R*"))
			),
			f.none(
				f.radius(radius, f.terrain("C*,K*^*,Urb^*"), f.terrain("!,W*^*"))
			)
		)),
		f.radius(radius, f.terrain("C*,K*^*,Urb^*"), f.terrain("!,W*^*"))
	))
end

local function wct_industry_bridge(bridge, directions)

	set_terrain { "*^Bw" .. bridge,
		f.all(
			f.terrain("Ww,"),
			f.adjacent(f.terrain("*^Vud"), directions, 1),
			f.adjacent(f.terrain("Urb,G*,C*,K*^*,A*"), directions, 1)
		),
		layer = "overlay",
	}
	set_terrain { "Rr",
		f.all(
			f.terrain("G*,A*"),
			f.adjacent(f.terrain("*^Bw" .. bridge), directions, "1-2")
		),
	}

end

local function world_conquest_tek_map_decoration_6c()
	-- industrial villages near river and road
	set_terrain { "Hhd^Vud",
		f.all(
			f.terrain("H*^V*"),
			f.adjacent(f.terrain("Urb,Ww"))
		),
	}
	set_terrain { "Urb^Vud",
		f.all(
			f.terrain("G*^V*,D*^V*"),
			f.adjacent(f.terrain("Urb^*"))
		),
	}
	set_terrain { "Gs^Vud",
		f.all(
			f.terrain("G*^V*,D*^V*"),
			f.adjacent(f.terrain("Ww"))
		),
	}

	-- expand infrastructure of industrial villages near river
	wct_industry_bridge("\\", "nw,se")
	wct_industry_bridge("/", "ne,sw")
	wct_industry_bridge("|", "n,s")
	wct_iterate_roads_to(wct_roads_to_river_industry, 3, "Re")
	set_terrain { "Urb",
		f.terrain("Re,Rp"),
	}
	set_terrain { "*^Bcx\\",
		f.terrain("*^Bw\\"),
		layer = "overlay",
	}
	set_terrain { "*^Bcx/",
		f.terrain("*^Bw/"),
		layer = "overlay",
	}
	set_terrain { "*^Bcx|",
		f.terrain("*^Bw|"),
		layer = "overlay",
	}

	wct_iterate_roads_to(wct_roads_to_river_industry, 4, "Re")
	set_terrain { "Urb",
		f.terrain("Rr,Rp,Re,Gs^Vud"),
		layer = "base",
	}

	-- expand infrastructure to factories
	set_terrain { "*^Vhh",
		f.all(
			f.terrain("*^Vh"),
			f.radius(5, f.terrain("C*,K*^*,Urb^*"), f.terrain("!,W*^*"))
		),
		layer = "overlay",
	}

	wct_iterate_roads_to(wct_roads_to_factory, 4, "Rr")
	set_terrain { "Rr^Vhh",
		f.terrain("G*^Vhh"),
	}
	set_terrain { "Hhd^Vhh",
		f.terrain("H*^Vhh"),
	}

	-- expand infrastructure to cities
	set_terrain { "*^Vhc",
		f.all(
			f.terrain("*^Vh"),
			f.radius(6, f.terrain("C*,K*^*,Urb^*,Rr^*"), f.terrain("!,W*^*"))
		),
		layer = "overlay",
	}
	set_terrain { "Rrc^Vhc",
		f.terrain("G*^Vhc"),
	}

	wct_iterate_roads_to(wct_roads_to_industrial_city, 5, "Rrc")
	-- sawmills
	set_terrain { "*^Vl",
		f.all(
			f.adjacent(f.terrain("*^F*")),
			f.terrain("*^Vh")
		),
		layer = "overlay",
	}
	set_terrain { "Rb^Vl",
		f.terrain("G*^Vl"),
	}

	-- villages
	set_terrain { "*^Ve",
		f.all(
			f.terrain("*^Vh"),
			f.radius(4, f.terrain("C*,K*^*,Urb^*,Rr*^*"), f.terrain("!,W*^*"))
		),
		layer = "overlay",
	}
	set_terrain { "Rb^Ve",
		f.terrain("G*^Ve"),
	}

	-- expand infrastructure to villages and sawmills
	wct_iterate_roads_to(wct_roads_to_industrial_village, 3, "Rb")
	-- castle rails
	set_terrain { "Rr^Br|",
		f.all(
			f.terrain("Rr"),
			f.radius(8, f.terrain("K*^*"), f.terrain("Rr,C*,K*^*"))
		),
	}

	wct_iterate_roads_to(wct_rails_to_industrial_keep, 3, "Cud^Br|")
	set_terrain { "Rr^Br\\",
		f.all(
			f.adjacent(f.terrain("*^Vhh"), "nw,se", "1-2"),
			f.terrain("*^Br|")
		),
		layer = "overlay",
	}
	set_terrain { "Rr^Br/",
		f.all(
			f.adjacent(f.terrain("*^Vhh"), "ne,sw", "1-2"),
			f.terrain("*^Br|")
		),
		layer = "overlay",
	}

	wct_conect_factory_rails()
	-- muddy rivers
	set_terrain { "Sm",
		f.all(
			f.terrain("Ww^*"),
			f.radius(2, f.terrain("Wwt,Wot"), f.terrain("Ww^*,C*,K*^*"))
		),
		layer = "base",
	}
	local terrain_to_change = map:find(f.all(
		f.terrain("Sm^*"),
		f.adjacent(f.terrain("Ww^*"))
	))

	for swamp_i, swamp_loc in ipairs(terrain_to_change) do
		local r = mathx.random(3, map.width // 4)
		set_terrain { "Sm",
			f.all(
				f.terrain("Ww^*"),
				-- raduis, filter, filter_radius
				f.radius(r, f.is_loc(swamp_loc), f.terrain("Ww^*,C*,K*^*"))
			),
			layer = "base"
		}
	end
	-- dirty rivers
	local terrain_to_change = map:find(f.all(
		f.terrain("Sm^*"),
		f.adjacent(f.terrain("Ww^*"))
	))

	for water_i, water_loc in ipairs(terrain_to_change) do
		local r = mathx.random(4, map.width // 6)
		set_terrain { "Wwg",
			f.all(
				f.terrain("Ww^*"),
				-- raduis, filter, filter_radius
				f.radius(r, f.is_loc(water_loc), f.terrain("Ww^*,C*,K*^*"))
			),
			layer = "base"
		}
	end
	-- fords
	local r = mathx.random(4, map.width // 10)
	set_terrain { "Wwf",
		f.all(
			f.terrain("Ww^*"),
			f.none(
				f.radius(r, f.terrain("Wwg^*"), f.terrain("Ww^*,C*,K*^*"))
			)
		),
		layer = "base",
	}


	-- narrow rivers
	wct_ford_deltas()
	wct_dirty_deltas()
	-- detritus on sand
	set_terrain { "Dd,Dd,Dd,Dd,Dd,Dd^Edt",
		f.all(
			f.terrain("Dd"),
			f.adjacent(f.terrain("Sm,*^Vud"))
		),
	}
	set_terrain { "Ds,Ds,Ds^Edt",
		f.all(
			f.terrain("Ds"),
			f.adjacent(f.terrain("Sm,*^Vud"))
		),
	}

	-- reefs
	set_terrain { "Wwr",
		f.all(
			f.terrain("Wot,Wwt"),
			f.adjacent(f.terrain("Wwt,G*^*,M*^*,H*^*")),
			f.none(
				f.radius(4, f.terrain("Sm"))
			)
		),
		fraction = 15,
	}

	-- beachs sand and stones
	set_terrain { "Ds^Esd",
		f.all(
			f.terrain("Ds"),
			f.adjacent(f.terrain("Wwt,Wot"))
		),
		fraction_rand = "9..11",
	}

	-- extra rough near clean water
	set_terrain { "Hh",
		f.all(
			f.terrain("G*^*"),
			f.radius(2, f.terrain("Wwf"))
		),
		fraction = 6,
		layer = "base",
	}
	set_terrain { "*^Fp,*^Fp,*^Fp,*^Fms,*^Fms,*^Fmf,*^Fmf,*^Tf,*^Fds,*^Fms,*^Fms,*^Fet,*^Efm,*",
		f.all(
			f.terrain("Hh,G*"),
			f.radius(3, f.any(
				f.terrain("Wwf"),
				f.radius(2, f.terrain("Ww^*"))
			))
		),
		fraction = 5,
		layer = "overlay",
	}
	set_terrain { "*^Em",
		f.terrain("H*^Efm,Gd^Efm"),
		layer = "overlay",
	}

	-- fix ocean water type
	set_terrain { "Rd",
		f.all(
			f.adjacent(f.terrain("Sm")),
			f.terrain("Wwt,Wot")
		),
	}
	set_terrain { "Ww",
		f.terrain("Wwt^*"),
		layer = "base",
	}
	set_terrain { "Wo",
		f.terrain("Wot"),
	}
	set_terrain { "Wwg",
		f.all(
			f.terrain("Sm"),
			f.none(
				f.radius(999, f.terrain("*^V*,C*,K*^*"), f.terrain("Sm^*,Wwg"))
			)
		),
	}
	set_terrain { "Wwg",
		f.terrain("Rd"),
	}

	-- castle kinds
	set_terrain { "Cud",
		f.all(
			f.terrain("Ch^*"),
			f.radius(5, f.terrain("K*^*,Sm,Wwg"), f.terrain("C*^*,Sm,Wwg,K*^*"))
		),
		layer = "base",
	}
	set_terrain { "Chw",
		f.terrain("Ch"),
	}

	-- industry damages landscape
	set_terrain { "Re,Gd",
		f.all(
			f.terrain("G*^*"),
			f.adjacent(f.terrain("Urb,*^Vud"))
		),
		fraction = 1,
		layer = "base",
	}
	set_terrain { "Re,Gd,Gs",
		f.all(
			f.terrain("G*"),
			f.adjacent(f.terrain("*^Vl"))
		),
		fraction = 1,
		layer = "base",
	}
	set_terrain { "Gs,Gs,Gs,Gd",
		f.all(
			f.terrain("G*^*"),
			f.adjacent(f.terrain("Rr^*,*^Vhh,Cud^*,Ch^*"))
		),
		fraction = 2,
		layer = "base",
	}
	set_terrain { "Gd",
		f.all(
			f.adjacent(f.terrain("Re")),
			f.terrain("G*^*")
		),
		layer = "base",
	}
	set_terrain { "Gs",
		f.all(
			f.adjacent(f.terrain("Gd*^*")),
			f.terrain("Gg*^*")
		),
		layer = "base",
	}
	set_terrain { "Hhd,Hhd,Hhd,Hhd,Hhd^Dr",
		f.all(
			f.terrain("Hh^*"),
			f.adjacent(f.terrain("Urb,*^Vud"))
		),
		fraction = 2,
		layer = "base",
	}
	set_terrain { "Gd^Dr,Gd^Edt,Hhd^Edt,Hhd,Hhd,Hhd,Hhd,Hhd^Dr",
		f.all(
			f.terrain("Hh"),
			f.radius(2, f.terrain("*^Vud"))
		),
		fraction = 2,
	}

	-- rough extra terrain noise
	set_terrain { "Gs^Fp,Gs^Fp,Gs^Fp,Gs^Fp,Gs^Fp,Gs^Fmw,Gs^Fmf,Hh^Fp,Hh,Hh,Mm,Mm,Gs^Tf",
		f.all(
			f.terrain("G*"),
			f.adjacent(f.terrain("Sm^*,C*^*,K*^*,W*^*,*^V*,Ds"), nil, 0)
		),
		fraction = 15,
	}
	set_terrain { "Ch",
		f.all(
			f.terrain("G*"),
			f.adjacent(f.terrain("Sm^*,C*^*,K*^*,W*^*,*^V*,Ds"), nil, 0),
			f.radius(2, f.terrain("Urb"))
		),
		fraction = 20,
	}
	set_terrain { "Hh^Fp,Hh^Fp,Hh^Fp,Hh^Fp,Hh^Fmw",
		f.terrain("Hh"),
		fraction = 24,
	}
	set_terrain { "Hd",
		f.terrain("Dd"),
		fraction = 10,
	}

	-- cave path
	set_terrain { "Rb",
		f.terrain("Uh,Uu"),
		fraction = 20,
	}

	-- difumine dry border
	set_terrain { "Gs",
		f.all(
			f.terrain("Gd^*"),
			f.adjacent(f.terrain("Gs^*"))
		),
		fraction = 11,
		layer = "base",
	}
	set_terrain { "*^Fdw",
		f.terrain("Gd^Fet,Re^Fet"),
		layer = "overlay",
	}

end

local function world_conquest_tek_map_repaint_6c()
	wct_map_reduce_castle_expanding_recruit("Ce", "Wwf")
	set_terrain { "Ch",
		f.terrain("Ce"),
	}

	-- soft hills clusters
	set_terrain { "Gs,Gs,Gg,Gs,Gs,Gg,Gs,Gs,Gg,Gs,Gs,Gg,Hh^Fp",
		f.all(
			f.terrain("Hh,Hh^F*,Hh^Tf"),
			f.adjacent(f.terrain("H*^*,M*^*,C*,K*^*"), nil, 6)
		),
		fraction = 2,
	}
	set_terrain { "Gs,Aa^Fma,Wwf,Aa,Aa",
		f.all(
			f.terrain("Ha,Ha^F*"),
			f.adjacent(f.terrain("H*^*,M*^*,C*,K*^*"), nil, 6)
		),
		fraction = 2,
	}


	world_conquest_tek_map_decoration_6c()

	wct_reduce_wall_clusters("Uu,Uu^Tf,Uh,Uu^Tf,Uu,Uu^Tf,Uh,Ql,Qxu,Xu,Uu,Rb")
	wct_fill_lava_chasms()
	wct_volcanos()
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

	-- decoration fix to first intended
	set_terrain { "Rb",
		f.terrain("Urb^*"),
		layer = "base",
	}

end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"Industrial")
	wct_enemy_castle_expansion()
	world_conquest_tek_map_repaint_6c()
end
