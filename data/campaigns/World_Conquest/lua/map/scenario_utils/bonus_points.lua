
----------------------------------------------------------
---- Chooses the location for the bonus points,       ----
---- the correct sceneries and creates events to      ----
---- place them                                       ----
----------------------------------------------------------

function random_placement(locs, num_items, min_distance, command)
	local distance = min_distance or 0
	local num_items = num_items or 1
	local allow_less = true
	local math_abs = math.abs
	local size = #locs
	for i = 1, num_items do
		if size == 0 then
			if allow_less then
				print("placed only " .. i .. " items")
				return
			else
				wml.error("[random_placement] failed to place items. only " .. i .. " items were placed")
			end
		end
		local index = mathx.random(size)
		local point = locs[index]

		command(point, i)
		if distance < 0 then
			-- optimisation: nothing to do for distance < 0
		elseif distance == 0 then
			-- optimisation: for distance = 0 we just need to remove the element at index
			-- optimisation: swapping elements and storing size in an extra variable is faster than table.remove(locs, j)
			locs[index] = locs[size]
			size = size - 1
		else
			-- the default case and the main reason why this was implemented.
			for j = size, 1, -1 do
				local x1 = locs[j][1]
				local y1 = locs[j][2]
				local x2 = point[1]
				local y2 = point[2]
				-- optimisation: same effect as "if wesnoth.map.distance_between(x1,y1,x2,y2) <= distance then goto continue; end" but faster.
				local d_x = math_abs(x1-x2)
				if d_x > distance then
					goto continue
				end
				if d_x % 2 ~= 0 then
					if x1 % 2 == 0 then
						y2 = y2 - 0.5
					else
						y2 = y2 + 0.5
					end
				end
				local d_y = math_abs(y1-y2)
				if d_x + 2*d_y > 2*distance then
					goto continue
				end
				-- optimisation: swapping elements and storing size in an extra variable is faster than table.remove(locs, j)
				locs[j] = locs[size]
				size = size - 1
				::continue::
			end
		end
	end

end

function get_f_wct_bonus_location_filter(map)
	local scenario_num = wml.variables.wc2_scenario or 1
	return f.all(
		f.terrain("G*,Hh,Uu,Uh,Dd,Ds,R*,Mm,Md,Ss,Hd,Hhd,Ww,Wwt,Wwg,Ds^Esd,Ur"),
		--no adjacent to village, deep water, chasm or walls
		f.adjacent(f.terrain("Wo*,M*^Xm,Xu*,Q*,Mv,*^V*"), nil, 0),
		--no out/at map borders
		f.x("2-" .. map.width-3),
		f.y("2-" .. map.height-3),
		f.none(
			--not too close to a keep
			f.radius(scenario_num + 4, f.terrain("K*^*")),
			--just isolated mountains
			f.all(
				f.terrain("M*"),
				f.adjacent(f.terrain("M*"))
			),
			--no swamps near sand or water
			f.all(
				f.terrain("Ss"),
				f.adjacent(f.terrain("Wo*,Ww,Wwg,Wwt,Ds,Ds^Esd"))
			),
			-- no river/lake water next to 2 coast, bridge or frozen
			-- (it means restrict lilies image)
			f.all(
				f.terrain("Ww*"),
				f.any(
					f.all(
						f.adjacent(f.terrain("Ha^*,A*^*,Ms^*,*^B*,C*")),
						f.adjacent(f.terrain("W*^*"), nil, "0-3")
					),
					f.adjacent(f.terrain("W*^*"), nil, 4)
				),
				f.none(
					f.find_in("oceanic")
				)
			)
		)
	)
