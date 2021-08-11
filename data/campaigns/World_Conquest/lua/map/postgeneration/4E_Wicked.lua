-- Wicked
function world_conquest_tek_map_repaint_4e()
	wct_reduce_wall_clusters("Uu,Uu,Uu,Uu,Uu,Uu,Uu,Uu,Uh,Uh,Uh,Uh,Uu^Tf,Wwf,Ur,Qxu,Ql,Wwg")
	wct_fill_lava_chasms()
	-- soft rough terrain around caves
	set_terrain { "Aa,Aa,Gs",
		f.terrain("Ha"),
		fraction = 3,
	}
	set_terrain { "Dd,Dd,Gg",
		f.terrain("Hd"),
		fraction = 4,
	}
	set_terrain { "Gs,Gg,Gg",
		f.terrain("H*^Tf,Hh,Hhd"),
		fraction = 3,
	}
	set_terrain { "Aa,Aa,Aa,Gs",
		f.terrain("Ms"),
		fraction = 3,
	}
	set_terrain { "Gs,Gg,Gg",
		f.terrain("Mm,Md"),
		fraction = 3,
	}

	-- store map size and create axi filters variables
	local f_north_half = f.x("1-" .. (map.height // 2))
	local f_south_half = f.none(f_north_half)
	local f_west_half = f.y("1-" .. (map.width // 2))
	local f_east_half = f.none(f_west_half)

	-- north and west ice become snow, else becomes sand
	set_terrain { "Aa",
		f.all(
			f.terrain("Ai"),
			f.any(
				f_north_half,
				f_west_half
			)
		),
	}
	set_terrain { "Ds",
		f.terrain("Ai"),
	}

	-- north dessert become swamp
	set_terrain { "Ss",
		f.all(
			f_north_half,
			f.terrain("Dd^*")
		),
		layer = "base",
	}
	set_terrain { "Hh",
		f.all(
			f.terrain("Hd^*,Hhd^*"),
			f.any(
				f_north_half,
				f_west_half
			)
		),
		layer = "base",
	}
	set_terrain { "Sm",
		f.all(
			f.terrain("Ss^*"),
			f.adjacent(f.terrain("Dd*^*,Hd^*"))
		),
		layer = "base",
	}

	-- north cave water become ice
	set_terrain { "Ai",
		f.all(
			f_north_half,
			f.terrain("Wwg")
		),
	}

	-- south snow become swamp or water
	set_terrain { "Wwt",
		f.all(
			f_west_half,
			f.terrain("Aa^*"),
			f.none(
				f_north_half
			)
		),
		layer = "base",
	}
	set_terrain { "Sm",
		f.all(
			f.terrain("Aa^*"),
			f.none(
				f_north_half
			),
			f.none(
				f_west_half
			)
		),
		layer = "base",
	}
	set_terrain { "Hh",
		f.all(
			f.terrain("Ha^*"),
			f.none(
				f_north_half
			)
		),
		layer = "base",
	}
	set_terrain { "Mm",
		f.all(
			f.terrain("Ms^*"),
			f.none(
				f_north_half
			)
		),
		layer = "base",
	}
	set_terrain { "Ss",
		f.all(
			f.terrain("Sm^*"),
			f.radius(4, f.terrain("Aa^*,Ha^*,Wwt^*"), f.terrain("Aa^*,Ha^*,Sm^*"))
		),
		layer = "base",
	}
	set_terrain { "Wwf",
		f.all(
			f.terrain("Wwt"),
			f.adjacent(f.terrain("M*^*,C*^*,K*^*"))
		),
	}
	set_terrain { "Wwf",
		f.all(
			f.terrain("Wwt,Ds"),
			f.adjacent(f.terrain("Wwf"))
		),
		fraction = 3,
	}

	-- expand ice west
	set_terrain { "Ds,Ai,Ai,Ai",
		f.all(
			f.terrain("Ds"),
			f.adjacent(f.terrain("Aa^*,Ha^*,Ms^*"))
		),
	}
	set_terrain { "Ss,Ai,Ai,Ai,Ai,Ai",
		f.all(
			f.terrain("Ss"),
			f.adjacent(f.terrain("Aa^*,Ha^*,Ms^*"))
		),
	}
	set_terrain { "Wwg,Ai,Ai,Ai,Ai",
		f.all(
			f.terrain("W*"),
			f.adjacent(f.terrain("Aa^*,Ha^*,Ms^*"))
		),
	}

	-- oasis and dessert plants southwest
	set_terrain { "Dd^Do,Dd^Edpp",
		f.all(
			f_west_half,
			f.terrain("Dd"),
			f.none(
				f_north_half
			)
		),
		fraction = 7,
	}

	-- swamp mushrooms east
	set_terrain { "Ss^Tf",
		f.all(
			f.terrain("S*"),
			f.none(
				f_west_half
			)
		),
		fraction = 10,
		layer = "overlay",
	}

	-- castles
	set_terrain { "Coa",
		f.all(
			f_north_half,
			f.terrain("Aa"),
			f.adjacent(f.terrain("C*^*,K*^*"), nil, 0),
			f.none(
				f_west_half
			)
		),
		fraction = 12,
	}
	set_terrain { "Chw",
		f.all(
			f.terrain("Wwt"),
			f.adjacent(f.terrain("C*^*,K*^*"), nil, 0)
		),
		fraction = 11,
	}
	set_terrain { "Chs",
		f.all(
			f.terrain("Ss"),
			f.adjacent(f.terrain("C*^*,K*^*"), nil, 0),
			f_north_half,
			f_west_half
		),
		fraction = 10,
	}
	set_terrain { "Cdr^Edt,Cd",
		f.all(
			f.terrain("Dd"),
			f.adjacent(f.terrain("C*^*,K*^*"), nil, 0),
			f.none(
				f_west_half
			),
			f.none(
				f_north_half
			)
		),
		fraction = 12,
	}

	-- reefs
	set_terrain { "Ww",
		f.terrain("Wwt,Wwg"),
	}
	set_terrain { "Wwr,Wwrg",
		f.all(
			f_north_half,
			f.terrain("Ww,Wo")
		),
		fraction = 14,
	}
	set_terrain { "Wwr,Wwrt",
		f.all(
			f.terrain("Ww,Wo"),
			f.none(
				f_north_half
			)
		),
		fraction = 14,
	}

	-- twisted water
	set_terrain { "Wwg",
		f.all(
			f_north_half,
			f.terrain("Ww^*")
		),
		fraction = 2,
		layer = "base",
	}
	set_terrain { "Wog",
		f.all(
			f_north_half,
			f.terrain("Wo")
		),
		fraction = 2,
	}
	set_terrain { "Wwt",
		f.all(
			f.terrain("Ww^*"),
			f.none(
				f_north_half
			)
		),
		fraction = 2,
		layer = "base",
	}
	set_terrain { "Wot",
		f.all(
			f.terrain("Wo"),
			f.none(
				f_north_half
			)
		),
		fraction = 2,
	}

	-- fix south grass
	set_terrain { "Gd",
		f.all(
			f.terrain("Gg^*"),
			f.none(
				f_north_half
			)
		),
		layer = "base",
	}
	set_terrain { "Gg",
		f.all(
			f_west_half,
			f.terrain("Gs^*"),
			f.none(
				f_north_half
			)
		),
		layer = "base",
	}
	set_terrain { "Gs",
		f.all(
			f_west_half,
			f.terrain("Gd^*"),
			f.none(
				f_north_half
			)
		),
		layer = "base",
	}
	set_terrain { "Gs",
		f.all(
			f.terrain("Gd^*"),
			f.adjacent(f.terrain("Gg^*"))
		),
		layer = "base",
	}

	-- forest and extra rough
	set_terrain { "Hh^Fp",
		f.terrain("Ha,Hh,Hhd"),
		fraction = 7,
		layer = "overlay",
	}
	set_terrain { "Hh^Fp",
		f.terrain("G*"),
		fraction = 5,
		layer = "overlay",
	}
	set_terrain { "Aa^Fpa",
		f.terrain("Aa"),
		fraction = 5,
		layer = "overlay",
	}
	set_terrain { "Hh,Hh,Hh,Hh,Hh^Fp,Mm",
		f.terrain("G*"),
		fraction = 6,
	}
	set_terrain { "Ha,Ha,Ha,Ha,Ha^Fpa,Ms",
		f.terrain("Aa"),
		fraction = 6,
	}
	set_terrain { "Hd,Hd,Hd,Hd,Hd,Md",
		f.terrain("Dd"),
		fraction = 8,
	}

	wct_randomize_snowed_forest()
	set_terrain { "Gg^Fp,Gg^Fms,Gg^Fds,Gg^Fds,Gg^Ft,Gg^Ftp,Gg^Ftr,Gg^Ftd,Gg^Fet,Gg^Fet",
		f.all(
			f_west_half,
			f.terrain("Gg^F*,Hh^F*"),
			f.none(
				f_north_half
			)
		),
		fraction = 1,
		layer = "overlay",
	}

	local r = "Gs^Fp,Gs^Fms,Gs^Fds,Gs^Ft,Gs^Ft,Gs^Ftp,Gs^Ftr,Gs^Ftd,Gs^Fet"
	r = mathx.random_choice(r)
	set_terrain { r,
		f.all(
			f_west_half,
			f.terrain("Gs^F*"),
			f.none(
				f_north_half
			)
		),
	}

	local r = "Gs^Fp,Gs^Fms,Gs^Fds,Gs^Ftp,Gs^Ft,Gs^Ftp,Gs^Ftr,Gs^Ftd,Gs^Fet,Gs^Fts,Gs^Fts,Gs^Ft,Gs^Ft,Gs^Ftd,Gs^Fp"
	r = mathx.random_choice(r)
	set_terrain { r,
		f.all(
			f.terrain("Gs^F*,Hh^F*"),
			f.none(
				f_west_half
			),
			f.none(
				f_north_half
			)
		),
		fraction = 1,
		layer = "overlay",
	}
	set_terrain { "Gs^Fp,Gs^Fms,Gs^Fdf,Gs^Ftp,Gs^Ftd,Gs^Fmf,Gs^Fp,Gs^Ftd,Gs^Fetd,Gs^Fts,Gs^Fts,Gs^Fmw,Gs^Fdf,Gs^Fp",
		f.all(
			f.terrain("Gd^F*,Hhd^F*"),
			f.none(
				f_west_half
			),
			f.none(
				f_north_half
			)
		),
		fraction = 1,
		layer = "overlay",
	}
	set_terrain { "Gs^Fp,Gs^Fp,Gs^Fp,Gs^Fms,Gs^Fp,Gs^Fmf,Gs^Fmw,Gs^Fmw,Gs^Fmw,Gs^Fdw,Gs^Fet,Gs^Fetd",
		f.all(
			f_north_half,
			f.terrain("Gs^F*,Hh^F*")
		),
		fraction = 1,
		layer = "overlay",
	}
	set_terrain { "Gg^Fp,Gg^Fp,Gg^Fms,Gg^Fp,Gg^Fmf,Gg^Fp,Gg^Fmf,Gg^Fds,Gs^Fms,Gs^Ftr,Gs^Fet,Gs^Fet",
		f.all(
			f_north_half,
			f.terrain("Gg^F*")
		),
	}

	-- expand snow northwest
	set_terrain { "Ms",
		f.all(
			f_west_half,
			f.terrain("M*^*"),
			f.adjacent(f.terrain("Aa^*,Ha^*,Ms^*,Ai"))
		),
		layer = "base",
	}

	-- fix villages
	set_terrain { "Ww^Vm",
		f.terrain("W*^V*"),
	}
	set_terrain { "*^Vhs",
		f.terrain("S*^V*"),
		layer = "overlay",
	}
	set_terrain { "Hh^Vo",
		f.terrain("Hh^Vd"),
	}
	set_terrain { "Hh^Vhh",
		f.all(
			f_west_half,
			f.terrain("Hh^Vhha")
		),
	}
	set_terrain { "Hh^Vo",
		f.terrain("Hh^Vhha"),
	}
	set_terrain { "Mm^Vo",
		f.all(
			f.terrain("Mm^Vhh"),
			f_west_half,
			f_north_half
		),
	}
	set_terrain { "Ha^Vaa",
		f.all(
			f_west_half,
			f.terrain("Ha^Vhha")
		),
	}
	set_terrain { "Ds^Vd",
		f.all(
			f.terrain("Ds^Vda"),
			f.none(
				f_west_half
			),
			f.none(
				f_north_half
			)
		),
	}
	set_terrain { "Hh^Vo",
		f.all(
			f.terrain("Hh^Vhh"),
			f.none(
				f_west_half
			),
			f.none(
				f_north_half
			)
		),
	}
	set_terrain { "Ds^Vc",
		f.all(
			f.terrain("Ds^Vda"),
			f_west_half,
			f_north_half
		),
	}
	set_terrain { "Gd^Vc",
		f.terrain("Gd^Vh"),
	}
	set_terrain { "Dd^Vda",
		f.all(
			f.terrain("Dd^Vd"),
			f.none(
				f_west_half
			)
		),
	}
	set_terrain { "Aa^Voa",
		f.all(
			f.terrain("Aa^V*,Ha^V*,Ms^V*"),
			f.none(
				f_west_half
			)
		),
		layer = "overlay",
	}
	set_terrain { "Rr^Vhc",
		f.all(
			f_west_half,
			f.terrain("Gg^Vh"),
			f.adjacent(f.terrain("Ch,Kh")),
			f.none(
				f_north_half
			)
		),
	}
	set_terrain { "Gg^Ve",
		f.all(
			f_west_half,
			f.terrain("G*^Vh"),
			f.adjacent(f.terrain("*^F*"), nil, "2-6"),
			f.none(
				f_north_half
			)
		),
		layer = "overlay",
	}
	set_terrain { "Gg^Ve",
		f.all(
			f_north_half,
			f.terrain("G*^Vh"),
			f.adjacent(f.terrain("*^F*"), nil, "2-6"),
			f.none(
				f_west_half
			)
		),
		layer = "overlay",
	}

	-- fix roads
	set_terrain { "Rb",
		f.all(
			f_north_half,
			f.terrain("Re"),
			f.adjacent(f.all(
				f_west_half,
				f.terrain("Gs^*")
			), nil, 0),
			f.none(
				f.radius(4, f.y("$map_data.axi.y"))
			)
		),
	}
	set_terrain { "Rd",
		f.all(
			f.terrain("Re"),
			f.adjacent(f.terrain("Gd^*,Dd^*,Hd^*,Hhd"), nil, "3-6"),
			f.none(
				f.radius(3, f.y("$map_data.axi.y"))
			)
		),
	}

	wct_volcanos()
	wct_volcanos_dirt()
	wct_dirt_beachs("9..11")
end

function wct_map_4e_post_bunus_decoration()
	-- store map size and create axi filters variables
	local f_north_half = f.x("1-" .. (map.height // 2))
	local f_south_half = f.none(f_north_half)
	local f_west_half = f.y("1-" .. (map.width // 2))
	local f_east_half = f.none(f_west_half)
	-- flowers southwest
	set_terrain { "Gg^Efm",
		f.all(
			f_west_half,
			f.terrain("Gg"),
			f.none(
				f_north_half
			)
		),
		fraction = mathx.random(10,30),
	}

	-- small mushrooms northeast
	set_terrain { "Gg^Em",
		f.all(
			f_north_half,
			f.terrain("Gg"),
			f.adjacent(f.terrain("Ss")),
			f.adjacent(f.terrain("*^F*")),
			f.none(
				f_west_half
			)
		),
		fraction = mathx.random(2,3),
	}

	-- slighty soft dessert southwest
	set_terrain { "Dd,Dd,Rd,Dd,Dd,Rd,Dd,Dd,Rd,Dd,Dd,Gd",
		f.all(
			f_west_half,
			f.terrain("Dd"),
			f.adjacent(f.terrain("Dd^*,Hd^*"), nil, "0-5"),
			f.none(
				f_north_half
			)
		),
		fraction = 3,
	}

	-- slighty soft muddy swamps
	set_terrain { "Sm,Sm,Rb",
		f.all(
			f.terrain("Sm"),
			f.adjacent(f.terrain("Sm^*"), nil, "0-5")
		),
		fraction = 3,
	}

	-- cave paths
	set_terrain { "Rb",
		f.all(
			f_north_half,
			f.terrain("Ur")
		),
	}

	wct_map_cave_path_to("Re")

end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"Wicked")
	wct_enemy_castle_expansion()
	world_conquest_tek_map_repaint_4e()
	world_conquest_tek_bonus_points()
	wct_map_4e_post_bunus_decoration()
	wct_map_enemy_themed("undead", "Soulless", "ha", "Aa^Vha", 1)
	wct_map_enemy_themed("elf", "Wolf", "v", "Gg^Ve", 1)
	wct_map_enemy_themed("dwarf", "Giant Mudcrawler", "ud", "Ur^Vud", 1)
	wct_map_enemy_themed("orc", "Giant Scorpion", "o", "Gs^Vo", 1)
	wct_map_enemy_themed("troll", "Giant Scorpion", "o", "Gs^Vo", 1)
	wct_map_enemy_themed("wolf", "Giant Scorpion", "o", "Gs^Vo", 1)
	wct_map_enemy_themed("human", "Young Ogre", "e", "Gg^Vl", 1)
end
