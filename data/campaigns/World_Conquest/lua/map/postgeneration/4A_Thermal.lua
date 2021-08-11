-- Thermal

function world_conquest_tek_map_repaint_4a()
	world_conquest_tek_map_rebuild("Uu,Uu^Tf,Uh,Uu^Tf,Uu,Uu^Tf,Uh,Ql,Ql,Ql", 3)
	world_conquest_tek_map_decoration_4a()
	world_conquest_tek_map_dirt("Gg^Tf,Gg^Tf,Gs^Tf")
end

function world_conquest_tek_map_decoration_4a()
	set_terrain { "Gs^Fmf",
		f.terrain("Gs^Ft"),
	}
	set_terrain { "Gg^Fds",
		f.all(
			f.terrain("Gs^Fmf"),
			f.adjacent(f.terrain("Ss")),
			f.adjacent(f.terrain("Ds,Dd,Hd"), nil, 0)
		),
	}
	set_terrain { "Hh^Fmf",
		f.terrain("Hh^Ft"),
	}
	set_terrain { "Gs^Vc",
		f.all(
			f.terrain("Gs^Vht"),
			f.adjacent(f.terrain("Ss,Ds"), nil, 0)
		),
	}
	set_terrain { "Hh^Fds",
		f.all(
			f.terrain("Hh^Fp"),
			f.adjacent(f.terrain("Mm,Mm^Xm,Xu,Mv"), nil, "2-6"),
			f.adjacent(f.terrain("Hh^Fmf,Gs^Fmf"), nil, 0)
		),
	}
	set_terrain { "Gs^Fms",
		f.all(
			f.terrain("Gs^Fmf"),
			f.radius(2, f.terrain("Ww,Wo,Ss,Gg^Fds"))
		),
	}
	set_terrain { "Gs^Fms",
		f.all(
			f.terrain("Gs^Fp"),
			f.adjacent(f.terrain("Gg^Fds"))
		),
	}

	-- tropical forest near lava
	set_terrain { "Hh^Ft",
		f.all(
			f.terrain("Hh^Fp,Hh^Fmf"),
			f.radius(2, f.terrain("Ql,Mv"))
		),
	}
	set_terrain { "Gs^Ft",
		f.all(
			f.terrain("Gs^Fp,Gs^Fmf"),
			f.radius(2, f.terrain("Ql,Mv"))
		),
	}
	set_terrain { "Gs^Fms",
		f.all(
			f.terrain("Gs^Fp,Gs^Fmf"),
			f.adjacent(f.terrain("Gs^Ft,Hh^Ft"))
		),
	}
	set_terrain { "Gs^Fms",
		f.all(
			f.terrain("Gs^Fmf"),
			f.adjacent(f.terrain("Gg^Fet"))
		),
	}
	set_terrain { "Hh^Fms",
		f.all(
			f.terrain("Hh^Fp,Hh^Fmf"),
			f.adjacent(f.terrain("Gs^Ft,Hh^Ft"))
		),
	}
	set_terrain { "Hh^Fms",
		f.all(
			f.terrain("Hh^Fmf"),
			f.adjacent(f.terrain("Gg^Fet"))
		),
	}

	-- Soft pines clusters
	set_terrain { "Hh^Fms",
		f.all(
			f.terrain("Hh^Fp"),
			f.adjacent(f.terrain("Hh^Fp,G*^Fp"), nil, "5-6")
		),
		fraction = 5,
	}
	set_terrain { "Gg^Fms",
		f.all(
			f.terrain("G*^Fp"),
			f.adjacent(f.terrain("Hh^Fp,G*^Fp"), nil, "5-6")
		),
		fraction = 5,
	}

	-- better road near castle
	local rad = mathx.random_choice("1,2,3,3,3,3,4,4")
	set_terrain { "Rr",
		f.all(
			f.terrain("Re"),
			f.radius(rad, f.terrain("Ch"))
		),
	}

	-- randomize a few forest
	set_terrain { "Gg^Fds",
		f.all(
			f.terrain("G*^F*"),
			f.radius(4, f.terrain("Ql,Mv"))
		),
		fraction = 9,
	}
	set_terrain { "Hh^Fds",
		f.all(
			f.terrain("H*^F*"),
			f.radius(4, f.terrain("Ql,Mv"))
		),
		fraction = 9,
	}
	set_terrain { "Gg^Fds,Gg^Fms,Gg^Fp,Gg^Fmf,Gs^Fms,Gs^Fmf,Gs^Fp",
		f.all(
			f.terrain("G*^F*"),
			f.none(
				f.terrain("G*^Fet")
			)
		),
		fraction = 11,
	}
	set_terrain { "Hh^Fds,Hh^Fms,Hh^Fp,Hh^Fmf,Hh^Fms,Hh^Fmf,Hh^Fp",
		f.terrain("Hh*^F*"),
		fraction = 11,
	}

	-- chances of few elvish castles
	wct_possible_map4_castle("Cv^Fds", 1)
	-- High quality villages near castle or stone road
	set_terrain { "Rr^Vhc",
		f.all(
			f.terrain("G*^Vh"),
			f.radius(1, f.terrain("Ch,Cha,Kh*^*,Rr"))
		),
	}

	-- chances flowers
	local terrain_to_change = wct_store_possible_flowers("Rr^Vhc")
	while #terrain_to_change > 0 and mathx.random(10) > 5 do
		local loc = terrain_to_change[mathx.random(#terrain_to_change)]
		map[loc] = "^Efm"
		terrain_to_change = wct_store_possible_flowers("Rr^Vhc")
	end
	-- 1.12 new forest
	set_terrain { "*^Ftp",
		f.terrain("*^Ft"),
		fraction = 2,
		layer = "overlay",
	}

	if mathx.random(20) == 1 then
		set_terrain { "*^Ftr",
			f.terrain("G*^Fds"),
			layer = "overlay",
		}
	end
	if mathx.random(20) == 1 then
		set_terrain { "*^Ftr",
			f.terrain("G*^Fms"),
			fraction = 3,
			layer = "overlay",
		}
	end
	if mathx.random(20) == 1 then
		set_terrain { "*^Ftr",
			f.terrain("G*^Fp"),
			fraction = 5,
			layer = "overlay",
		}
	end
	if mathx.random(20) == 1 then
		wct_map_decorative_docks()
	end
	if mathx.random(20) == 1 then
		wct_change_map_water("g")
	end
end

function wct_map_4a_post_bunus_decoration()
	wct_map_cave_path_to("Re")
	wct_noise_snow_to("Gd")
end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"Thermal")
	world_conquest_tek_map_noise_classic("Gs^Fp")
	wct_enemy_castle_expansion()
	world_conquest_tek_map_repaint_4a()
	world_conquest_tek_bonus_points()
	wct_map_4a_post_bunus_decoration()
end
