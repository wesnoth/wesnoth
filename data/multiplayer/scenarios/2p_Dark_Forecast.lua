
local helper = wesnoth.require("helper")
local _ = wesnoth.textdomain 'wesnoth-multiplayer'
local T = helper.set_wml_tag_metatable {}
local on_event = wesnoth.require("on_event")

local random_spawns = {
	{
		{"Heavy Infantryman", "Shock Trooper", "Iron Mauler", "none"},
		{"Elvish Fighter", "Elvish Hero", "more", "Elvish Champion"},
		{"Goblin Spearman", "more", "more", "more"},
		{"Goblin Spearman", "Goblin Rouser", "none", "none"},
		{"Elvish Fighter", "Elvish Captain", "Elvish Marshal", "none"},
	},
	{
		{"Mage", "Red Mage", "Silver Mage", "none"},
		{"Footpad", "Outlaw", "more", "none"},
		{"Drake Fighter", "Drake Warrior", "Drake Blademaster", "none"},
		{"Walking Corpse", "more", "more", "more"},
	},
	{
		{"Merman Hunter", "Merman Spearman", "Merman Entangler", "none"},
		{"Naga Fighter", "Naga Warrior", "Naga Myrmidon", "none"},
		{"Spearman", "Pikeman", "none", "Halberdier"},
	},
	{
		{"Elvish Shaman", "Elvish Druid", "Elvish Shyde", "Elvish Sylph"},
		{"Drake Burner", "Fire Drake", "Inferno Drake", "Armageddon Drake"},
		{"Skeleton", "Revenant", "more", "Draug"},
	},
	{
		{"Giant Mudcrawler", "Sea Serpent", "none", "none"},
		{"Mudcrawler", "more", "Giant Mudcrawler", "more"},
	},
	{
		{"Ghoul", "Necrophage", "more", "none"},
		{"Elvish Archer", "Elvish Marksman", "none", "Elvish Sharpshooter"},
		{"Elvish Archer", "Elvish Ranger", "Elvish Avenger", "none"},
		{"Drake Clasher", "Drake Thrasher", "more", "Drake Enforcer"},
	},
	{
		{"Skeleton Archer", "Bone Shooter", "more", "Banebow"},
		{"Fencer", "Duelist", "none", "Master at Arms"},
		{"Drake Glider", "Sky Drake", "Hurricane Drake", "none"},
	},
	{
		{"Merman Fighter", "Merman Warrior", "more", "Merman Triton"},
		{"Dark Adept", "Dark Sorcerer", "Necromancer", "none"},
		{"Elvish Scout", "Elvish Rider", "more", "none"},
	},
	{
		{"Wose", "Elder Wose", "Ancient Wose", "none"},
		{"Orcish Archer", "Orcish Crossbowman", "Orcish Slurbow", "none"},
		{"Saurian Skirmisher", "more", "Saurian Ambusher", "more"},
	},
	{
		{"Orcish Grunt", "Orcish Warrior", "more", "none"},
		{"Vampire Bat", "Blood Bat", "more", "none"},
		{"Dwarvish Thunderer", "Dwarvish Thunderguard", "none", "none"},
		{"Peasant", "more", "more", "more"},
		{"Woodsman", "more", "Sergeant", "Orcish Ruler"},
	},
	{
		{"Dwarvish Guardsman", "Dwarvish Stalwart", "none", "none"},
		{"Bowman", "Longbowman", "more", "Master Bowman"},
		{"Troll Whelp", "Troll", "Troll Warrior", "none"},
	},
	{
		{"Orcish Assassin", "Orcish Slayer", "more", "none"},
		{"Cavalryman", "Dragoon", "more", "Cavalier"},
		{"Saurian Augur", "Saurian Soothsayer", "none", "none"},
	},
	{
		{"Wolf Rider", "Goblin Pillager", "more", "none"},
		{"Ghost", "Shadow", "more", "more"},
		{"Sergeant", "Lieutenant", "General", "Grand Marshal"},
	},
	{
		{"Gryphon Rider", "none", "more", "none"},
		{"Thief", "Rogue", "more", "Assassin"},
	},
	{
		{"Dwarvish Fighter", "Dwarvish Steelclad", "more", "Dwarvish Lord"},
		{"Poacher", "Trapper", "more", "none"},
		{"Cuttle Fish", "more", "more", "none"},
	},
	{
		{"Walking Corpse", "more", "more", "more"},
		{"Mage", "White Mage", "Mage of Light", "none"},
		{"Thug", "Bandit", "more", "none"},
	},
}

