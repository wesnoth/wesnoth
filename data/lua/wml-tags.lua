local helper = wesnoth.require "helper"
local location_set = wesnoth.require "location_set"
local utils = wesnoth.require "wml-utils"
local wml_actions = wesnoth.wml_actions
local T = helper.set_wml_tag_metatable {}

function wesnoth.game_events.on_load(cfg)
	if #cfg == 0 then return end
	local t = {}
	for i = 1,#cfg do t[i] = string.format("[%s]", cfg[i][1]) end
	helper.wml_error(string.format("%s not supported at scenario toplevel", table.concat(t, ', ')))
end

function wesnoth.game_events.on_save()
	return {}
end

wesnoth.require "wml-flow"
wesnoth.require "wml"

--[[

Note: When adding new WML tags, unless they're very simple, it's preferred to
add a new file in the "data/lua/wml" directory rather than implementing it in this file.
The file will then automatically be loaded by the above require statement.

Also note: The above on_load event needs to be registered before any other on_load events.
That means before loading the WML tags via wesnoth.require "wml".

]]

function wml_actions.sync_variable(cfg)
	local names = cfg.name or helper.wml_error "[sync_variable] missing required name= attribute."
	local result = wesnoth.synchronize_choice(
		function()
			local res = {}
			for name_raw in utils.split(names) do
				local name = utils.trim(name_raw)
				local variable_type = string.sub(name, string.len(name)) == "]" and "indexed" or ( wesnoth.get_variable(name .. ".length") > 0 and "array" or "attribute")
				local variable_info = { name = name, type = variable_type }
				table.insert(res, { "variable", variable_info })
				if variable_type == "indexed" then
					table.insert(variable_info, { "value", wesnoth.get_variable(name) } )
				elseif variable_type == "array" then
					for i = 1, wesnoth.get_variable(name .. ".length") do
						table.insert(variable_info, { "value",  wesnoth.get_variable(string.format("%s[%d]", name, i - 1)) } )
					end
				else
					variable_info.value = wesnoth.get_variable(name)
				end
			end
			return res
		end
	)
	for variable in helper.child_range(result, "variable") do
		local name = variable.name

		if variable.type == "indexed" then
			wesnoth.set_variable(name, variable[1][2])
		elseif variable.type == "array" then
			for index, cfg_pair in ipairs(variable) do
				wesnoth.set_variable( string.format("%s[%d]", name, index - 1), cfg_pair[2])
			end
		else
			wesnoth.set_variable(name, variable.value)
		end
	end
end

function wml_actions.chat(cfg)
	local side_list = wesnoth.get_sides(cfg)
	local speaker = tostring(cfg.speaker or "WML")
	local message = tostring(cfg.message or
		helper.wml_error "[chat] missing required message= attribute."
	)

	for index, side in ipairs(side_list) do
		if side.controller == "human" then
			wesnoth.message(speaker, message)
			break
		end
	end
end

function wml_actions.gold(cfg)
	local amount = tonumber(cfg.amount) or
		helper.wml_error "[gold] missing required amount= attribute."
	local sides = wesnoth.get_sides(cfg)
	for index, team in ipairs(sides) do
		team.gold = team.gold + amount
	end
end

