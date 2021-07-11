-- Wetland
function world_conquest_tek_map_repaint_3f()
	set_terrain { "Ur",
		f.terrain("U*^Tf,U*"),
		fraction = 10,
	}

	wct_reduce_wall_clusters("Uu,Uu^Tf,Uh,Uu^Tf,Uu,Uh,Uu,Uu,Qxu,Uu,Wwf")

	-- soft rough terrain generated
	local terrain_to_change = wct_store_cave_passages_candidates()
	while #terrain_to_change > 0 do
		-- the original code also did not randomize this.
		-- todo: but maybe we should? (use mathx.random(#terrain_to_change) instead of 1 here)
		map[terrain_to_change[1]] = "Mm"
		terrain_to_change = wct_store_cave_passages_candidates()
	end
	set_terrain { "Gd",
		f.terrain("Mm"),
		fraction = 4,
	}
	set_terrain { "Gd",
		f.terrain("Hh,Hh^F*,Hh^Tf"),
		fraction = 4,
	}
	set_terrain { "Ss",
		f.terrain("Gs,Gg"),
		fraction = 7,
	}
	set_terrain { "*^Ftr",
		f.terrain("G*"),
		fraction = 5,
		layer = "overlay",
	}
	set_terrain { "*^Tf",
		f.terrain("G*"),
		fraction = 13,
		layer = "overlay",
	}
	set_terrain { "Hh,Hh,Hh,Hh^Ft,Mm",
		f.terrain("G*"),
		fraction = 5,
	}
	set_terrain { "Gg^Vht,Hh^Vo",
		f.all(
			f.terrain("*^Ve"),
			f.adjacent(f.terrain("*^F*,Ss"), nil, 0)
		),
	}

	-- fix river water
	set_terrain { "Wwg",
		f.terrain("Ww^*"),
		layer = "base",
	}
	set_terrain { "Wwf",
		f.all(
			f.terrain("Wwg"),
			f.adjacent(f.terrain("Wwt^*,Wot"), nil, 0)
		),
		fraction = 3,
	}
	set_terrain { "Hh^Ftp",
		f.terrain("Hh^Fds"),
		fraction = 2,
	}

	-- fix bridge generation
	set_terrain { "Ww^Bw\\",
		f.all(
			f.terrain("Wwf"),
			f.adjacent(f.terrain("Rb"), "nw,se", nil),
			f.any(
				f.adjacent(f.terrain("*^Bw\\"), "nw,se", nil),
				f.adjacent(f.terrain("*^Bw|"), "n,s", nil),
				f.adjacent(f.terrain("*^Bw/"), "ne,sw", nil)
			)
		),
	}
	set_terrain { "Ww^Bw|",
		f.all(
			f.terrain("Wwf"),
			f.adjacent(f.terrain("Rb"), "n,s", nil),
			f.any(
				f.adjacent(f.terrain("*^Bw\\"), "nw,se", nil),
				f.adjacent(f.terrain("*^Bw|"), "n,s", nil),
				f.adjacent(f.terrain("*^Bw/"), "ne,sw", nil)
			)
		),
	}
	set_terrain { "Ww^Bw/",
		f.all(
			f.terrain("Wwf"),
			f.adjacent(f.terrain("Rb"), "ne,sw", nil),
			f.any(
				f.adjacent(f.terrain("*^Bw\\"), "nw,se", nil),
				f.adjacent(f.terrain("*^Bw|"), "n,s", nil),
				f.adjacent(f.terrain("*^Bw/"), "ne,sw", nil)
			)
		),
	}

	-- extra obstacles far from impassible and water
	set_terrain { "Ms",
		f.all(
			f.terrain("G*"),
			f.adjacent(f.terrain("M*^*,H*^*")),
			f.none(
				f.radius(8, f.terrain("X*,M*^Xm"))
			),
			f.none(
				f.radius(4, f.terrain("Ww*^*,Wo*"))
			)
		),
		fraction = 12,
	}
	set_terrain { "Wwg",
		f.all(
			f.terrain("G*"),
			f.none(
				f.radius(8, f.terrain("X*,M*^Xm"))
			),
			f.none(
				f.radius(4, f.terrain("Ww*^*,Wo*"))
			)
		),
		fraction = 12,
	}
	set_terrain { "Mm^Xm",
		f.terrain("Ms"),
	}

end

function wct_map_3f_post_bunus_decoration(bonus_points)
	-- add fords
	set_terrain { "Wwf",
		f.all(
			f.terrain("G*"),
			f.none(
				f.find_in("bonus_point")
			)
		),
		filter_extra = { bonus_point = bonus_points },
		fraction = 4,
	}

	if mathx.random(7) == 1 then
		set_terrain { "Wwf",
			f.all(
				f.terrain("G*"),
				f.adjacent(f.terrain("Wwf,Wwg")),
				f.none(
					f.find_in("bonus_point")
				)
			),
			filter_extra = { bonus_point = bonus_points },
			exact = false,
			percentage = 80,
		}

	end
	if mathx.random(7) == 1 then
		set_terrain { "Wwf",
			f.all(
				f.terrain("G*"),
				f.adjacent(f.terrain("Wwf,Wwg")),
				f.none(
					f.find_in("bonus_point")
				)
			),
			filter_extra = { bonus_point = bonus_points },
			exact = false,
			percentage = 70,
		}

	end
	-- fix water
	set_terrain { "Wwt",
		f.all(
			f.adjacent(f.terrain("!,Wot")),
			f.terrain("Wot")
		),
	}
	set_terrain { "Ww",
		f.all(
			f.adjacent(f.terrain("!,Wot,Wwt^*")),
			f.terrain("Wwt^*")
		),
		layer = "base",
	}

	-- reefs
	set_terrain { "Wwrt",
		f.all(
			f.terrain("Wwt"),
			f.adjacent(f.terrain("*^V*"), nil, 0),
			f.none(
				f.find_in("bonus_point")
			)
		),
		filter_extra = { bonus_point = {} },
		fraction_rand = "40..100",
	}

	-- fix grass generated (used to mark altitude)
	set_terrain { "Gg",
		f.terrain("Gs^*"),
		layer = "base",
	}
	set_terrain { "Gs",
		f.terrain("Gd^*"),
		layer = "base",
	}

	-- small mushrooms next to forest and swamp
	set_terrain { "^Em",
		f.all(
			f.adjacent(f.terrain("Ss")),
			f.adjacent(f.terrain("G*^Ftr")),
			f.terrain("G*")
		),
		layer = "overlay",
	}

	-- flowers on swamps
	set_terrain { "Ss^Efm",
		f.terrain("Ss"),
	}

	-- dry grass on some roads
	set_terrain { "Gd",
		f.terrain("Rb"),
		fraction_rand = "3..21",
	}

	wct_map_cave_path_to("Rb")
end

function wct_store_cave_passages_candidates()
	return map:find(f.all(
		f.terrain("Mm^Xm"),
		f.adjacent(f.terrain("Mm^Xm,Xu"), nil, "2-6"),
		f.adjacent(f.terrain("U*^*"))
	))

end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"Wetland")
	wct_enemy_castle_expansion()
	world_conquest_tek_map_repaint_3f()
	bonus_points = world_conquest_tek_bonus_points()
	wct_map_3f_post_bunus_decoration(bonus_points)
	wct_map_enemy_themed("human", "Young Ogre", "e", "Gg^Vl", 8)
end
