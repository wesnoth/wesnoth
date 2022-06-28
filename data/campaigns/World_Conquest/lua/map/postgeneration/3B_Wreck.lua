-- Wreck
function world_conquest_tek_map_repaint_3b()
	world_conquest_tek_map_rebuild("Uu,Uu^Tf,Uh,Uu^Tf,Uu,Uu^Tf,Uh,Ql,Qxu,Xu", 3)
	world_conquest_tek_map_decoration_3b()
	world_conquest_tek_map_dirt("Gs^Tf")
end

function world_conquest_tek_map_decoration_3b()
	set_terrain { "Gs^Vd",
		f.all(
			f.terrain("Gs^Vht"),
			f.adjacent(f.terrain("Gs^Ft"), nil, 0)
		),
	}
	set_terrain { "Hh^Vo",
		f.all(
			f.terrain("Hh^Vhh"),
			f.adjacent(f.terrain("Mm,Mm^Xm"), nil, 0)
		),
	}

	wct_map_reduce_castle_expanding_recruit("Chs", "Re")
	-- hunging bridges between high terrain
	wct_hunging_bridge("\\", "nw,se")
	wct_hunging_bridge("|", "n,s")
	wct_hunging_bridge("/", "ne,sw")
	-- decorative orc castles near orc villages
	set_terrain { "Co",
		f.all(
			f.terrain("Chs"),
			f.adjacent(f.terrain("Chs,Ch,Kh"), nil, 0),
			f.radius(3, f.terrain("Hh^Vo"))
		),
	}
	set_terrain { "Gs",
		f.terrain("Gg"),
		layer = "base",
	}

	-- 1.12 new forest
	set_terrain { "Gs^Ftp",
		f.terrain("*^Fet"),
	}
	set_terrain { "*^Ftr",
		f.terrain("*^Ft,*^Fet"),
		fraction = 2,
		layer = "overlay",
	}

	wct_change_map_water("t")
end

function wct_hunging_bridge(bridge, directions)
	set_terrain { "*^Bh" .. bridge,
		f.any(
			f.all(
				f.terrain("*^Bw" .. bridge),
				f.adjacent(f.terrain("C*,K*,Hh^*,M*^*"), directions, 2)
			),
			f.all(
				f.terrain("*^Bw" .. bridge,
					f.adjacent(f.terrain("C*,K*,Hh^*,M*^*"), directions, 1),
					f.adjacent(f.all(
						f.terrain("*^Bw" .. bridge),
						f.adjacent(f.terrain("C*,K*,Hh^*,M*^*"), directions, 1)
					), directions, 1)
				)
			)
		),
		layer = "overlay",
	}
end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"civilization^Wreck")
	world_conquest_tek_map_noise_classic("Gs^Ft")
	wct_enemy_castle_expansion()
	world_conquest_tek_map_repaint_3b()
	world_conquest_tek_bonus_points()
	wct_map_cave_path_to("Rb")
	wct_map_enemy_themed("drake", "Fire Guardian", "d", "Gs^Vd", 8)
	wct_map_enemy_themed("lizard", "Fire Guardian", "d", "Gs^Vd", 8)
end
