function world_conquest_tek_map_noise_proxy(radius, fraction, terrain)
	local terrain_to_change = map:find(f.terrain(terrain))
	local nop_filter = wesnoth.map.filter(f.all())
	mathx.shuffle(terrain_to_change)
	for terrain_i = 1, math.ceil(#terrain_to_change / fraction) do
		local loc_a = terrain_to_change[terrain_i]
		local terrain_to_swap_b = map:find_in_radius({loc_a}, radius, nop_filter)
		
		terrain_to_swap_b  = map:find(f.all(
			f.none(f.is_loc(loc_a)),
			f.terrain(terrain)
		), terrain_to_swap_b)
		
		if #terrain_to_swap_b > 0 then
			local loc_b = terrain_to_swap_b[mathx.random(#terrain_to_swap_b)]
			local terrain_a, terrain_b = map[loc_a], map[loc_b]
			map[loc_a] = terrain_b
			map[loc_b] = terrain_a
		end
	end
end

function world_conquest_tek_map_noise_classic(tree)
	-- generic utility to soft chunks of rought terrain and add extra rought on grass zones.
	-- created for classic maps postgeneration.
	
	set_terrain_simul {
		{ "Gs^Ft,Gs^Ft,Hh,Hh,Hh",
			f.terrain("Gs"),
			per_thousand = 125,
			exact = false,
		},
		{ "Hd,Hd,Hd,Hd,Hd,Hd,Hd,Hd,Dd^Dr,Gs^Ft,Dd^Do",
			f.terrain("Dd"),
			per_thousand = 222,
			exact = false,
		},
		{ string.format("%s,%s,Hh,Hh,Hh", tree, tree),
			f.terrain("Gg"),
			per_thousand = 222,
			exact = false,
		},
		{ string.format("Aa^Fpa,Aa^Fpa,%s,Ha,Ha,Ha", noise_snow),
			f.terrain("Aa"),
			per_thousand = 222,
			exact = false,
		},
		{ "Ur,Uu^Tf,Uh,Uu",
			f.terrain("Uu,Uh,Uu^Tf"),
			per_thousand = 420,
			exact = false,
		},
		{ string.format("Gs,Gs,Gs,Gs,Hh,Hh,Hh,%s,%s,%s,Mm^Xm", tree, tree, tree),
			f.terrain("Mm"),
			per_thousand = 500,
			exact = false,
		},
		{ string.format("Gs,Gs,Gs,Gs,Gs,Gs,%s,%s,%s,Mm", tree, tree,tree),
			f.terrain("Hh"),
			per_thousand = 610,
			exact = false,
		},
		{ string.format("Aa,Aa,Aa,Aa,%s,Ai,Aa^Fpa,Aa^Fpa,Aa^Fpa,Mm", noise_snow),
			f.terrain("Ha"),
			per_thousand = 610,
			exact = false,
		},
		{ "Gs,Gs,Gs,Gs,Gs,Gs,Gg^Fet,Hh,Hh,Hh,Hh",
			f.terrain("Gs^Fp,Gs^Ft,Gg^Fet"),
			per_thousand = 680,
			exact = false,
		},
		{ string.format("Aa,Aa,Aa,Aa,Ai,%s,Gg^Fet,Ha,Ha,Ha,Ha", noise_snow),
			f.terrain("Aa^Fpa"),
			per_thousand = 680,
			exact = false,
		},
		{ "Rd,Rd",
			f.all(
				f.terrain("Ww"),
				f.adjacent(f.terrain("Ql,Qlf"))
			),
			per_thousand = 125,
			exact = false,
		},
		{ "Mm,Mm",
			f.all(
				f.terrain("Mm^Xm"),
				f.adjacent(f.terrain("Xu")),
				f.adjacent(f.terrain("Uu,Uh,Uu^Tf")),
				f.adjacent(f.none(
					f.terrain("Uu,Uh,Uu^Uh,Xu")
				))
			),
			per_thousand = 125,
			exact = false,
		}
	}
end

function world_conquest_tek_map_noise_maritime()
	-- variation of classic noise, created for maritime generator
	set_terrain_simul {
		{ "Gs^Fp,Gs^Fp,Hh,Hh,Hh",
			f.terrain("Gs"),
			per_thousand = 125,
			exact = false,
		},
		{ "Hhd,Hhd,Hhd,Hd,Rd,Rd,Rd,Rd,Rd,Rd,Dd^Dr,Rd^Fet,Rd^Fdw,Dd^Do",
			f.terrain("Dd"),
			per_thousand = 125,
			exact = false,
		},
		{ "Gg^Fet,Gg^Fet,Hh,Hh,Hh",
			f.terrain("Gg"),
			per_thousand = 222,
			exact = false,
		},
		{ "Gd^Fmw,Gd^Fp,Hh^Fmw,Hh,Mm,Hh,Hh",
			f.terrain("Gd"),
			per_thousand = 222,
			exact = false,
		},
		{ "Aa^Fpa,Aa^Fpa,Wwf,Ha,Ha,Ha",
			f.terrain("Aa"),
			per_thousand = 222,
			exact = false,
		},
		{ "Ur,Uu^Tf,Uh,Uu",
			f.terrain("Uu,Uh,Uu^Tf"),
			per_thousand = 410,
			exact = false,
		},
		{ "Gs,Gs,Gs,Gs,Hh,Hh,Hh,Gs^Fp,Gs^Fp,Gs^Fp,Mm^Xm",
			f.terrain("Mm"),
			per_thousand = 460,
			exact = false,
		},
		{ "Gs,Gs,Gs,Gs,Gs,Gs,Gs^Fp,Gs^Fp,Gs^Fp,Mm",
			f.terrain("Hh"),
			per_thousand = 590,
			exact = false,
		},
		{ "Aa,Aa,Aa,Aa,Wwf,Ai,Aa^Fpa,Aa^Fpa,Aa^Fpa,Mm",
			f.terrain("Ha"),
			per_thousand = 610,
			exact = false,
		},
		{ "Mm,Hh,Hh",
			f.terrain("G*^F*"),
			per_thousand = 125,
			exact = false,
		},
		{ "Aa,Aa,Aa,Aa,Ai,Wwf,Gg^Fet,Ha,Ha,Ha,Ha",
			f.terrain("Aa^Fpa"),
			per_thousand = 680,
			exact = false,
		},
		{ "Mm,Mm",
			f.all(
				f.terrain("Mm^Xm"),
				f.adjacent(f.terrain("Xu")),
				f.adjacent(f.terrain("Uu,Uh,Uu^Tf")),
				f.adjacent(f.none(
					f.terrain("Uu,Uh,Uu^Uh,Xu")
				))
			),
			per_thousand = 125,
			exact = false,
		}
	}
end

noise_snow = "Sm^Em"

function wct_noise_snow_to(terrain)
	set_terrain { terrain,
		f.terrain(noise_snow),
	}
end
