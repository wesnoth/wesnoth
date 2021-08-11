-- Changes terrain when unit die on that tile. works mostly independend of the rest of the addon.

local on_event = wesnoth.require("on_event")

local snow = {
	"misc/blank-hex.png~BLIT(terrain/wct-snowcrater1.png~SCALE(63,63),5,5)",
	"misc/blank-hex.png~BLIT(terrain/wct-snowcrater2.png~SCALE(63,63),5,5)",
	"misc/blank-hex.png~BLIT(terrain/wct-snowcrater3.png~SCALE(54,54),9,9)"
}

local ice = {
	"misc/blank-hex.png~BLIT(terrain/water/ford-tile.png~MASK(terrain/wct-crack-7.png))",
	"misc/blank-hex.png~BLIT(terrain/water/ford-tile.png~MASK(terrain/wct-crack-2.png))",
	"misc/blank-hex.png~BLIT(terrain/water/ford-tile.png~MASK(terrain/wct-crack-3.png))",
	"misc/blank-hex.png~BLIT(terrain/water/ford-tile.png~MASK(terrain/wct-crack-4.png))",
	"misc/blank-hex.png~BLIT(terrain/water/ford-tile.png~MASK(terrain/wct-crack-5.png))"
}

--replaces terrain fo the wct  custom terrain mod.
local function wct_map_custom_ruin_village(loc)
	local map = wesnoth.current.map
	-- TODO: enable once https://github.com/wesnoth/wesnoth/issues/4894 is fixed.
	if false then
		if loc:matches{terrain = "*^Vh,*^Vha"} then
			map[loc] = "^Vhr"
		end
		if loc:matches{terrain = "*^Vhc,*^Vhca"} then
			map[loc] = "^Vhr"
		end
	end
end

on_event("die", function(cx)
	local map = wesnoth.current.map
	local loc = wesnoth.map.get(cx.x1, cx.y1)
	if wml.variables.wc2_config_enable_terrain_destruction == false then
		return
	end
	if not loc:matches{terrain = "K*^*,C*^*,*^Fet,G*^F*,G*^Tf,A*,*^B*,Rrc,Iwr,*^Vhh,*^Vh*,*^Fda*"} then
		return
	end
	local function item(image)
		wesnoth.wml_actions.item {
			x = cx.x1,
			y = cx.y1,
			image = image,
			z_order = -10,
		}
	end
	if loc:matches{terrain = "Kh,Kha,Kh^Vov,Kha^Vov"} then
		map[loc] = "Khr^"

	elseif loc:matches{terrain = "Ch,Cha"} then
		map[loc] = "Chr^Es"

		-- only without custom activated
	elseif loc:matches{terrain = "Ch^Vh,Ch^Vhc"} then
		map[loc] = "Chr^"

	elseif loc:matches{terrain = "Cd"} then
		map[loc] = "Cdr^Es"

	elseif loc:matches{terrain = "Cd^Vd"} then
		map[loc] = "Cdr^"

	elseif loc:matches{terrain = "Kd"} then
		map[loc] = "Kdr^Es"

	elseif loc:matches{terrain = "Gg^Fmf,Gg^Fdf,Gg^Fp,Gg^Tf,Gs^Fmf,Gs^Fdf,Gs^Fp,Gs^Tf"} then
		map[loc] = "Gll^"

	elseif loc:matches{terrain = "Cv^Fds"} then
		map[loc] "Cv^Fdw"

	elseif loc:matches{terrain = "Rr^Fet,Cv^Fet"} then
		map[loc] = "^Fetd"

	elseif loc:matches{terrain = "Aa"} then
		item(snow[mathx.random(#snow)])
	elseif loc:matches{terrain = "Ai"} then
		item(ice[mathx.random(#ice)])
	elseif loc:matches{terrain = "Ww^Bsb|,Ww^Bsb/,Ww^Bsb\\,Wwt^Bsb|,Wwt^Bsb/,Wwt^Bsb\\,Wwg^Bsb|,Wwg^Bsb/,Wwg^Bsb\\"} then
		map[loc] = "Wwf^Edt"
		wesnoth.audio.play("water-blast.wav")
		item("scenery/castle-ruins.png")
	elseif loc:matches{terrain = "Rrc"} then
		if wml.variables["bonus.theme"] == "paradise" then
			wesnoth.wml_actions.remove_item {
				x = cx.x1,
				y = cx.y1,
				image = "wc2_citadel_leanto"
			}
			item("scenery/trash.png")
			map[loc] = "Rrc^Edt"
		end
	elseif loc:matches{terrain = "Iwr"} then
		wesnoth.wml_actions.remove_item {
			x = cx.x1,
			y = cx.y1,
			image = "wc2_dock_ship"
		}
		item("scenery/trash.png")
		map[loc] = "Iwr^Edt"
	elseif loc:matches{terrain = "*^Vh,**^Vhc,*^Vha,**^Vhca,*^Fda"} then
		wct_map_custom_ruin_village(loc)
		if loc:matches{terrain = "Ch^V*"} then
			map[loc] = "Chr^"
		end
		--  TODO: enable once https://github.com/wesnoth/wesnoth/issues/4894 is fixed.
		if false then
			if loc:matches{terrain = "*^Fda"} then
				map[loc] = "^Fdw"
			end
		end
	else
		if loc:matches{terrain = "*^Vhh,*^Vhha"} then
			map[loc] = "^Vhhr"
		end
		if loc:matches{terrain = "*^Bw|,*^Bw/,*^Bw\\"} then
			map[loc] = map[loc] .. "r"
		end
	end
end)
