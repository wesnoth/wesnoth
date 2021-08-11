-- Podzol

local function world_conquest_tek_map_repaint_4d()
	local rad = mathx.random_choice("1,2,2")
	world_conquest_tek_map_noise_proxy(rad,  2 , "!,W*^*,Ai,Ds*^*,Xu,M*^Xm,R*^*,Ch*,Cud,K*,U*^*,Ql^B*")

	wct_reduce_wall_clusters("Uu,Uu,Uu,Uu,Uh,Uh,Ai")
	wct_fill_lava_chasms()
	-- tweak rough terrain generated
	set_terrain { "Aa,Aa,Aa,Aa^Fpa",
		f.terrain("Ms,Ha"),
		exact = false,
		percentage = 7,
	}
	set_terrain { "Gll",
		f.terrain("Hhd^*,Mm"),
		exact = false,
		percentage = 8,
	}
	set_terrain { "Mm,Gll,Gll",
		f.terrain("Hh^Tf,Ss,Hh,Gll^Tf"),
		exact = false,
		percentage = 9,
	}
	set_terrain { "Hh",
		f.terrain("Hhd^*"),
		layer = "base",
	}
	set_terrain { "Ur",
		f.all(
			f.terrain("U*"),
			f.adjacent(f.terrain("U*^*,Q*^*,X*"), nil, 6)
		),
		fraction = 7,
	}

	-- extra rough terrain on big plains
	set_terrain { "Ha,Ha^Fpa,Ms,Aa^Fpa",
		f.all(
			f.terrain("Aa"),
			f.adjacent(f.terrain("G*,R*,Ww,A*"), nil, 6)
		),
		exact = false,
		percentage = 10,
	}
	set_terrain { "Hh,Mm,Gd^Fp,Gd^Fp,Gd^Fp,Hh^Fp",
		f.all(
			f.terrain("Gd"),
			f.adjacent(f.terrain("G*,R*,Ww,A*,Gll^Efm"), nil, 6)
		),
		exact = false,
		percentage = 10,
	}
	set_terrain { "Hh,Mm,Gll^Fp,Gll^Fp,Gll^Fp,Gll^Fp,Hh^Fp,Gll^Tf,Hh^Tf",
		f.all(
			f.terrain("Gll"),
			f.adjacent(f.terrain("G*,R*,Ww,A*,Gll^Efm"), nil, 6)
		),
		exact = false,
		percentage = 10,
	}

	-- fix castles
	wct_map_reduce_castle_expanding_recruit("Ce", "Gll^Fet")
	wct_map_reduce_castle_expanding_recruit("Cud", "Rb")
	set_terrain { "Ch",
		f.terrain("Cud"),
	}

	-- better looking adjacences to lava and frozen
	set_terrain { "Uu,Uh,Uu^Tf",
		f.all(
			f.terrain("W*"),
			f.adjacent(f.terrain("Ql"))
		),
		fraction = 2,
	}
	set_terrain { "Uu,Uh",
		f.all(
			f.terrain("Ql"),
			f.adjacent(f.terrain("W*^*"))
		),
	}
	set_terrain { "Wwf",
		f.all(
			f.terrain("Ai"),
			f.adjacent(f.terrain("Ql"))
		),
	}
	set_terrain { "Ms",
		f.all(
			f.terrain("Mm"),
			f.adjacent(f.terrain("Aa^*,Ai,Ms*^*,Ha^*,Kha,Cha"))
		),
	}


	wct_dirt_beachs("15..20")
	wct_volcanos()
	-- set sea color and river temperature
	set_terrain { "Wwg",
		f.terrain("Ww"),
	}

	if mathx.random(2) == 1 then
		set_terrain { "Ai",
			f.all(
				f.terrain("Wwg"),
				f.adjacent(f.terrain("Aa^*,Ai,Ha*^*,Ms*^*"))
			),
			exact = false,
			percentage = 30,
		}
		set_terrain { "Wwg",
			f.terrain("Wwt^*"),
			layer = "base",
		}
		set_terrain { "Wwrg",
			f.terrain("Wwrt"),
		}
		set_terrain { "Wog",
			f.terrain("Wot"),
			layer = "base",
		}
	end
end

local function wct_map_4d_post_bunus_decoration()
	wct_map_cave_path_to("Re")
end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"Podzol")
	wct_enemy_castle_expansion()
	world_conquest_tek_map_repaint_4d()
	world_conquest_tek_bonus_points()
	wct_map_4d_post_bunus_decoration()
end
