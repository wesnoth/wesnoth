-- Clayey
function world_conquest_tek_map_repaint_2f()
	wct_reduce_wall_clusters("Uu,Uu^Tf,Uh,Uu^Tf,Uu,Uh,Ql,Uu,Xu,Uu,Ww")
	set_terrain { "Xu",
		f.all(
			f.adjacent(f.terrain("Uu^*")),
			f.terrain("Mm^Xm")
		),
	}

	wct_volcanos()
	wct_volcanos_dirt()
	-- fix water types
	set_terrain { "Wwf",
		f.terrain("Ww"),
	}
	set_terrain { "Wo",
		f.terrain("Wog"),
	}
	set_terrain { "Ww",
		f.terrain("Wwg^*"),
		layer = "base",
	}
	set_terrain { "Sm",
		f.all(
			f.adjacent(f.terrain("Dd^*,Rd^*")),
			f.terrain("Wwf")
		),
	}
	set_terrain { "Ss",
		f.all(
			f.adjacent(f.terrain("Gg^*")),
			f.adjacent(f.terrain("Wwf"), nil, "3-6"),
			f.terrain("Wwf")
		),
	}
	set_terrain { "Ww",
		f.all(
			f.adjacent(f.terrain("Wo")),
			f.terrain("Wwf")
		),
	}

	-- extra random rough terrain
	set_terrain { "*^Fp,*^Fp,*^Fp,*^Fp,*^Fp,*^Fp,*^Fp,*^Fpa,*^Fpa,*^Fma,*^Fda",
		f.terrain("G*,R*,Dd"),
		fraction = 3,
		layer = "overlay",
	}
	set_terrain { "Mm",
		f.terrain("*^Fda"),
	}
	set_terrain { "Dd^Fetd,Hd",
		f.terrain("Dd^Fma"),
	}
	set_terrain { "Hh^Fds",
		f.terrain("G*^Fma"),
	}
	set_terrain { "Hh^Fms",
		f.terrain("Rb^Fma"),
	}
	set_terrain { "Hh^Fp",
		f.terrain("*^Fma"),
	}
	set_terrain { "Hd",
		f.terrain("Dd^Fpa"),
	}
	set_terrain { "Hhd",
		f.terrain("Rd^Fpa"),
	}
	set_terrain { "Hh",
		f.terrain("*^Fpa"),
	}
	set_terrain { "Dd^Fdw",
		f.terrain("Dd^Fp"),
	}
	set_terrain { "Rd^Fmw",
		f.terrain("Rd^Fp"),
	}
	set_terrain { "Rb^Fms",
		f.terrain("Rb^Fp"),
	}
	set_terrain { "Gs^Fds",
		f.terrain("Gs^Fp"),
	}
	set_terrain { "Gg^Ftr",
		f.terrain("Gg^Fp"),
	}
	set_terrain { "*^Fet",
		f.terrain("G*^F*"),
		fraction = 9,
		layer = "overlay",
	}
	-- villages variations
	set_terrain { "Gg^Ve",
		f.all(
			f.terrain("Gg^Vht"),
			f.adjacent(f.terrain("*^F*"))
		),
		fraction = 2,
	}
	set_terrain { "Gg^Vh",
		f.terrain("Gg^Vht"),
		fraction = 2,
	}
	set_terrain { "Gg^Vc",
		f.terrain("Gg^Vht"),
		fraction = 2,
	}

end

function wct_map_2f_post_bunus_decoration()
	-- earthy caves
	set_terrain { "Uue",
		f.terrain("Uu^*"),
		layer = "base",
	}
	set_terrain { "Xuce",
		f.terrain("Xu"),
	}
end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"Clayey")
	wct_enemy_castle_expansion()
	world_conquest_tek_map_repaint_2f()
	world_conquest_tek_bonus_points("clayey")
	wct_map_2f_post_bunus_decoration()
end