--note: This tag can't easily (without deprecation) be extended to store an array,
--since the gold is stored in a scalar variable, not a container (there's no key).
function wml_actions.store_gold(cfg)
	local team = wesnoth.get_sides(cfg)[1]
	if team then wesnoth.set_variable(cfg.variable or "gold", team.gold) end
end

function wml_actions.clear_variable(cfg)
	local names = cfg.name or
		helper.wml_error "[clear_variable] missing required name= attribute."
	for w in utils.split(names) do
		wesnoth.set_variable(utils.trim(w))
	end
end

function wml_actions.store_unit_type_ids(cfg)
	local types = {}
	for k,v in pairs(wesnoth.unit_types) do
		table.insert(types, k)
	end
	table.sort(types)
	types = table.concat(types, ',')
	wesnoth.set_variable(cfg.variable or "unit_type_ids", types)
end

function wml_actions.store_unit_type(cfg)
	local types = cfg.type or
		helper.wml_error "[store_unit_type] missing required type= attribute."
	local writer = utils.vwriter.init(cfg, "unit_type")
	for w in utils.split(types) do
		local unit_type = wesnoth.unit_types[w] or
			helper.wml_error(string.format("Attempt to store nonexistent unit type '%s'.", w))
		utils.vwriter.write(writer, unit_type.__cfg)
	end
end

function wml_actions.fire_event(cfg)
	local u1 = helper.get_child(cfg, "primary_unit")
	u1 = u1 and wesnoth.get_units(u1)[1]
	local x1, y1 = 0, 0
	if u1 then x1, y1 = u1.x, u1.y end

	local u2 = helper.get_child(cfg, "secondary_unit")
	u2 = u2 and wesnoth.get_units(u2)[1]
	local x2, y2 = 0, 0
	if u2 then x2, y2 = u2.x, u2.y end

	local w1 = helper.get_child(cfg, "primary_attack")
	local w2 = helper.get_child(cfg, "secondary_attack")
	if w2 then w1 = w1 or {} end

	if cfg.id and cfg.id ~= "" then wesnoth.fire_event_by_id(cfg.id, x1, y1, x2, y2, w1, w2)
	elseif cfg.name and cfg.name ~= "" then wesnoth.fire_event(cfg.name, x1, y1, x2, y2, w1, w2)
	end
end

function wml_actions.allow_recruit(cfg)
	local unit_types = cfg.type or helper.wml_error("[allow_recruit] missing required type= attribute")
	for index, team in ipairs(wesnoth.get_sides(cfg)) do
		local v = team.recruit
		for type in utils.split(unit_types) do
			table.insert(v, type)
			wesnoth.add_known_unit(type)
		end
		team.recruit = v
		end
end

function wml_actions.allow_extra_recruit(cfg)
	local recruits = cfg.extra_recruit or helper.wml_error("[allow_extra_recruit] missing required extra_recruit= attribute")
	for index, unit in ipairs(wesnoth.get_units(cfg)) do
		local v = unit.extra_recruit
		for recruit in utils.split(recruits) do
			table.insert(v, recruit)
			wesnoth.add_known_unit(recruit)
		end
		unit.extra_recruit = v
	end
end

function wml_actions.disallow_recruit(cfg)
	local unit_types = cfg.type
	for index, team in ipairs(wesnoth.get_sides(cfg)) do
		if unit_types then
			local v = team.recruit
			for w in utils.split(unit_types) do
				for i, r in ipairs(v) do
					if r == w then
						table.remove(v, i)
						break
					end
				end
			end
			team.recruit = v
		else
			team.recruit = nil
		end
	end
end

function wml_actions.disallow_extra_recruit(cfg)
	local recruits = cfg.extra_recruit or helper.wml_error("[disallow_extra_recruit] missing required extra_recruit= attribute")
	for index, unit in ipairs(wesnoth.get_units(cfg)) do
		local v = unit.extra_recruit
		for w in utils.split(recruits) do
			for i, r in ipairs(v) do
				if r == w then
					table.remove(v, i)
					break
				end
			end
		end
		unit.extra_recruit = v
	end
end

function wml_actions.set_recruit(cfg)
	local recruit = cfg.recruit or helper.wml_error("[set_recruit] missing required recruit= attribute")
	for index, team in ipairs(wesnoth.get_sides(cfg)) do
		local v = {}
		for w in utils.split(recruit) do
			table.insert(v, w)
		end
		team.recruit = v
	end
end

function wml_actions.set_extra_recruit(cfg)
	local recruits = cfg.extra_recruit or helper.wml_error("[set_extra_recruit] missing required extra_recruit= attribute")
	local v = {}

	for w in utils.split(recruits) do
		table.insert(v, w)
	end

	for index, unit in ipairs(wesnoth.get_units(cfg)) do
		unit.extra_recruit = v
	end
end

function wml_actions.store_map_dimensions(cfg)
	local var = cfg.variable or "map_size"
	local w, h, b = wesnoth.get_map_size()
	wesnoth.set_variable(var .. ".width", w)
	wesnoth.set_variable(var .. ".height", h)
	wesnoth.set_variable(var .. ".border_size", b)
end

function wml_actions.unit_worth(cfg)
	local u = wesnoth.get_units(cfg)[1] or
		helper.wml_error "[unit_worth]'s filter didn't match any unit"
	local ut = wesnoth.unit_types[u.type]
	local hp = u.hitpoints / u.max_hitpoints
	local xp = u.experience / u.max_experience
	local best_adv = ut.cost
	for w in utils.split(ut.__cfg.advances_to) do
		local uta = wesnoth.unit_types[w]
		if uta and uta.cost > best_adv then best_adv = uta.cost end
	end
	wesnoth.set_variable("cost", ut.cost)
	wesnoth.set_variable("next_cost", best_adv)
	wesnoth.set_variable("health", math.floor(hp * 100))
	wesnoth.set_variable("experience", math.floor(xp * 100))
	wesnoth.set_variable("recall_cost", ut.recall_cost)
	wesnoth.set_variable("unit_worth", math.floor(math.max(ut.cost * hp, best_adv * xp)))
end

function wml_actions.lua(cfg)
	local cfg = helper.shallow_literal(cfg)
	local bytecode, message = load(cfg.code or "")
	if not bytecode then error("~lua:" .. message, 0) end
	bytecode(helper.get_child(cfg, "args"))
end

function wml_actions.music(cfg)
	if cfg.play_once then
		wesnoth.music_list.play(cfg.name)
	else
		if not cfg.append then
			if cfg.immediate then
				wesnoth.music_list.current.once = true
			end
			wesnoth.music_list.clear()
		end
		wesnoth.music_list.add(cfg.name, not not cfg.immediate, cfg.ms_before or 0, cfg.ms_after or 0)
		local n = #wesnoth.music_list
		if cfg.shuffle == false then
			wesnoth.music_list[n].shuffle = false
		end
		if cfg.title ~= nil then
			wesnoth.music_list[n].title = cfg.title
		end
	end
end

function wml_actions.volume(cfg)
	if cfg.music then
		local rel = tonumber(cfg.music) or 100.0
		wesnoth.music_list.volume = rel
	end
	if cfg.sound then
		local rel = tonumber(cfg.sound) or 100.0
		wesnoth.sound_volume(rel)
	end
end

function wml_actions.scroll_to(cfg)
	local loc = wesnoth.get_locations( cfg )[1]
	if not loc then return end
	if not utils.optional_side_filter(cfg) then return end
	wesnoth.scroll_to_tile(loc[1], loc[2], cfg.check_fogged, cfg.immediate)
	if cfg.highlight then
		wesnoth.highlight_hex(loc[1], loc[2])
		wml_actions.redraw{}
	end
end

function wml_actions.scroll_to_unit(cfg)
	local u = wesnoth.get_units(cfg)[1]
	if not u then return end
	if not utils.optional_side_filter(cfg, "for_side", "for_side") then return end
	wesnoth.scroll_to_tile(u.x, u.y, cfg.check_fogged, cfg.immediate)
	if cfg.highlight then
		wesnoth.highlight_hex(u.x, u.y)
		wml_actions.redraw{}
	end
end

function wml_actions.lock_view(cfg)
	wesnoth.lock_view(true)
end

function wml_actions.unlock_view(cfg)
	wesnoth.lock_view(false)
end

function wml_actions.select_unit(cfg)
	local u = wesnoth.get_units(cfg)[1]
	if not u then return end
	wesnoth.select_unit(u.x, u.y, cfg.highlight, cfg.fire_event)
end

function wml_actions.unit_overlay(cfg)
	local img = cfg.image or helper.wml_error( "[unit_overlay] missing required image= attribute" )
	for i,u in ipairs(wesnoth.get_units(cfg)) do
		local ucfg = u.__cfg
		for w in utils.split(ucfg.overlays) do
			if w == img then ucfg = nil end
		end
		if ucfg then
			ucfg.overlays = ucfg.overlays .. ',' .. img
			wesnoth.put_unit(ucfg)
		end
	end
end

function wml_actions.remove_unit_overlay(cfg)
	local img = cfg.image or helper.wml_error( "[remove_unit_overlay] missing required image= attribute" )

	-- Loop through all matching units.
	for i,u in ipairs(wesnoth.get_units(cfg)) do
		local ucfg = u.__cfg
		local t = utils.parenthetical_split(ucfg.overlays)
		-- Remove the specified image from the overlays.
		for i = #t,1,-1 do
			if t[i] == img then table.remove(t, i) end
		end
		-- Reassemble the list of remaining overlays.
		ucfg.overlays = table.concat(t, ',')
		wesnoth.put_unit(ucfg)
	end
end

function wml_actions.store_turns(cfg)
	local var = cfg.variable or "turns"
	wesnoth.set_variable(var, wesnoth.game_config.last_turn)
end

function wml_actions.store_unit(cfg)
	local filter = helper.get_child(cfg, "filter") or
		helper.wml_error "[store_unit] missing required [filter] tag"
	local kill_units = cfg.kill

	--cache the needed units here, since the filter might reference the to-be-cleared variable(s)
	local units = wesnoth.get_units(filter)
	local recall_units = wesnoth.get_recall_units(filter)

	local writer = utils.vwriter.init(cfg, "unit")

	for i,u in ipairs(units) do
		utils.vwriter.write(writer, u.__cfg)
		if kill_units then wesnoth.erase_unit(u) end
	end

	if (not filter.x or filter.x == "recall") and (not filter.y or filter.y == "recall") then
		for i,u in ipairs(recall_units) do
			local ucfg = u.__cfg
			ucfg.x = "recall"
			ucfg.y = "recall"
			utils.vwriter.write(writer, ucfg)
			if kill_units then wesnoth.erase_unit(u) end
		end
	end
end

function wml_actions.sound(cfg)
	local name = cfg.name or helper.wml_error("[sound] missing required name= attribute")
	wesnoth.play_sound(name, cfg["repeat"])
end

function wml_actions.store_locations(cfg)
	-- the variable can be mentioned in a [find_in] subtag, so it
	-- cannot be cleared before the locations are recovered
	local locs = wesnoth.get_locations(cfg)
	local writer = utils.vwriter.init(cfg, "location")
	for i, loc in ipairs(locs) do
		local x, y = loc[1], loc[2]
		local t = wesnoth.get_terrain(x, y)
		local res = { x = x, y = y, terrain = t }
		if wesnoth.get_terrain_info(t).village then
			res.owner_side = wesnoth.get_village_owner(x, y) or 0
		end
		utils.vwriter.write(writer, res)
	end
end

function wml_actions.store_reachable_locations(cfg)
	local unit_filter = helper.get_child(cfg, "filter") or
		helper.wml_error "[store_reachable_locations] missing required [filter] tag"
	local location_filter = helper.get_child(cfg, "filter_location")
	local range = cfg.range or "movement"
	local moves = cfg.moves or "current"
	local variable = cfg.variable or helper.wml_error "[store_reachable_locations] missing required variable= key"
	local reach_param = { viewing_side = cfg.viewing_side or 0 }
	if range == "vision" then
		moves = "max"
		reach_param.ignore_units = true
	end

	local reach = location_set.create()

	for i,unit in ipairs(wesnoth.get_units(unit_filter)) do
		local unit_reach
		if moves == "max" then
			local saved_moves = unit.moves
			unit.moves = unit.max_moves
			unit_reach = location_set.of_pairs(wesnoth.find_reach(unit, reach_param))
			unit.moves = saved_moves
		else
			unit_reach = location_set.of_pairs(wesnoth.find_reach(unit, reach_param))
		end

		if range == "vision" or range == "attack" then
			unit_reach:iter(function(x, y)
				reach:insert(x, y)
				for u,v in helper.adjacent_tiles(x, y) do
					reach:insert(u, v)
				end
			end)
		else
			reach:union(unit_reach)
		end
	end

	if location_filter then
		reach = reach:filter(function(x, y)
			return wesnoth.match_location(x, y, location_filter)
		end)
	end
	reach:to_wml_var(variable)
end

function wml_actions.hide_unit(cfg)
	for i,u in ipairs(wesnoth.get_units(cfg)) do
		u.hidden = true
	end
	wml_actions.redraw {}
end

function wml_actions.unhide_unit(cfg)
	for i,u in ipairs(wesnoth.get_units(cfg)) do
		u.hidden = false
	end
	wml_actions.redraw {}
end

function wml_actions.capture_village(cfg)
	local side = cfg.side
	local filter_side = helper.get_child(cfg, "filter_side")
	local fire_event = cfg.fire_event
	if side then side = tonumber(side) or helper.wml_error("invalid side in [capture_village]") end
	if filter_side then
		if side then helper.wml_error("duplicate side information in [capture_village]") end
		side = wesnoth.get_sides(filter_side)[1]
		if side then side = side.side end
	end
	local locs = wesnoth.get_locations(cfg)

	for i, loc in ipairs(locs) do
		wesnoth.set_village_owner(loc[1], loc[2], side, fire_event)
	end
end

function wml_actions.terrain(cfg)
	local terrain = cfg.terrain or helper.wml_error("[terrain] missing required terrain= attribute")
	cfg = helper.shallow_parsed(cfg)
	cfg.terrain = nil
	for i, loc in ipairs(wesnoth.get_locations(cfg)) do
		wesnoth.set_terrain(loc[1], loc[2], terrain, cfg.layer, cfg.replace_if_failed)
	end
end

function wml_actions.delay(cfg)
	local delay = tonumber(cfg.time) or
		helper.wml_error "[delay] missing required time= attribute."
	local accelerate = cfg.accelerate or false
	wesnoth.delay(delay, accelerate)
end

function wml_actions.floating_text(cfg)
	local locs = wesnoth.get_locations(cfg)
	local text = cfg.text or helper.wml_error("[floating_text] missing required text= attribute")

	for i, loc in ipairs(locs) do
		wesnoth.float_label(loc[1], loc[2], text, cfg.color)
	end
end

function wml_actions.petrify(cfg)
	for index, unit in ipairs(wesnoth.get_units(cfg)) do
		unit.status.petrified = true
		-- Extract unit and put it back to update animation (not needed for recall units)
		wesnoth.extract_unit(unit)
		wesnoth.put_unit(unit)
	end

	for index, unit in ipairs(wesnoth.get_recall_units(cfg)) do
		unit.status.petrified = true
	end
end

function wml_actions.unpetrify(cfg)
	for index, unit in ipairs(wesnoth.get_units(cfg)) do
		unit.status.petrified = false
		-- Extract unit and put it back to update animation (not needed for recall units)
		wesnoth.extract_unit(unit)
		wesnoth.put_unit(unit)
	end

	for index, unit in ipairs(wesnoth.get_recall_units(cfg)) do
		unit.status.petrified = false
	end
end

function wml_actions.transform_unit(cfg)
	local transform_to = cfg.transform_to

	for index, unit in ipairs(wesnoth.get_units(cfg)) do

		if transform_to then
			wesnoth.transform_unit( unit, transform_to )
		else
			local hitpoints = unit.hitpoints
			local experience = unit.experience
			local recall_cost = unit.recall_cost
			local status = helper.get_child( unit.__cfg, "status" )

			unit.experience = unit.max_experience
			wesnoth.advance_unit(unit, false, false)

			unit.hitpoints = hitpoints
			unit.experience = experience
			recall_cost = unit.recall_cost

			for key, value in pairs(status) do unit.status[key] = value end
			if unit.status.unpoisonable then unit.status.poisoned = nil end
		end
	end

	wml_actions.redraw {}
end

function wml_actions.store_side(cfg)
	local writer = utils.vwriter.init(cfg, "side")
	for t, side_number in helper.get_sides(cfg) do
		local container = t.__cfg
		-- set values not properly handled by the __cfg
		container.income = t.total_income
		container.net_income = t.net_income
		container.base_income = t.base_income
		container.expenses = t.expenses
		container.total_upkeep = t.total_upkeep
		container.num_units = t.num_units
		container.num_villages = t.num_villages
		container.side = side_number
		utils.vwriter.write(writer, container)
	end
end

function wml_actions.add_ai_behavior(cfg)
	local unit = wesnoth.get_units(helper.get_child(cfg, "filter"))[1] or
		helper.wml_error("[add_ai_behavior]: no unit specified")

	local side = cfg.side or
		helper.wml_error("[add_ai_behavior]: no side attribute given")

	local sticky = cfg.sticky or false
	local loop_id = cfg.loop_id or "main_loop"
	local eval = cfg.evaluation
	local exec = cfg.execution
	local id = cfg.bca_id or ("bca-" .. unit.__cfg.underlying_id)

	local ux = unit.x -- @note: did I get it right that coordinates in C++ differ by 1 from thos in-game(and in Lua)?
	local uy = unit.y

	if not (eval and exec) then
		helper.wml_error("[add_ai_behavior]: invalid execution/evaluation handler(s)")
	end

	local path = "stage[" .. loop_id .. "].candidate_action[" .. id .. "]" -- bca: behavior candidate action

	wesnoth.wml_actions.modify_ai {
		action = "add",
		engine = "lua",
		path = path,
		side = side,

		T.candidate_action {
			id = id,
			name = id,
			engine = "lua",
			sticky = sticky,
			unit_x = ux,
			unit_y = uy,
			evaluation = eval,
			execution = exec
		},
	}
end

function wml_actions.store_starting_location(cfg)
	local writer = utils.vwriter.init(cfg, "location")
	for possibly_wrong_index, side in ipairs(wesnoth.get_sides(cfg)) do
		local loc = wesnoth.get_starting_location(side.side)
		if loc then
			local terrain = wesnoth.get_terrain(loc[1], loc[2])
			local result = { x = loc[1], y = loc[2], terrain = terrain }
			if wesnoth.get_terrain_info(terrain).village then
				result.owner_side = wesnoth.get_village_owner(loc[1], loc[2]) or 0
			end
			utils.vwriter.write(writer, result)
		end
	end
end

function wml_actions.store_villages( cfg )
	local villages = wesnoth.get_villages( cfg )
	local writer = utils.vwriter.init(cfg, "location")
	for index, village in ipairs( villages ) do
		utils.vwriter.write(writer, {
			x = village[1],
			y = village[2],
			terrain = wesnoth.get_terrain( village[1], village[2] ),
			owner_side = wesnoth.get_village_owner( village[1], village[2] ) or 0
		})
	end
end

function wml_actions.put_to_recall_list(cfg)
	local units = wesnoth.get_units(cfg)

	for i, unit in ipairs(units) do
		if cfg.heal then
			unit.hitpoints = unit.max_hitpoints
			unit.moves = unit.max_moves
			unit.attacks_left = unit.max_attacks
			unit.status.poisoned = false
			unit.status.slowed = false
		end
		wesnoth.put_recall_unit(unit, unit.side)
	end
end

function wml_actions.allow_undo(cfg)
	wesnoth.allow_undo()
end

function wml_actions.allow_end_turn(cfg)
	wesnoth.allow_end_turn(true)
end

function wml_actions.disallow_end_turn(cfg)
	wesnoth.allow_end_turn(false)
end

function wml_actions.clear_menu_item(cfg)
	wesnoth.clear_menu_item(cfg.id)
end

function wml_actions.set_menu_item(cfg)
	wesnoth.set_menu_item(cfg.id, cfg)
end

function wml_actions.place_shroud(cfg)
	local sides = utils.get_sides(cfg)
	local tiles = wesnoth.get_locations(cfg)
	for i,side in ipairs(sides) do
		wesnoth.place_shroud(side.side, tiles)
	end
end

function wml_actions.remove_shroud(cfg)
	local sides = utils.get_sides(cfg)
	local tiles = wesnoth.get_locations(cfg)
	for i,side in ipairs(sides) do
		wesnoth.remove_shroud(side.side, tiles)
	end
end

function wml_actions.time_area(cfg)
	if cfg.remove then
		wml_actions.remove_time_area(cfg)
	else
		wesnoth.add_time_area(cfg)
	end
end

function wml_actions.remove_time_area(cfg)
	local id = cfg.id or helper.wml_error("[remove_time_area] missing required id= key")

	for w in utils.split(id) do
		wesnoth.remove_time_area(w)
	end
end

function wml_actions.replace_schedule(cfg)
	wesnoth.replace_schedule(cfg)
end

function wml_actions.scroll(cfg)
	local sides = utils.get_sides(cfg)
	local have_human = false
	for i, side in ipairs(sides) do
		if side.controller == 'human' and side.is_local then
			have_human = true
		end
	end
	if have_human or #sides == 0 then
		wesnoth.scroll(cfg.x, cfg.y)
	end
end

function wml_actions.color_adjust(cfg)
	wesnoth.color_adjust(cfg)
end

function wml_actions.end_turn(cfg)
	wesnoth.end_turn()
end

function wml_actions.event(cfg)
	if cfg.remove then
		wml_actions.remove_event(cfg)
	else
		wesnoth.add_event_handler(cfg)
	end
end

function wml_actions.remove_event(cfg)
	local id = cfg.id or helper.wml_error("[remove_event] missing required id= key")

	for w in utils.split(id) do
		wesnoth.remove_event_handler(w)
	end
end

function wml_actions.inspect(cfg)
	wesnoth.gamestate_inspector(cfg)
end

function wml_actions.label( cfg )
	local new_cfg = helper.parsed( cfg )
	for index, location in ipairs( wesnoth.get_locations( cfg ) ) do
		new_cfg.x, new_cfg.y = location[1], location[2]
		wesnoth.label( new_cfg )
	end
end

function wml_actions.open_help(cfg)
	wesnoth.open_help(cfg.topic)
end

function wml_actions.redraw(cfg)
	local clear_shroud = cfg.clear_shroud

	-- Backwards compat, the behavior of the tag was to clear shroud in case that side= is given.
	if cfg.clear_shroud == nil and cfg.side ~= nil then
		clear_shroud = true
	end

	wesnoth.redraw(cfg, clear_shroud)
end

function wml_actions.print(cfg)
	wesnoth.print(cfg)
end

function wml_actions.unsynced(cfg)
	wesnoth.unsynced(function ()
		wml_actions.command(cfg)
	end)
end

local function on_board(x, y)
	if type(x) ~= "number" or type(y) ~= "number" then
		return false
	end
	local w, h = wesnoth.get_map_size()
	return x >= 1 and y >= 1 and x <= w and y <= h
end

wml_actions.unstore_unit = function(cfg)
	local variable = cfg.variable or helper.wml_error("[unstore_unit] missing required 'variable' attribute")
	local unit_cfg = wesnoth.get_variable(variable)
	local unit = wesnoth.create_unit(unit_cfg)
	local advance = cfg.advance ~= false
	local animate = cfg.animate ~= false
	local check_passability = cfg.check_passability ~= false or nil
	local x = cfg.x or unit.x or -1
	local y = cfg.y or unit.y or -1
	wesnoth.add_known_unit(unit.type)
	if on_board(x, y) then
		if cfg.find_vacant then
			x,y = wesnoth.find_vacant_tile(x, y, check_passability and unit)
		end
		unit:to_map(x, y, cfg.fire_event)
		local text = nil
		if unit_cfg.gender == "female" then
			text = cfg.female_text or cfg.text
		else
			text = cfg.male_text or cfg.text
		end
		local color = cfg.color
		if color == nil and cfg.red and cfg.blue and cfg.green then
			color = cfg.red .. "," .. cfg.green .. "," .. cfg.blue
		end
		if text ~= nil and not wesnoth.is_skipping_messages() then
			wesnoth.float_label(x, y, text, color)
		end
		if advance then
			wesnoth.advance_unit(unit, animate, cfg.fire_event)
		end
	else
		unit:to_recall()
	end
end

wml_actions.teleport = function(cfg)
	local context = wesnoth.current.event_context
	local filter = helper.get_child(cfg, "filter") or { x = context.x1, y = context.y1 }
	local unit = wesnoth.get_units(filter)[1]
	if not unit then
		-- No error if no unit matches.
		return
	end
	wesnoth.teleport(unit, cfg.x, cfg.y, cfg.check_passability == false, cfg.clear_shroud ~= false, cfg.animate)
end

function wml_actions.remove_sound_source(cfg)
	wesnoth.remove_sound_source(cfg.id)
end

function wml_actions.sound_source(cfg)
	wesnoth.add_sound_source(cfg)
end

function wml_actions.deprecated_message(cfg)
	if not wesnoth.game_config.debug then return end
	wesnoth.log('wml', cfg.message)
end

function wml_actions.wml_message(cfg)
	wesnoth.log(cfg.logger, cfg.message, cfg.to_chat)
end

local function parse_fog_cfg(cfg)
	-- Side filter
	local ssf = helper.get_child(cfg, "filter_side")
	local sides = wesnoth.get_sides(ssf or {})
	-- Location filter
	local locs = wesnoth.get_locations(cfg)
	return locs, sides
end

function wml_actions.lift_fog(cfg)
	local locs, sides = parse_fog_cfg(cfg)
	for i = 1, #sides do
		wesnoth.remove_fog(sides[i].side, locs, not cfg.multiturn)
	end
end

function wml_actions.reset_fog(cfg)
	local locs, sides = parse_fog_cfg(cfg)
	for i = 1, #sides do
		wesnoth.add_fog(sides[i].side, locs, cfg.reset_view)
	end
end

function wesnoth.wml_actions.change_theme(cfg)
	wesnoth.game_config.theme = cfg.theme
end

function wesnoth.wml_actions.zoom(cfg)
	wesnoth.zoom(cfg.factor, cfg.relative)
end

function wesnoth.wml_actions.story(cfg)
	wesnoth.show_story(cfg, cfg.title)
end

function wesnoth.wml_conditionals.proceed_to_next_scenario(cfg)
	local endlevel_data = wesnoth.get_end_level_data()
	if not endlevel_data then
		return false
	else
		return endlevel_data.proceed_to_next_level
	end
end
