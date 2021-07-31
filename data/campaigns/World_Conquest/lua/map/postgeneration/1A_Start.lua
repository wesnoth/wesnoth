
function world_conquest_tek_map_repaint_1()
	world_conquest_tek_map_rebuild("Uu,Uu^Tf,Uh,Uu^Tf,Uu,Ai,Uh,Ql,Qxu,Xu", 3)
	world_conquest_tek_map_decoration_1()
	world_conquest_tek_map_dirt("Gg^Tf")
end

function world_conquest_tek_map_decoration_1()
	set_terrain { "Gs^Fdw",
		f.terrain("Gs^Ft"),
	}
	set_terrain { "Gs^Fmf",
		f.all(
			f.terrain("Gs^Fdw"),
			f.adjacent(f.terrain("Ww,Wo,Ss,Wwr,Gg^Fet"))
		),
	}
	set_terrain { "Hh^Fmf",
		f.terrain("Hh^Ft"),
	}
	set_terrain { "Gs^Fmw",
		f.all(
			f.terrain("Gs^Fp"),
			f.none(
				f.adjacent(f.terrain("Ww,Wo,Ss,Wwr"))
			)
		),
	}
	set_terrain { "Hh^Fmw",
		f.terrain("Hh^Fp"),
	}
	-- the old code used 'Hh^Fmd', 'Gg^Fmd' which is invaldi terrain, i assumed hea meant "Fmw"
	set_terrain { "Hh^Fmw",
		f.all(
			f.terrain("Hh^F*,!,Hh^Fmf"),
			f.radius(2, f.terrain("Ql,Mv"))
		),
	}
	set_terrain { "Gg^Fmw",
		f.all(
			f.terrain("G*^F*,!,Gs^Fmf"),
			f.radius(2, f.terrain("Ql,Mv"))
		),
	}

	set_terrain { "Gs^Vo",
		f.terrain("Gs^Vht"),
	}

	-- tweak roads
	if mathx.random(20) ~= 1 then
		local rad = mathx.random(5, 9)
		local ter = mathx.random_choice("Ch*^*,Kh*^*,Ch*^*,Kh*^*")
		set_terrain { "Rb",
			f.all(
				f.terrain("Re"),
				f.none(
					f.radius(rad, f.terrain(ter))
				)
			),
		}
	end
	-- chances of fords
	local terrain_to_change = wct_store_possible_encampment_ford();

	while #terrain_to_change > 0 and mathx.random(2) == 1 do
		local i = mathx.random(#terrain_to_change)
		map[terrain_to_change[i]] = "Wwf"
		terrain_to_change = wct_store_possible_encampment_ford()
	end

	if mathx.random(20) ~= 1 then
		wct_change_map_water("g")
	end
	-- randomize a few forest
	set_terrain { "Gg^Fms",
		f.terrain("G*^Fmw,G*^Fp"),
		fraction = 11,
	}

	-- become impassible mountains isolated walls
	set_terrain { "Ms^Xm",
		f.all(
			f.terrain("Xu"),
			f.adjacent(f.terrain("U*^*,Q*^*,Mv"), nil, 0),
			f.adjacent(f.terrain("A*^*,Ha^*,Ms^*"))
		),
	}

	if mathx.random(8) ~= 1 then
		set_terrain { "Mm^Xm",
			f.all(
				f.terrain("Xu"),
				f.adjacent(f.terrain("U*^*,Q*^*"), nil, 0)
			),
		}

	end
end

function wct_map_1_post_bunus_decoration()
	table.insert(prestart_event, wml.tag.item {
		terrain = noise_snow,
		image = "scenery/snowbits.png",
	})
	wct_noise_snow_to("Gg,Gg,Rb")
	-- some small mushrooms
	set_terrain { "Gg^Em",
		f.all(
			f.terrain("Gg,Gs"),
			f.adjacent(f.terrain("Ww*,S*^*,U*^*,Xu,Qxu"))--,
			-- todo: maybe bring this back?
			--f.none(
			--	f.find_in_wml("bonus.point")
			--)
		),
		fraction_rand = "12..48",
	}

	wct_map_cave_path_to("Rb")
end

function wct_map_1_post_castle_expansion_fix()
	wct_map_reduce_castle_expanding_recruit("Ce", "Wwf")
	local r = mathx.random_choice("Ch,Ch,Ch,Chw,Chw,Chs,Ce,Wwf")
	set_terrain { r,
		f.all(
			f.terrain("Ce"),
			f.adjacent(f.terrain("Ce"))
		),
	}

end

function wct_store_possible_encampment_ford()
	return map:find(f.all(
		f.terrain("Ww"),
		f.adjacent(f.terrain("Ce,Chw,Chs*^V*")),
		f.adjacent(f.terrain("W*^B*,Wo"), nil, 0)
	))
end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"Start")

	world_conquest_tek_map_noise_classic("Gs^Fp")
	world_conquest_tek_map_repaint_1()
	world_conquest_tek_bonus_points()
	wct_map_1_post_bunus_decoration()

	-- the original code did the choose difficulty dialog here.
	-- claiming it would ne needed before wct_enemy_castle_expansion
	-- but i don't know why.
	wct_enemy_castle_expansion()
	wct_map_1_post_castle_expansion_fix()

end
