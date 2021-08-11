-- Springs
function world_conquest_tek_map_repaint_2a()
	world_conquest_tek_map_rebuild("Uu,Uu^Tf,Uh,Uu^Tf,Uu,Uh,Ql,Qxu,Xu,Ww,Ww", 3)
	world_conquest_tek_map_decoration_2a()
	world_conquest_tek_map_dirt("Gg^Tf,Gg^Tf,Gg^Tf,Gs^Tf")
end

function world_conquest_tek_map_decoration_2a()
	set_terrain { "Gs^Fms",
		f.all(
			f.terrain("Gs^Ft"),
			f.none(
				f.adjacent(f.terrain("Ss,Ds,Dd,Hd,Dd^Do,Ww,Wo"), nil, "2-6")
			)
		),
	}
	set_terrain { "Hh^Fms",
		f.terrain("Hh^Ft"),
	}

	-- chances of tropical palm forest near caves
	if mathx.random(2) == 1 then
		local terrain_to_change = map:find(f.all(
			f.terrain("Hh*^F*"),
			f.adjacent(f.terrain("Xu,U*^*,Mv,Ql,Qxu"))
		))
		if #terrain_to_change > 3 then
			local r = mathx.random(0, #terrain_to_change - 3)
			for i = 1, r do
				local loc = terrain_to_change[mathx.random(#terrain_to_change)]
				map[loc] = "Hh^Ftp"
			end
		end
	end
	if mathx.random(2) == 1 then
		local terrain_to_change = map:find(f.all(
			f.terrain("G*^F*"),
			f.adjacent(f.terrain("Xu,U*^*,Mv,Ql,Qxu"))
		))
		if #terrain_to_change > 3 then
		local r = mathx.random(0, #terrain_to_change - 3)
			for i = 1, r do
				local loc = terrain_to_change[mathx.random(#terrain_to_change)]
				map[loc] = "Gs^Ftp"
			end
		end
	end
	-- tropical forest near sand, swamp or water
	set_terrain { "Gs^Ftd",
		f.all(
			f.terrain("Gs^Fp"),
			f.adjacent(f.terrain("Ss,Ds,Dd,Hd,Dd^Do,Ww,Wo"), nil, "3-6"),
			f.adjacent(f.terrain("Aa,Aa^Fpa,Ha,Aa^Vea,Aa^Vha,Ms"), nil, 0)
		),
	}
	set_terrain { "Gg^Fds",
		f.all(
			f.terrain("Gs^Fp"),
			f.radius(2, f.all(
				f.terrain("Gg^Fet"),
				f.none(
					f.radius(3, f.terrain("Ds,Dd,Hd"))
				)
			))
		),
	}

	-- tropical forest near lava and water
	set_terrain { "Gs^Ft",
		f.all(
			f.terrain("Gs^Fp"),
			f.radius(3, f.terrain("Ql,Mv")),
			f.radius(3, f.terrain("Ww,Wo,Wwr"))
		),
	}
	set_terrain { "Hh^Ft",
		f.all(
			f.terrain("Hh^Fp"),
			f.radius(3, f.terrain("Ql,Mv")),
			f.radius(3, f.terrain("Ww,Wo,Wwr"))
		),
	}
	set_terrain { "Hh^Fds",
		f.all(
			f.terrain("Hh^Fp"),
			f.radius(2, f.all(
				f.terrain("Gg^Fet"),
				f.none(
					f.radius(1, f.terrain("Ds,Dd,Hd"))
				)
			))
		),
	}
	set_terrain { "Hh^Fds",
		f.all(
			f.terrain("Hh^Fp"),
			f.radius(2, f.all(
				f.terrain("Ql,Mv"),
				f.none(
					f.radius(1, f.terrain("Ds,Dd,Hd"))
				)
			))
		),
	}
	set_terrain { "Hh^Fds",
		f.all(
			f.terrain("Hh^Fp"),
			f.radius(3, f.all(
				f.terrain("Ql,Uu,Hu,Uu^Tf,Qxu,Mv"),
				f.none(
					f.radius(3, f.terrain("Ds,Dd,Hd"))
				)
			)),
			f.radius(3, f.terrain("Ww"))
		),
	}
	set_terrain { "Hh^Fms",
		f.all(
			f.terrain("Hh^Fp"),
			f.adjacent(f.terrain("Gg^Fds"))
		),
	}
	set_terrain { "Gg^Fms",
		f.all(
			f.terrain("Gg^Fds"),
			f.adjacent(f.terrain("Gs^Fp"))
		),
	}

	-- randomize a few forest
	set_terrain { "Gg^Fet,Gg^Fds,Gg^Fms,Gg^Fp,Gg^Fds,Gg^Fms,Gs^Fds,Gs^Ftp",
		f.terrain("G*^F*"),
		fraction = 11,
	}
	set_terrain { "Hh^Fds,Hh^Fms,Hh^Fp,Hh^Fds,Hh^Fms,Hh^Fds,Hh^Ftp",
		f.terrain("Hh*^F*"),
		fraction = 11,
	}

	-- fords

	local r = mathx.random(0, 4)
	for i = 1, r do
		set_terrain { "Wwf",
			f.all(
				f.terrain("Gg,Gs"),
				f.adjacent(f.terrain("Wwf,Ww*^*,Wwo")),
				f.adjacent(f.terrain("U*^*,M*^*,Xu"))
			),
		}

	end
	-- chances flowers
	local terrain_to_change = wct_store_possible_flowers("G*^Fet")
	while #terrain_to_change > 0 and mathx.random(10) ~= 1 do
		local loc = terrain_to_change[mathx.random(#terrain_to_change)]
		map[loc] = "^Efm"
		local terrain_to_change = wct_store_possible_flowers("G*^Fet")
	end
	-- extra coast
	set_terrain { "Ww,Ww,Ww,Wwr",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.terrain("Ds"))
		),
		fraction_rand = "10..12",
	}

	wct_map_reduce_castle_expanding_recruit("Ce", "Re")
	if mathx.random(5) ~= 1 then
		wct_map_decorative_docks()
	end
end

function wct_map_2a_post_bunus_decoration()
	wct_map_cave_path_to("Rb")
	wct_noise_snow_to("Wwf")
end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"water^Springs")
	world_conquest_tek_map_noise_classic("Gs^Fp")
	wct_enemy_castle_expansion()
	world_conquest_tek_map_repaint_2a()
	world_conquest_tek_bonus_points()
	wct_map_2a_post_bunus_decoration()
	wct_map_enemy_themed("undead", "Soulless", "ha", "Aa^Vha", 12)
end
