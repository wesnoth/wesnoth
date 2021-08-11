-- Savannah

function world_conquest_tek_map_repaint_3a()
	world_conquest_tek_map_rebuild("Uu,Uu^Tf,Uh,Uu^Tf,Uu,Uu^Tf,Uh,Ql,Qxu,Xu", 2)
	world_conquest_tek_castle_swamp_bridges()
	world_conquest_tek_map_decoration_3a()
	world_conquest_tek_map_dirt("Gs^Tf")
end

function world_conquest_tek_map_decoration_3a()
	-- chances of some dessert over swamp
	if mathx.random(4) ~= 1 then
		set_terrain { "Dd^Edp,Dd^Do,Dd,Dd,Dd,Dd,Dd,Dd",
			f.all(
				f.terrain("Ss"),
				f.adjacent(f.terrain("Hd"))
			),
		}
		set_terrain { "Sm,Sm,Sm,Sm,Sm,Sm,Dd,Dd,Dd,Dd,Dd,Dd^Edp",
			f.all(
				f.terrain("Ss"),
				f.adjacent(f.terrain("Hd*^*,D*^*,Ss"), nil, "4-6"),
				f.none(
					f.radius(11, f.terrain("Wo"))
				)
			),
		}
	end
	-- add desert castles in base to desert clusters
	set_terrain { "Dd,Dd,Cdr^Edp,Cdr^Esd,Cdr^Es,Cdr^Edt,Cdr^Edb",
		f.all(
			f.terrain("Dd"),
			f.adjacent(f.terrain("Dd*^*,Ds"), nil, "4-6"),
			f.adjacent(f.terrain("Ss,C*,K*"), nil, 0)
		),
		fraction_rand = "11..14",
	}
	set_terrain { "Rd",
		f.all(
			f.terrain("Re"),
			f.adjacent(f.terrain("U*^*,Xu,Qxu"), nil, 0)
		),
	}

	wct_map_reduce_castle_expanding_recruit("Ce", "Rd")
	set_terrain { "Gs^Vd",
		f.all(
			f.terrain("Gs^Vht"),
			f.adjacent(f.terrain("Gs^Ft"), nil, 0)
		),
	}
	set_terrain { "Rd^Vc",
		f.all(
			f.terrain("Gs^Vht"),
			f.adjacent(f.terrain("Rd"))
		),
	}
	set_terrain { "Gs^Fetd",
		f.all(
			f.terrain("Gg^Fet"),
			f.adjacent(f.terrain("Gs^Vd,Dd,Hd"))
		),
	}

	-- dry grass, hills and mountains near dessert
	set_terrain { "Gd",
		f.all(
			f.terrain("Gs"),
			f.adjacent(f.terrain("Hd")),
			f.adjacent(f.terrain("Ss,Ds,Rd,Re"), nil, 0),
			f.adjacent(f.terrain("G*^F*"), nil, "0-1")
		),
	}
	set_terrain { "Hhd^Tf",
		f.all(
			f.terrain("Hh^Tf"),
			f.adjacent(f.terrain("Dd,Hd"), nil, "2-6"),
			f.adjacent(f.terrain("Ss,Wwt,Wot,Wwrt"), nil, 0)
		),
	}
	set_terrain { "Hhd",
		f.all(
			f.terrain("Hh"),
			f.adjacent(f.terrain("Dd,Hd,Hhd^Tf"), nil, "2-6"),
			f.adjacent(f.terrain("Ss,Wwt,Wot,Wwrt"), nil, 0)
		),
	}
	set_terrain { "Md",
		f.all(
			f.terrain("Mm"),
			f.adjacent(f.terrain("Dd,Hd,Hhd,Hhd^Tf"), nil, "2-6")
		),
	}
	set_terrain { "Hh^Vl",
		f.all(
			f.terrain("Hh^Vhh"),
			f.none(
				f.radius(2, f.terrain("M*^*,Xu,Uu,Uh,Uu^Tf,Ds,Dd,Hd,Ql,Qxu"))
			)
		),
	}

	-- desrt plants near wet terrain
	set_terrain { "Dd^Edp",
		f.all(
			f.terrain("Dd"),
			f.adjacent(f.terrain("W*^*,Ss,*^F*"), nil, "3-6")
		),
		fraction_rand = "2..3",
	}

	-- 1.12 new forest
	set_terrain { "*^Ftd",
		f.terrain("*^Ft"),
		fraction = 2,
		layer = "overlay",
	}
	set_terrain { "*^Fts",
		f.terrain("*^Ft"),
		layer = "overlay",
	}
	set_terrain { "*^Ftp",
		f.all(
			f.terrain("*^Fts"),
			f.radius(2, f.terrain("S*^*,W*^*"))
		),
		fraction = 3,
		layer = "overlay",
	}
	set_terrain { "*^Fts",
		f.all(
			f.terrain("*^Ftd"),
			f.radius(3, f.terrain("Dd*^*,Hd*^*"))
		),
		fraction = 2,
		layer = "overlay",
	}
	set_terrain { "Xue",
		f.terrain("Xu"),
	}

	if mathx.random(20) ~= 1 then
		wct_change_map_water("t")
	end
	set_terrain { "Gs^Ft",
		f.terrain("*^Fet"),
	}
end

function wct_castle_swamp_bridge(bridge, directions ,terrain)
	set_terrain { "Ss^" .. bridge,
		f.all(
			f.terrain("Ss"),
			f.adjacent(f.terrain("Ch,Kh"), directions, 1),
			f.adjacent(f.terrain(terrain), directions, 1)
		),
		layer = "overlay",
	}

end

function world_conquest_tek_castle_swamp_bridges()
	wct_castle_swamp_bridge("Bh\\", "se,nw", "Hh*^*,Mm,Md,,M*^V*")
	wct_castle_swamp_bridge("Bh/", "sw,ne", "Hh*^*,Mm,Md,,M*^V*")
	wct_castle_swamp_bridge("Bh|", "s,n", "Hh*^*,Mm,Md,M*^V*")

	wct_castle_swamp_bridge("Bw|", "s,n", "!,Ds,S*,*^B*,Xu,Q*,M*^*,W*,Ch*,Kh*")
	wct_castle_swamp_bridge("Bw/", "sw,ne", "!,Ds,S*,*^B*,Xu,Q*,M*^*,W*,Ch*,Kh*")
	wct_castle_swamp_bridge("Bw\\", "se,nw", "!,Ds,S*,*^B*,Xu,Q*,M*^*,W*,Ch*,Kh*")
end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"Savannah")
	world_conquest_tek_map_noise_classic("Gs^Ft")
	wct_enemy_castle_expansion()
	world_conquest_tek_map_repaint_3a()
	world_conquest_tek_bonus_points()
	wct_map_cave_path_to("Re")
	wct_map_enemy_themed("orc", "Giant Scorpion", "o", "Gs^Vo", 8)
	wct_map_enemy_themed("troll", "Giant Scorpion", "o", "Gs^Vo", 8)
	wct_map_enemy_themed("wolf", "Giant Scorpion", "o", "Gs^Vo", 8)
end
