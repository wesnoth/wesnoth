-- Provinces

function world_conquest_tek_map_repaint_2d()
	world_conquest_tek_map_decoration_2d()
	world_conquest_tek_map_rebuild("Uu,Uu^Tf,Uu^Dr,Ql,Qxu,Cud", 2)
	set_terrain { "Uh",
		f.all(
			f.adjacent(f.terrain("Ch*,Kh*,Cd,Cv^Fds,Ce,Co*")),
			f.terrain("Cud")
		),
	}

	world_conquest_tek_map_dirt("Gg^Tf,Gg^Tf,Gg^Tf,Gs^Tf")
end

function wct_provinces_castle(terrain_to_change, terrain)
	if #terrain_to_change > 0 then
		local loc = terrain_to_change[mathx.random(#terrain_to_change)]
		map[loc] = terrain
	end
end

function wct_provinces_castle_separation()
	return f.none(f.radius(8, f.terrain("C*^*,K*^*")))
end

function world_conquest_tek_map_decoration_2d()
	for i = 1, 2 do
		local terrain_to_change = map:find(f.all(
			f.terrain("Hh,Mm,Mm^Xm,Xu,*^F*"),
			f.adjacent(f.terrain("D*^*,Hd*^*")),
			wct_provinces_castle_separation()
		))
		wct_provinces_castle(terrain_to_change, "Cd")

		local terrain_to_change = map:find(f.all(
			f.terrain("Hh^F*"),
			wct_provinces_castle_separation()
		))
		wct_provinces_castle(terrain_to_change,  "Cv^Fds")

		local terrain_to_change = map:find(f.all(
			f.terrain("Hh"),
			f.none(f.radius(2, f.terrain("D*^*,Hd*^*"))),
			wct_provinces_castle_separation()
		))
		wct_provinces_castle(terrain_to_change,  "Ce")

		local terrain_to_change = map:find(f.all(
			f.terrain("Mm"),
			wct_provinces_castle_separation()
		))
		wct_provinces_castle(terrain_to_change,  "Co")
	end

	set_terrain { "*^Fds",
		f.terrain("*^Ft"),
		layer = "overlay",
	}
	set_terrain { "*^Fms",
		f.terrain("*^Fp"),
		fraction = 2,
		layer = "overlay",
	}


	-- villages
	set_terrain { "*^Vc",
		f.all(
			f.terrain("*^V*"),
			f.none(
				f.terrain("W*^*,S*^*,U*^*,D*^*,A*^*")
			)
		),
		layer = "overlay",
	}
	set_terrain { "*^Vh",
		f.all(
			f.terrain("*^V*"),
			f.none(
				f.terrain("W*^*,S*^*,U*^*,D*^*")
			),
			f.radius(6, f.terrain("Ch*,Kh*"))
		),
		layer = "overlay",
	}
	set_terrain { "Rr^Vhc",
		f.all(
			f.terrain("*^V*"),
			f.none(
				f.terrain("W*^*,S*^*,U*^*,D*^*")
			),
			f.radius(1, f.terrain("Ch*,Kh*"))
		),
	}
	set_terrain { "*^Ve",
		f.all(
			f.terrain("*^V*"),
			f.none(
				f.terrain("W*^*,S*^*,U*^*")
			),
			f.radius(5, f.terrain("Cv*^*"))
		),
		layer = "overlay",
	}
	set_terrain { "*^Vd",
		f.all(
			f.terrain("*^V*"),
			f.none(
				f.terrain("W*^*,S*^*,U*^*")
			),
			f.radius(5, f.terrain("Cd"))
		),
		layer = "overlay",
	}
	set_terrain { "*^Vo",
		f.all(
			f.terrain("*^V*"),
			f.none(
				f.terrain("W*^*,S*^*,U*^*")
			),
			f.radius(5, f.terrain("Co"))
		),
		layer = "overlay",
	}
	set_terrain { "*^Vl",
		f.all(
			f.terrain("*^V*"),
			f.none(
				f.terrain("W*^*,S*^*,U*^*")
			),
			f.radius(5, f.terrain("Ce"))
		),
		layer = "overlay",
	}
	set_terrain { "Re^Vhh",
		f.all(
			f.terrain("*^Vc"),
			f.radius(2, f.terrain("Re"))
		),
	}
	set_terrain { "*^Vht",
		f.all(
			f.terrain("G*^Vc"),
			f.radius(1, f.terrain("S*^*"))
		),
		layer = "overlay",
	}
	set_terrain { "*^Vud",
		f.terrain("*^Vu"),
		layer = "overlay",
	}

	wct_road_to_village("Re", "Re^Vhh")
end

function wct_map_2d_post_bunus_decoration()
	wct_map_cave_path_to("Urb")
	wct_noise_snow_to("Gd")
end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"Provinces")
	world_conquest_tek_map_noise_classic("Gs^Fp")
	wct_enemy_castle_expansion()
	world_conquest_tek_map_repaint_2d()
	world_conquest_tek_bonus_points()
	wct_map_2d_post_bunus_decoration()
end