end
function wct_bonus_chose_scenery(loc, theme, filter_extra)
	local terrain = map[loc]
	-- determine possible scenery values based on terrain
	local scenery = "well_g,temple,tent2_g,tent1,village,monolith3,burial"
	local terrain_to_scenery =
	{
		{
			terrain = "Re,Rd,Rb,Rr,Rrc",
			scenery ="well_r,signpost,rock_cairn,obelisk,dolmen,monolith2,temple2,shop"
		},
		{
			terrain = "Ww,Wwt,Wwg",
			scenery ="ship1,ship2"
		},
		{
			terrain = "Hh,Hhd",
			scenery ="temple,shelter,village,monolith1,monolith4"
		},
		{
			terrain = "Mm,Md",
			scenery ="mine,mine,mine,mine,mine,mine,doors,doors,doors,doors,doors,doors,temple3,temple4"
		},
		{
			terrain = "Uu",
			scenery ="altar,coffin,bones,rock_cairn_c,trapdoor,crystal,monolith2"
		},
		{
			terrain = "Ur",
			scenery ="altar,bones,rock_cairn_c,well,monolith2,monolith3,tent1"
		},
		{
			terrain = "Uh",
			scenery ="altar,coffin,bones,rock_cairn_c,trapdoor,monolith2,crystal3,burial_c"
		},
		{
			terrain = "Ds,Ds^Esd,Dd",
			scenery ="rock1,rock4,bones,tent2,tent1,burial_s"
		},
		{
			terrain = "Hd",
			scenery ="tower_r1,tower_r4,bones,tent2,tent1,rock1,rock4"
		},
		{
			terrain = "Ss",
			scenery ="bones_s,rock3,rock3,burial,lilies,bones_s"
		}
	}
	for i,v in ipairs(terrain_to_scenery) do
		for j,str in ipairs(stringx.split(v.terrain or "")) do
			if str == terrain then
				scenery = v.scenery
				goto intial_list_screated
			end
		end
	end
	::intial_list_screated::

	local function matches_location(f)
		local filter_object = wesnoth.map.filter(f, filter_extra)
		return #map:find(filter_object, {loc}) > 0
	end

	-- chance of rock cairn on isolated hills
	if matches_location(
		f.all(
			f.terrain("Hh,Hhd"),
			f.adjacent(f.terrain("H*^*"), nil, 0)
		)) then

		scenery = scenery .. "," .. "rock_cairn"
	end
	-- chance of dolmen on grass not next to forest

	if matches_location(
		f.all(
			f.terrain("G*"),
			f.adjacent(f.terrain("*^F*"), nil, 0)
		)) then
		scenery = scenery .. "," .. "dolmen_g"
	end
	-- chances of green temple on gras next to swamp, hills and forest

	if matches_location(
		f.all(
			f.terrain("G*"),
			f.adjacent(f.terrain("Ss")),
			f.adjacent(f.terrain("Hh^*,Ha^*")),
			f.adjacent(f.terrain("G*^F*,A*^F*,G*^Tf"))
		)) then

		scenery = scenery .. "," .. "temple_green_g,temple_green_g,temple_green_g,temple_green_g2,temple_green_g2"
	end
	-- chances of green temple in hills next to swamp or cold

	if matches_location(
		f.all(
			f.terrain("Hh"),
			f.adjacent(f.terrain("Ss,Ai,A*^*,Ha^*,Ms^*"))
		)) then

		scenery = scenery .. "," .. "temple_green_h,temple_green_h,temple_green_h,temple_green_h2,temple_green_h2"
	end
	-- chance of temple in hills next to mountain

	if matches_location(
		f.all(
			f.terrain("Hh,Hhd"),
			f.adjacent(f.terrain("M*^*"))
		)) then

		scenery = scenery .. "," .. "temple4,temple4"
	end
	-- chances of detritus and lilies on some swamps

	if matches_location(
		f.all(
			f.terrain("Ss"),
			f.adjacent(f.terrain("*^F*,C*^*,K*^*"), nil, 0)
		)) then

		scenery = scenery .. "," .. "detritus,detritus2,lilies_s"
	end
	-- chances of buildings next to road
	if theme ~= "volcanic" and theme ~= "clayey" and theme ~= "wild" then

		if matches_location(
			f.all(
				f.terrain("G*,Hh*"),
				f.adjacent(f.terrain("R*^*,W*^Bsb*"), nil, "2-6"),
				f.adjacent(f.terrain("*^F*"), nil, 0),
				f.radius(7, f.terrain("*^Vh,*^Vhh,*^Ve,*^Vl,*^Vhc,*^Vd,*^Vy*,*^Vz*"))
			)) then

			scenery = scenery .. "," .. "rock_cairn,temple2_g,shop_g"
		end
	end
	-- chance of fancy shop on road
	if theme ~= "volcanic" and theme ~= "clayey" and theme ~= "wild" then

		if matches_location(
			f.all(
				f.terrain("R*^*"),
				f.radius(5, f.terrain("*^Vh,*^Vhh,*^Ve,*^Vl,*^Vhc,*^Vd,*^Vy*,*^Vz*"))
			)) then

			scenery = scenery .. "," .. "tent2_r"
		end
	end
	-- high chances of oak surronded by flat

	if matches_location(
		f.all(
			f.terrain("Gg,Gs,Gll"),
			f.adjacent(f.terrain("G*,R*,R*^Em,G*^Efm,Wwf,G*^Em,G*^Eff,*^Gvs,W*^B*,Ce,Ch"), nil, 6)
		)) then

		scenery = scenery .. "," .. "oak1,oak2,oak3,oak4,oak5,oak6,oak7"
	end
	-- remove chances of ships on river/lake coast for lilies

	if matches_location(
		f.all(
			f.terrain("Ww,Wwt,Wwg"),
			f.adjacent(f.terrain("W*^*"), nil, "0-3"),
			f.none(
				f.find_in("oceanic")
			)
		)) then

		scenery = "lilies"
	end
	-- different meaning for roads in some maps
	if theme == "clayey" or theme == "wild" then

		if matches_location(
			f.all(
				f.terrain("R*")
			)) then
			scenery = "well_g,temple,tent2_g,tent1,village,monolith3,burial"
		end
	end
	-- TODO: bring back?
	if false then
		if theme == "wild" then
			if matches_location(
				f.all(
					f.find_in_wml("map_data.road_in_cave")
				)) then

				scenery = "altar,bones,rock_cairn,well,monolith2,monolith3,tent1"
			end
		end
	end
	if theme == "volcanic" then
		if matches_location(
			f.all(
				f.terrain("Rd,Rb")
			)) then

			scenery = "bones,rock_cairn,well_g,monolith2,tent1,tent1,tent2,tent2_g,monolith3,well_g,rock_cairn,dolmen,monolith2,temple,dolmen_g,monolith1_r,monolith4_r"
		end
		if matches_location(
			f.all(
				f.terrain("Ur")
			)) then

			scenery = "bones,rock_cairn,well_g,monolith2,tent1,monolith3,well_g,dolmen,monolith2,temple,monolith1_r,monolith4_r"
		end
	end
	-- high chances of lighthouse next to ocean
	if matches_location(
		f.all(
			f.terrain("!,Ww*,U*,Ss"),
			f.adjacent(f.all(
				f.terrain("Ww*"),
				f.find_in("oceanic")
			))
		)) then

		scenery = scenery .. "," .. "lighthouse,lighthouse,lighthouse,lighthouse,lighthouse"
		if matches_location(
			f.all(
				f.terrain("G*^*,R*^*")
			)) then

			scenery = scenery .. "," .. "lighthouse,lighthouse"
		end
		-- high chances of light signal on cliff next to ocean

		if matches_location(
			f.all(
				f.terrain("Hh,Hhd,Mm,Md")
			)) then

			scenery = scenery .. "," .. "campfire,campfire,campfire,campfire,lighthouse,lighthouse,lighthouse,lighthouse"

			if matches_location(
				f.all(
					f.terrain("Mm,Md")
				)) then
				scenery = scenery .. "," .. "campfire,campfire,campfire,campfire,lighthouse,lighthouse,lighthouse,lighthouse"
			end
		end
	end
	-- chances of tower on dessert far from village
	if matches_location(
		f.all(
			f.terrain("Dd,Hd"),
			f.none(
				f.radius(5, f.terrain("*^V*"))
			)
		)) then

		scenery = scenery .. "," .. "tower_r1,tower_r4"
	end
	-- chance of outpost in sands
	if matches_location(
		f.all(
			f.terrain("Ds,Hd,Dd"),
			f.adjacent(f.terrain("D*^*,Hd,G*,R*,Ur"), "se", 1),
			f.adjacent(f.terrain("D*^*,H*^*,G*^*,R*,Ur,M*^*"), "nw,sw,n,s,ne", 5),
			f.adjacent(f.terrain("D*^*,Hd*^*"), nil, "1-6")
		)) then

		scenery = scenery .. "," .. "outpost,outpost"
	end
	-- chances of dead oak in desolated
	if matches_location(
		f.any(
			f.all(
				f.terrain("Dd"),
				f.adjacent(f.terrain("Dd,Ds*^*,Hd,S*"), nil, "4-6"),
				f.adjacent(f.terrain("*^F*,C*^*,K*^*"), nil, 0)
			),
			f.all(
				f.terrain("Ds,Rd"),
				f.adjacent(f.terrain("Dd,Ds*^*,Hd,S*,Rd"), nil, 6),
				f.none(
					f.radius(2, f.terrain("W*^*"))
				)
			)
		)) then

		scenery = scenery .. "," .. "oak_dead,oak_dead,oak_dead,oak_dead,oak_dead2,oak_dead2,oak_dead2,oak_dead2"
	end
	::final_pick::
	-- pick random scenery value from our list
	local res = mathx.random_choice(scenery)
	wesnoth.log("debug", "scenery:" ..  res .. " from " .. scenery)
	return res
end


function world_conquest_tek_bonus_points(theme)
	local res = {}
	local scenario_num = wml.variables.wc2_scenario or 1
	oceanic = get_oceanic()
	f_wct_bonus_location_filter = wesnoth.map.filter(get_f_wct_bonus_location_filter(map), { oceanic = oceanic })
	local possible_locs = map:find(f_wct_bonus_location_filter)
	function place_item(loc)
		scenery = wct_bonus_chose_scenery(loc, theme, { oceanic = oceanic })
		table.insert(prestart_event, wml.tag.wc2_place_bonus {
			x= loc[1],
			y= loc[2],
			scenery = scenery,
		})
		table.insert(res, loc)
	end
	random_placement(possible_locs, 3, 9 + scenario_num, place_item)
	return res
end