local function get_spawn_types(num_units, max_gold, unit_pool)
	local gold_left = max_gold
	local units_left = num_units
	local current_types = {}
	while gold_left > 0 and units_left > 0 do
		local unit_group = wesnoth.random(#unit_pool)
		local unit_rank = 1
		local unit_type = wesnoth.unit_types[unit_pool[unit_group][unit_rank]]
		table.insert(current_types, { group = unit_group, type =  unit_type})
		gold_left = gold_left - unit_type.cost
		units_left = units_left - 1
	end
	-- Upgrade units, eigher by replacing them with better units or by duplicating them.
	for next_rank = 2,4 do
		local next_types = {}
		for i,v in ipairs(current_types) do
			local advanceto = unit_pool[v.group][next_rank] or ""
			local unit_type = wesnoth.unit_types[advanceto]
			if unit_type then
				local upgrade_cost = math.ceil((unit_type.cost - v.type.cost) * 1.25)
				if gold_left >= upgrade_cost then
					gold_left = gold_left - upgrade_cost
					table.insert(next_types, { group = v.group, type = unit_type})
				else
					table.insert(next_types, { group = v.group, type = v.type})
				end
			elseif advanceto == "more" then
				local upgrade_cost = v.type.cost + 2
				if gold_left >= upgrade_cost then
					gold_left = gold_left - upgrade_cost
					table.insert(next_types, { group = v.group, type = v.type})
				end
				table.insert(next_types, { group = v.group, type = v.type})
			else
				table.insert(next_types, { group = v.group, type = v.type})
			end
		end
		current_types = next_types
	end
	--spend remaining gold
	local min_cost = 100
	for i,v in ipairs(unit_pool) do
		min_cost = math.min(min_cost, wesnoth.unit_types[v[1]].cost)
	end
	while gold_left >= min_cost do
		local possible_groups = {}
		for i,v in ipairs(unit_pool) do
			local unit_type = wesnoth.unit_types[v[1]]
			if unit_type.cost <= gold_left then
				table.insert(possible_groups, { group = i, type = unit_type})
			end
		end
		local index = wesnoth.random(#possible_groups)
		table.insert(current_types, possible_groups[index])
		gold_left = gold_left - possible_groups[index].type.cost
	end
	local res = {}
	for i,v in ipairs(current_types) do
		table.insert(res, v.type.id)
	end
	return res
end

-- creates the 'timed_spawn' wml array.
-- @a num_spawns: the total number of times units get spawned
-- @a interval: the number of turns between 2 spawns
-- @a base_gold_amount, gold_increment: used to cauculate the amount of gold available for each timed spawn
-- @a units_amount, gold_per_unit_amount: used to cauculate the number of units spawned in each timed spawn
local function create_timed_spaws(interval, num_spawns, base_gold_amount, gold_increment, units_amount, gold_per_unit_amount)
	local configure_gold_factor = ((wesnoth.get_variable("enemey_gold_factor") or 0) + 100)/100
	local random_spawn_numbers = {}
	for i = 1, #random_spawns do
		table.insert(random_spawn_numbers, i)
	end
	helper.shuffle(random_spawn_numbers)
	local final_turn = math.ceil(((num_spawns - 1) * interval + 40 + wesnoth.random(2,4))/2)
	local end_spawns = 0
	for spawn_number = 1, num_spawns do
		local turn = 3 + (spawn_number - 1) * interval
		local gold = base_gold_amount + (turn - 3) * gold_increment
		if spawn_number > 1 then
			-- foruma taken from original Dark forecast, TODO: find easier formula.
			local unit_gold = (turn - 3) * gold_increment + math.min(wesnoth.random(base_gold_amount), wesnoth.random(base_gold_amount))
			local gold_per_unit = gold_per_unit_amount + turn / 1.5
			local units = unit_gold / gold_per_unit + units_amount + wesnoth.random(-1, 2)
			if wesnoth.random(5) == 5 then
				units = units - 1
			end
			-- end complicated formula
			turn = turn + wesnoth.random(-1, 1)
			-- reduce gold and units for spawns after the final spawn
			if turn >= final_turn then
				units = units / (end_spawns + 3)
				gold = gold / (end_spawns + 4)
				end_spawns = end_spawns + 1
				-- we only want two spawns after the final turn.
				if end_spawns > 2 then
					break
				end
			end
			wesnoth.set_variable(string.format("timed_spawn[%d]", spawn_number - 1), {
				units = math.ceil(units),
				turn = turn,
				gold = helper.round(gold * configure_gold_factor),
				pool_num = random_spawn_numbers[spawn_number],
			})
		else
			wesnoth.set_variable(string.format("timed_spawn[%d]", spawn_number - 1), {
				units = units_amount + 1,
				turn = turn,
				gold = gold,
				pool_num = random_spawn_numbers[spawn_number],
			})
		end
	end
	wesnoth.set_variable("final_turn", final_turn)
end

-- @a unittypes: a array of strings
-- @a x,y: the location where to spawn the units on the map.
local function place_units(unittypes, x, y)
	for i,v in ipairs(unittypes) do
		local u = wesnoth.create_unit { type = v, generate_name = true, side = 2 }
		u:add_modification("object", {
			T.effect {
				apply_to = "movement_costs",
				replace = true,
				T.movement_costs {
					flat = 1,
					sand = 2,
					forest = 2,
					impassable = 3,
					unwalkable = 3,
					deep_water = 3,
				}
			}
		})
		-- give the unit less moves on its first turn.
		u.status.slowed = true
		u:add_modification("object", {
			duration = "turn end",
			T.effect {
				apply_to = "movement",
				increase = "-50%",
			}
		})
		local dst_x, dst_y = wesnoth.find_vacant_tile(x, y, u)
		u:to_map(dst_x, dst_y)
	end
end

local function final_spawn()
	local spawns_left = wesnoth.get_variable("fixed_spawn.length")
	if spawns_left == 0 then
		return
	end
	local spawn_index = wesnoth.random(spawns_left) - 1
	local spawn = wesnoth.get_variable(string.format("fixed_spawn[%d]", spawn_index))
	wesnoth.set_variable(string.format("fixed_spawn[%d]", spawn_index))
	local types = {}
	for tag in helper.child_range(spawn, "type") do
		table.insert(types, tag.type)
	end
	place_units(types, spawn.x, spawn.y)
end

-- convert all 'veteran' units from side 2 to the more agressive side 1
-- this must happen before the new units are created from spawns.
on_event("new turn", function()
	for i, unit in ipairs(wesnoth.get_units { side = 2 }) do
		unit.side = 1
	end
end)

on_event("prestart", function()
	local leaders = wesnoth.get_units { side = "3,4", canrecruit= true}
	if #leaders < 2 then
		create_timed_spaws(5, 11, 50, 5, 4, 21)
	else
		create_timed_spaws(4, 11, 90, 4, 5, 23)
	end
end)

-- the regular spawns:
--   when they appear is defined in the 'timed_spawn' wml array. which is created at prestart
--   which unit types get spawned is defined in the 'main_spawn' wml array which is also spawned at prestart
on_event("new turn", function()
	local next_spawn = wesnoth.get_variable("timed_spawn[0]")
	if wesnoth.current.turn ~= next_spawn.turn then
		return
	end
	wesnoth.set_variable("timed_spawn[0]")
	local unit_types = get_spawn_types(next_spawn.units, next_spawn.gold, random_spawns[next_spawn.pool_num])
	local spawn_areas = {{"3-14", "15"}, {"1", "4-13"}, {"2-13", "1"}, {"1", "2-15"}}
	local spawn_area = spawn_areas[wesnoth.random(#spawn_areas)]
	local locations_in_area = wesnoth.get_locations { x = spawn_area[1], y = spawn_area[2], radius=1, include_borders=false }
	local chosen_location = locations_in_area[wesnoth.random(#locations_in_area)]
	place_units(unit_types, chosen_location[1], chosen_location[2])
end)

-- on turn 'final_turn' the first 'final spawn' appears
on_event("new turn", function()
	if wesnoth.current.turn ~= wesnoth.get_variable("final_turn") then
		return
	end
	wesnoth.wml_actions.music {
		name = "battle.ogg",
		ms_before = 200,
		immediate = true,
		append = true,
	}
	final_spawn()
	wesnoth.game_config.last_turn = wesnoth.current.turn + 12
	wesnoth.wml_actions.message {
		side="3,4",
		canrecruit=true,
		message= _ "The last and most powerful of these creatures are almost upon us. I feel that if we can finish them off in time, we shall be victorious.",
	}
end)

-- after the first final spawn, spawn a new final spawn every 1 or 2 turns.
on_event("new turn", function()
	if wesnoth.current.turn ~= wesnoth.get_variable("next_final_spawn") then
		return
	end
	final_spawn()
	wesnoth.set_variable("next_final_spawn", wesnoth.current.turn + wesnoth.random(1,2))
end)

-- The victory condition: win when there are no enemy unit after the first final spawn appeared.
on_event("die", function()
	if wesnoth.current.turn < wesnoth.get_variable("final_turn") then
		return
	end
	if wesnoth.wml_conditionals.have_unit { side = "1,2"} then
		return
	end
	wesnoth.wml_actions.music {
		name = "victory.ogg",
		play_once = true,
		immediate = true,
	}
	wesnoth.wml_actions.message {
		speaker = "narrator",
		message = _"The screams and pleas for mercy are finally silenced, as you remove your blood soaked blade from the last of the rebels. There will be no more resistance from the local scum. Your reign has finally earned stability.",
		image ="wesnoth-icon.png",
	}
	wesnoth.wml_actions.endlevel {
		result = "victory",
	}
end)

-- initilize the 'fixed_spawn' and 'main_spawn'
on_event("prestart", function()
	local fixed_spawn = function(x, y, ...)
		local res = { x = x, y = y }
		for i,v in ipairs {...} do
			table.insert(res, T.type { type = v })
		end
		return res
	end
	helper.set_variable_array("fixed_spawn", {
		fixed_spawn(1, 15, "Fire Dragon", "Gryphon Master", "Hurricane Drake"),
		fixed_spawn(5, 1, "Yeti", "Elvish Druid", "Elvish Druid"),
		fixed_spawn(1, 7, "Lich", "Walking Corpse", "Walking Corpse", "Walking Corpse", "Ghoul", "Soulless", "Walking Corpse", "Walking Corpse", "Walking Corpse"),
		fixed_spawn(11, 15, "Elvish Champion", "Dwarvish Stalwart", "Dwarvish Stalwart", "Orcish Slayer"),
	})
end)

-------------------------------------------------------------------------------
-------------------------- Weather events -------------------------------------
-------------------------------------------------------------------------------
-- The weather evens are complateleey unrelated to the spawn events.

local function get_weather_duration(max_duration)
	local res = wesnoth.random(2)
	while res < max_duration and wesnoth.random(2) == 2 do
		res = res + 1
	end
	return res
end
-- initilize the weather_event wml array which defines at which turns the weather changes.
on_event("prestart", function()
	local turn = wesnoth.random(4,6)
	local event_num = 0
	local weather_to_dispense = {
		{ turns_left = 10, id = "drought"},
		{ turns_left = 12, id = "heavy rain"},
		{ turns_left = 9, id = "snowfall"},
	}
	local clear_turns_left = 22
	local heavy_snowfall_turns_left = 6
	while turn < 55 and #weather_to_dispense > 0 do
		-- pick a random weather except 'clear' and 'heavy snow'
		local index = wesnoth.random(#weather_to_dispense)
		local num_turns = get_weather_duration(weather_to_dispense[index].turns_left)
		local weather_id = weather_to_dispense[index].id
		weather_to_dispense[index].turns_left = weather_to_dispense[index].turns_left - num_turns
		if weather_to_dispense[index].turns_left <= 0 then
			table.remove(weather_to_dispense, index)
		end
		wesnoth.set_variable(string.format("weather_event[%d]", event_num), {
			turn = turn,
			weather_id = weather_id,
		})
		event_num = event_num + 1
		turn = turn + num_turns
		-- Second snow happens half the time.
		if weather_id == "snowfall" and heavy_snowfall_turns_left >= 0 and wesnoth.random(2) == 2 then
			num_turns = get_weather_duration(heavy_snowfall_turns_left)
			wesnoth.set_variable(string.format("weather_event[%d]", event_num), {
				turn = turn,
				weather_id = "heavy snowfall",
			})
			event_num = event_num + 1
			turn = turn + num_turns
			heavy_snowfall_turns_left = heavy_snowfall_turns_left - num_turns
		end
		-- Go back to clear weather.
		num_turns = get_weather_duration(clear_turns_left)
		wesnoth.set_variable(string.format("weather_event[%d]", event_num), {
			turn = turn,
			weather_id = "clear",
		})
		event_num = event_num + 1
		turn = turn + num_turns
		clear_turns_left = clear_turns_left - num_turns
	end
end)

local function weather_alert(text, red, green, blue)
	wesnoth.wml_actions.print {
		text = text,
		duration = 120,
		size = 26,
		red = red,
		green = green,
		blue = blue,
	}
end

local function weather_map(name)
	wesnoth.wml_actions.terrain_mask {
		mask = wesnoth.read_file(name),
		x = 1,
		y = 1,
		border = true,
	}
	wesnoth.redraw {}
end

-- change weather at side 3 turns, TODO: consider the case that side 3 is empty.
on_event("side 3 turn", function()
	-- get next weather event
	local weather_event = wesnoth.get_variable("weather_event[0]")
	if wesnoth.current.turn ~= weather_event.turn then
		return
	end
	-- remove the to-be-consumed weather event from the todo list.
	wesnoth.set_variable("weather_event[0]")
	if weather_event.weather_id == "clear" then
		weather_map("multiplayer/maps/Dark_Forecast_basic.map")
		wesnoth.wml_actions.sound {
			name = "magic-holy-miss-2.ogg",
		}
		weather_alert(_"Clear Weather", 221, 253, 171)
	elseif weather_event.weather_id == "drought" then
		weather_map ("multiplayer/maps/Dark_Forecast_drought.map")
		wesnoth.wml_actions.sound {
			name = "gryphon-shriek-1.ogg",
		}
		weather_alert(_"Drought", 251, 231, 171)
	elseif weather_event.weather_id == "heavy rain" then
		weather_map("multiplayer/maps/Dark_Forecast_rain.map")
		wesnoth.wml_actions.sound {
			name = "magic-faeriefire-miss.ogg",
		}
		wesnoth.wml_actions.delay {
			time = 250,
		}
		wesnoth.wml_actions.sound {
			name = "ambient/ship.ogg",
		}
		weather_alert(_"Heavy Rains", 174, 220, 255)
	elseif weather_event.weather_id == "snowfall" then
		weather_map("multiplayer/maps/Dark_Forecast_firstsnow.map")
		wesnoth.wml_actions.sound {
			name = "wail.wav",
		}
		weather_alert(_"Snowfall", 229, 243, 241)
	elseif weather_event.weather_id == "heavy snowfall" then
		weather_map("multiplayer/maps/Dark_Forecast_secondsnow.map")
		wesnoth.wml_actions.sound {
			name = "bat-flapping.wav",
		}
		weather_alert(_"Heavy Snowfall", 224, 255, 251)
	else
		error("unknown weather '" .. tostring(weather_event.weather_id) .. "'")
	end
end)


