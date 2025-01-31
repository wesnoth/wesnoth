local utils = wesnoth.require "wml-utils"
local wml_actions = wesnoth.wml_actions
local T = wml.tag

wesnoth.require "wml-conditionals"
wesnoth.require "wml-flow"
wesnoth.require "wml"

--[[

Note: When adding new WML tags, unless they're very simple, it's preferred to
add a new file in the "data/lua/wml" directory rather than implementing it in this file.
The file will then automatically be loaded by the above require statement.

]]

function wml_actions.sync_variable(cfg)
	local names = cfg.name or wml.error "[sync_variable] missing required name= attribute."
	local result = wesnoth.sync.evaluate_single(
		function()
			local res = {}
			for _,name_raw in ipairs(names:split()) do
				local name = name_raw:trim()
				local variable_type = string.sub(name, string.len(name)) == "]" and "indexed" or ( wml.variables[name .. ".length"] > 0 and "array" or "attribute")
				local variable_info = { name = name, type = variable_type }
				table.insert(res, { "variable", variable_info })
				if variable_type == "indexed" then
					table.insert(variable_info, { "value", wml.variables[name] } )
				elseif variable_type == "array" then
					for i = 1, wml.variables[name .. ".length"] do
						table.insert(variable_info, { "value",  wml.variables[string.format("%s[%d]", name, i - 1)] } )
					end
				else
					variable_info.value = wml.variables[name]
				end
			end
			return res
		end
	)
	for variable in wml.child_range(result, "variable") do
		local name = variable.name

		if variable.type == "indexed" then
			wml.variables[name] = variable[1][2]
		elseif variable.type == "array" then
			for index, cfg_pair in ipairs(variable) do
				wml.variables[string.format("%s[%d]", name, index - 1)] = cfg_pair[2]
			end
		else
			wml.variables[name] = variable.value
		end
	end
end

function wml_actions.chat(cfg)
	local side_list = wesnoth.sides.find(cfg)
	local speaker = tostring(cfg.speaker or "WML")
	local message = tostring(cfg.message or
		wml.error "[chat] missing required message= attribute."
	)

	for index, side in ipairs(side_list) do
		if side.controller == "human" and side.is_local then
			wesnoth.interface.add_chat_message(speaker, message)
			break
		end
	end

	local observable = cfg.observable ~= false

	if observable then
		local all_sides = wesnoth.sides.find{}
		local has_human_side = false
		for index, side in ipairs(all_sides) do
			if side.controller == "human" and side.is_local then
				has_human_side = true
				break
			end
		end

		if not has_human_side then
			wesnoth.interface.add_chat_message(speaker, message)
		end
	end
end

function wml_actions.gold(cfg)
	local amount = tonumber(cfg.amount) or
		wml.error "[gold] missing required amount= attribute."
	local sides = wesnoth.sides.find(cfg)
	for index, team in ipairs(sides) do
		team.gold = team.gold + amount
	end
end

--note: This tag can't easily (without deprecation) be extended to store an array,
--since the gold is stored in a scalar variable, not a container (there's no key).
function wml_actions.store_gold(cfg)
	local team = wesnoth.sides.find(cfg)[1]
	if team then wml.variables[cfg.variable or "gold"] = team.gold end
end

---@diagnostic disable-next-line: redundant-parameter
function wml_actions.clear_variable(cfg, variables)
	local names = cfg.name or
		wml.error "[clear_variable] missing required name= attribute."
	if variables == nil then variables = wml.variables end
	for _,w in ipairs(tostring(names):split()) do
		variables[w:trim()] = nil
	end
end

function wml_actions.store_unit_type_ids(cfg)
	local types = {}
	for k in pairs(wesnoth.unit_types) do
		table.insert(types, k)
	end
	table.sort(types)
	wml.variables[cfg.variable or "unit_type_ids"] = table.concat(types, ',')
end

function wml_actions.store_unit_type(cfg)
	local types = cfg.type or
		wml.error "[store_unit_type] missing required type= attribute."
	local writer = utils.vwriter.init(cfg, "unit_type")
	for _,w in ipairs(types:split()) do
		local unit_type = wesnoth.unit_types[w] or
			wml.error(string.format("Attempt to store nonexistent unit type '%s'.", w))
		utils.vwriter.write(writer, unit_type.__cfg)
	end
end

function wml_actions.fire_event(cfg)
	local u1 = wml.get_child(cfg, "primary_unit")
	u1 = u1 and wesnoth.units.find_on_map(u1)[1]
	local x1, y1 = 0, 0
	if u1 then x1, y1 = u1.x, u1.y end

	local u2 = wml.get_child(cfg, "secondary_unit")
	u2 = u2 and wesnoth.units.find_on_map(u2)[1]
	local x2, y2 = 0, 0
	if u2 then x2, y2 = u2.x, u2.y end

	local w1 = wml.get_child(cfg, "primary_attack")
	local w2 = wml.get_child(cfg, "secondary_attack")
	local data = wml.get_child(cfg, "data") or {}
	if w1 then table.insert(data, wml.tag.first(w1)) end
	if w2 then table.insert(data, wml.tag.second(w2)) end

	local scoped_data <close> = utils.scoped_var("data")
	scoped_data:set({wml.parsed(data)})

	if cfg.id and cfg.id ~= "" then wesnoth.game_events.fire_by_id(cfg.id, x1, y1, x2, y2, data)
	elseif cfg.name and cfg.name ~= "" then wesnoth.game_events.fire(cfg.name, x1, y1, x2, y2, data)
	end
end

function wml_actions.allow_recruit(cfg)
	local unit_types = cfg.type or wml.error("[allow_recruit] missing required type= attribute")
	for index, team in ipairs(wesnoth.sides.find(cfg)) do
		local v = team.recruit
		for _,type in ipairs(unit_types:split()) do
			table.insert(v, type)
			wesnoth.add_known_unit(type)
		end
		team.recruit = v
		end
end

function wml_actions.allow_extra_recruit(cfg)
	local recruits = cfg.extra_recruit or wml.error("[allow_extra_recruit] missing required extra_recruit= attribute")
	for index, unit in ipairs(wesnoth.units.find_on_map(cfg)) do
		local v = unit.extra_recruit
		for _,recruit in ipairs(recruits:split()) do
			table.insert(v, recruit)
			wesnoth.add_known_unit(recruit)
		end
		unit.extra_recruit = v
	end
end

function wml_actions.disallow_recruit(cfg)
	local unit_types = cfg.type
	for index, team in ipairs(wesnoth.sides.find(cfg)) do
		if unit_types then
			local v = team.recruit
			for _,w in ipairs(unit_types:split()) do
				for i, r in ipairs(v) do
					if r == w then
						table.remove(v, i)
						break
					end
				end
			end
			team.recruit = v
		else
			team.recruit = {}
		end
	end
end

function wml_actions.disallow_extra_recruit(cfg)
	local recruits = cfg.extra_recruit or wml.error("[disallow_extra_recruit] missing required extra_recruit= attribute")
	for index, unit in ipairs(wesnoth.units.find_on_map(cfg)) do
		local v = unit.extra_recruit
		for _,w in ipairs(recruits:split()) do
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
	local recruit = cfg.recruit or wml.error("[set_recruit] missing required recruit= attribute")
	for index, team in ipairs(wesnoth.sides.find(cfg)) do
		team.recruit = recruit:split()
	end
end

function wml_actions.set_extra_recruit(cfg)
	local recruits = cfg.extra_recruit or wml.error("[set_extra_recruit] missing required extra_recruit= attribute")
	local v = recruits:split()

	for index, unit in ipairs(wesnoth.units.find_on_map(cfg)) do
		unit.extra_recruit = v
	end
end

function wml_actions.store_map_dimensions(cfg)
	local var = cfg.variable or "map_size"
	local map = wesnoth.current.map
	wml.variables[var .. ".width"] = map.playable_width
	wml.variables[var .. ".height"] = map.playable_height
	wml.variables[var .. ".border_size"] = map.border_size
end

function wml_actions.unit_worth(cfg)
	local u = wesnoth.units.find_on_map(cfg)[1] or
		wml.error "[unit_worth]'s filter didn't match any unit"
	local ut = wesnoth.unit_types[u.type]
	local hp = u.hitpoints / u.max_hitpoints
	local xp = u.experience / u.max_experience
	local best_adv = ut.cost
	for _,w in ipairs(ut.advances_to) do
		local uta = wesnoth.unit_types[w]
		if uta and uta.cost > best_adv then best_adv = uta.cost end
	end
	wml.variables["cost"] = ut.cost
	wml.variables["next_cost"] = best_adv
	wml.variables["health"] = math.floor(hp * 100)
	wml.variables["experience"] = math.floor(xp * 100)
	wml.variables["recall_cost"] = ut.recall_cost
	wml.variables["unit_worth"] = math.floor(math.max(ut.cost * hp, best_adv * xp))
end

function wml_actions.lua(cfg)
	cfg = wml.shallow_literal(cfg)
	--fixme untested
	local bytecode, message = load(cfg.code or "", cfg.name or nil)
	if not bytecode then error("~lua:" .. message, 0) end
	bytecode(wml.get_child(cfg, "args"))
end

function wml_actions.music(cfg)
	if cfg.play_once then
		wesnoth.audio.music_list.play(cfg.name)
	else
		if not cfg.append then
			if cfg.immediate and wesnoth.audio.music_list.current_i then
				wesnoth.audio.music_list.current.once = true
			end
			wesnoth.audio.music_list.clear()
		end
		local m = #wesnoth.audio.music_list
		wesnoth.audio.music_list.add(cfg.name, not not cfg.immediate, cfg.ms_before or 0, cfg.ms_after or 0)
		local n = #wesnoth.audio.music_list
		if n == 0 then
			return
		end
		if cfg.shuffle == false then
			wesnoth.audio.music_list[n].shuffle = false
		end
		-- Always overwrite shuffle even if the new track couldn't be added,
		-- but title shouldn't be overwritten.
		if cfg.title ~= nil and m ~= n then
			wesnoth.audio.music_list[n].title = cfg.title
		end
	end
end

function wml_actions.volume(cfg)
	if cfg.music then
		local rel = tonumber(cfg.music) or 100.0
		wesnoth.audio.music_list.volume = rel
	end
	if cfg.sound then
		local rel = tonumber(cfg.sound) or 100.0
		wesnoth.audio.volume = rel
	end
end

function wml_actions.scroll_to(cfg)
	local loc = wesnoth.map.find( cfg )[1]
	if not loc then return end
	if not utils.optional_side_filter(cfg) then return end
	wesnoth.interface.scroll_to_hex(loc[1], loc[2], cfg.check_fogged, cfg.immediate)
	if cfg.highlight then
		wesnoth.interface.highlight_hex(loc[1], loc[2])
		wml_actions.redraw{}
	end
end

function wml_actions.scroll_to_unit(cfg)
	local u = wesnoth.units.find_on_map(cfg)[1]
	if not u then return end
	if not utils.optional_side_filter(cfg, "for_side", "for_side") then return end
	wesnoth.interface.scroll_to_hex(u.x, u.y, cfg.check_fogged, cfg.immediate)
	if cfg.highlight then
		wesnoth.interface.highlight_hex(u.x, u.y)
		wml_actions.redraw{}
	end
end

function wml_actions.lock_view(cfg)
	wesnoth.interface.lock(true)
end

function wml_actions.unlock_view(cfg)
	wesnoth.interface.lock(false)
end

function wml_actions.select_unit(cfg)
	local u = wesnoth.units.find_on_map(cfg)[1]
	if not u then return end
	wesnoth.interface.select_unit(u.x, u.y, cfg.highlight, cfg.fire_event)
end

local function get_overlay_object_id(overlay)
	return "overlay_" .. overlay
end

function wml_actions.unit_overlay(cfg)
	local img = cfg.image or wml.error( "[unit_overlay] missing required image= attribute" )
	for i,u in ipairs(wesnoth.units.find_on_map(cfg)) do
		local has_already = false
		for j, w in ipairs(u.overlays) do
			if w == img then has_already = true end
		end
		if has_already == false then
			u:add_modification("object", {
				id = cfg.object_id or get_overlay_object_id(img),
				duration = cfg.duration,
				wml.tag.effect {
					apply_to = "overlay",
					add = img,
				}
			})
		end
	end
end

function wml_actions.remove_unit_overlay(cfg)
	local img = cfg.image or wml.error( "[remove_unit_overlay] missing required image= attribute" )
	for i,u in ipairs(wesnoth.units.find_on_map(cfg)) do
		local has_already = false
		for j, w in ipairs(u.overlays) do
			if w == img then has_already = true end
		end
		if has_already then
			u:add_modification("object", {
				id = cfg.object_id or get_overlay_object_id(img),
				wml.tag.effect {
					apply_to = "overlay",
					remove = img,
				}
			})
		end
	end
end

function wml_actions.store_turns(cfg)
	local var = cfg.variable or "turns"
	wml.variables[var] = wesnoth.scenario.turns
end

function wml_actions.store_unit(cfg)
	local filter = wml.get_child(cfg, "filter") or
		wml.error "[store_unit] missing required [filter] tag"
	local kill_units = cfg.kill

	--cache the needed units here, since the filter might reference the to-be-cleared variable(s)
	local units = wesnoth.units.find(filter)

	local writer = utils.vwriter.init(cfg, "unit")

	for i,u in ipairs(units) do
		local ucfg = u.__cfg
		if u.valid == 'recall' then
			ucfg.x = 'recall'
			ucfg.y = 'recall'
		end
		utils.vwriter.write(writer, ucfg)
		if kill_units then u:erase() end
	end
end

function wml_actions.sound(cfg)
	local name = cfg.name or wml.error("[sound] missing required name= attribute")
	wesnoth.audio.play(name, cfg["repeat"])
end

function wml_actions.store_locations(cfg)
	-- the variable can be mentioned in a [find_in] subtag, so it
	-- cannot be cleared before the locations are recovered
	local locs = wesnoth.map.find(cfg)
	local writer = utils.vwriter.init(cfg, "location")
	for i, loc in ipairs(locs) do
		local x, y = loc[1], loc[2]
		local t = wesnoth.current.map[{x, y}]
		local res = { x = x, y = y, terrain = t }
		if wesnoth.terrain_types[t].village then
			res.owner_side = wesnoth.map.get_owner(x, y) or 0
		end
		utils.vwriter.write(writer, res)
	end
end

function wml_actions.hide_unit(cfg)
	for i,u in ipairs(wesnoth.units.find_on_map(cfg)) do
		u.hidden = true
	end
	wml_actions.redraw {}
end

function wml_actions.unhide_unit(cfg)
	for i,u in ipairs(wesnoth.units.find_on_map(cfg)) do
		u.hidden = false
	end
	wml_actions.redraw {}
end

function wml_actions.capture_village(cfg)
	local side = cfg.side
	local filter_side = wml.get_child(cfg, "filter_side")
	local fire_event = cfg.fire_event
	if side then side = tonumber(side) or wml.error("invalid side in [capture_village]") end
	if filter_side then
		if side then wml.error("duplicate side information in [capture_village]") end
		side = wesnoth.sides.find(filter_side)[1]
		if side then side = side.side end
	end
	local locs = wesnoth.map.find(cfg)

	for i, loc in ipairs(locs) do
		-- The fire_event parameter doesn't currently exist but probably should someday
		---@diagnostic disable-next-line : redundant-parameter
		wesnoth.map.set_owner(loc[1], loc[2], side, fire_event)
	end
end

function wml_actions.terrain(cfg)
	local terrain = cfg.terrain or wml.error("[terrain] missing required terrain= attribute")
	local layer = cfg.layer or 'both'
	if not (wesnoth.terrain_types[terrain] or (layer == "overlay" and terrain == "^")) then
		wml.error("[terrain] invalid terrain="..terrain)
	end
	if layer ~= 'both' and layer ~= 'overlay' and layer ~= 'base' then
		wml.error('[terrain] invalid layer=')
	end
	cfg = wml.shallow_parsed(cfg)
	cfg.terrain = nil
	for i, loc in ipairs(wesnoth.map.find(cfg)) do
		local replacement = cfg.replace_if_failed
			and wesnoth.map.replace_if_failed(terrain, layer)
			or wesnoth.map['replace_' .. layer](terrain)
		wesnoth.current.map[loc] = replacement
	end
end

function wml_actions.delay(cfg)
	local delay = tonumber(cfg.time) or
		wml.error "[delay] missing required time= attribute."
	local accelerate = cfg.accelerate or false
	wesnoth.interface.delay(delay, accelerate)
end

function wml_actions.floating_text(cfg)
	local locs = wesnoth.map.find(cfg)
	local text = cfg.text or wml.error("[floating_text] missing required text= attribute")

	for i, loc in ipairs(locs) do
		wesnoth.interface.float_label(loc[1], loc[2], text, cfg.color)
	end
end

function wml_actions.petrify(cfg)
	for index, unit in ipairs(wesnoth.units.find_on_map(cfg)) do
		unit.status.petrified = true
		-- Extract unit and put it back to update animation (not needed for recall units)
		unit:extract()
		unit:to_map(false)
	end

	for index, unit in ipairs(wesnoth.units.find_on_recall{wml.tag["and"](cfg)}) do
		unit.status.petrified = true
	end
end

function wml_actions.unpetrify(cfg)
	for index, unit in ipairs(wesnoth.units.find_on_map(cfg)) do
		unit.status.petrified = false
		-- Extract unit and put it back to update animation (not needed for recall units)
		unit:extract()
		unit:to_map(false)
	end

	for index, unit in ipairs(wesnoth.units.find_on_recall(cfg)) do
		unit.status.petrified = false
	end
end

function wml_actions.transform_unit(cfg)
	local transform_to = cfg.transform_to

	for index, unit in ipairs(wesnoth.units.find_on_map(cfg)) do

		if transform_to then
			unit:transform(transform_to)
		else
			local hitpoints = unit.hitpoints
			local experience = unit.experience
			local recall_cost = unit.recall_cost
			local status = wml.get_child( unit.__cfg, "status" )

			unit.experience = unit.max_experience
			unit:advance(false, false)

			unit.hitpoints = hitpoints
			unit.experience = experience
			unit.recall_cost = recall_cost

			for key, value in pairs(status) do unit.status[key] = value end
			if unit.status.unpoisonable then unit.status.poisoned = nil end
		end
	end

	wml_actions.redraw {}
end

function wml_actions.store_side(cfg)
	local writer = utils.vwriter.init(cfg, "side")
	for t, side_number in wesnoth.sides.iter(cfg) do
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
	local unit = wesnoth.units.find_on_map(wml.get_child(cfg, "filter"))[1] or
		wml.error("[add_ai_behavior]: no unit specified")

	local side = cfg.side or
		wml.error("[add_ai_behavior]: no side attribute given")

	local sticky = cfg.sticky or false
	local loop_id = cfg.loop_id or "main_loop"
	local eval = cfg.evaluation
	local exec = cfg.execution
	local id = cfg.bca_id or ("bca-" .. unit.__cfg.underlying_id)

	local ux = unit.x -- @note: did I get it right that coordinates in C++ differ by 1 from thos in-game(and in Lua)?
	local uy = unit.y

	if not (eval and exec) then
		wml.error("[add_ai_behavior]: invalid execution/evaluation handler(s)")
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
	for _, side in ipairs(wesnoth.sides.find(cfg)) do
		local loc = side.starting_location
		if loc then
			local result = { x = loc[1], y = loc[2], terrain = wesnoth.current.map[loc] }
			if wesnoth.terrain_types[result.terrain].village then
				result.owner_side = wesnoth.map.get_owner(loc) or 0
			end
			utils.vwriter.write(writer, result)
		end
	end
end

function wml_actions.store_villages( cfg )
	local villages = wesnoth.map.find{gives_income = true, wml.tag['and'](cfg)}
	local writer = utils.vwriter.init(cfg, "location")
	for index, village in ipairs( villages ) do
		utils.vwriter.write(writer, {
			x = village[1],
			y = village[2],
			terrain = wesnoth.current.map[village],
			owner_side = wesnoth.map.get_owner(village) or 0
		})
	end
end

function wml_actions.put_to_recall_list(cfg)
	local units = wesnoth.units.find_on_map(cfg)

	for i, unit in ipairs(units) do
		if cfg.heal then
			unit.hitpoints = unit.max_hitpoints
			unit.moves = unit.max_moves
			unit.attacks_left = unit.max_attacks
			unit.status.poisoned = false
			unit.status.slowed = false
		end
		unit:to_recall(unit.side)
	end
end

function wml_actions.allow_undo(cfg)
	wesnoth.experimental.game_events.set_undoable(true)
end

function wml_actions.disallow_undo(cfg)
	wesnoth.experimental.game_events.set_undoable(false)
end

function wml_actions.allow_end_turn(cfg)
	wesnoth.interface.allow_end_turn(true)
end

function wml_actions.disallow_end_turn(cfg)
	wesnoth.interface.allow_end_turn(cfg.reason or false)
end

function wml_actions.clear_menu_item(cfg)
	wesnoth.interface.clear_menu_item(cfg.id)
end

function wml_actions.set_menu_item(cfg)
	wesnoth.interface.set_menu_item(cfg.id, cfg)
	wesnoth.game_events.add_menu(cfg.id, wml_actions.command)
end

function wml_actions.place_shroud(cfg)
	local sides = utils.get_sides(cfg)
	local tiles = wesnoth.map.find(cfg)
	for i,side in ipairs(sides) do
		side:place_shroud(tiles)
	end
end

function wml_actions.remove_shroud(cfg)
	local sides = utils.get_sides(cfg)
	local tiles = wesnoth.map.find(cfg)
	for i,side in ipairs(sides) do
		side:remove_shroud(tiles)
	end
end

function wml_actions.time_area(cfg)
	if cfg.remove then
		wml_actions.remove_time_area(cfg)
	else
		wesnoth.map.place_area(cfg.id or '', cfg, cfg)
	end
end

function wml_actions.remove_time_area(cfg)
	local id = cfg.id or wml.error("[remove_time_area] missing required id= key")

	for _,w in ipairs(id:split()) do
		wesnoth.map.remove_area(w)
	end
end

function wml_actions.replace_schedule(cfg)
	wesnoth.schedule.replace(cfg)
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
		wesnoth.interface.scroll(cfg.x or 0, cfg.y or 0)
	end
end

function wml_actions.color_adjust(cfg)
	wesnoth.interface.color_adjust(cfg.red or 0, cfg.green or 0, cfg.blue or 0)
end

function wml_actions.screen_fade(cfg)
	local color = {cfg.red or 0, cfg.green or 0, cfg.blue or 0, tonumber(cfg.alpha) or wml.error("invalid alpha in [screen_fade]")}
	wesnoth.interface.screen_fade(color, tonumber(cfg.duration) or wml.error("invalid duration in [screen_fade]"))
end

function wml_actions.end_turn(cfg)
	wesnoth.interface.end_turn()
end

function wml_actions.event(cfg)
	if cfg.remove then
		wesnoth.deprecated_message("[event]remove=yes", 2, "1.17.0", "Use [remove_event] instead of [event]remove=yes")
		wml_actions.remove_event(cfg)
	else
		wesnoth.game_events.add_wml(cfg)
	end
end

function wml_actions.remove_event(cfg)
	local id = cfg.id or wml.error("[remove_event] missing required id= key")

	for _,w in ipairs(id:split()) do
		wesnoth.game_events.remove(w)
	end
end

function wml_actions.inspect(cfg)
	gui.show_inspector(cfg.name)
end

function wml_actions.label( cfg )
	local new_cfg = wml.parsed( cfg )
	for index, location in ipairs( wesnoth.map.find( cfg ) ) do
		new_cfg.x, new_cfg.y = location[1], location[2]
		wesnoth.map.add_label( new_cfg )
	end
end

function wml_actions.open_help(cfg)
	gui.show_help(cfg.topic)
end

function wml_actions.redraw(cfg)
	local clear_shroud = cfg.clear_shroud

	-- Backwards compat, the behavior of the tag was to clear shroud in case that side= is given.
	if cfg.clear_shroud == nil and cfg.side ~= nil then
		clear_shroud = true
	end

	wesnoth.redraw(cfg, clear_shroud)
end

local wml_floating_label = {valid = false}
function wml_actions.print(cfg)
	local options = {}
	if wml_floating_label.valid then
		wml_floating_label:remove()
	end
	if cfg.size then
		options.size = cfg.size
	end
	if cfg.color then
		options.color = stringx.split(cfg.color)
	elseif cfg.red or cfg.green or cfg.blue then
		options.color = {cfg.red or 0, cfg.green or 0, cfg.blue or 0}
	end
	if cfg.duration then
		options.duration = cfg.duration
	end
	if cfg.fade_time then
		options.fade_time = cfg.fade_time
	end

	wml_floating_label = wesnoth.interface.add_overlay_text(cfg.text, options)
end

function wml_actions.unsynced(cfg)
	wesnoth.sync.run_unsynced(function ()
		wml_actions.command(cfg)
	end)
end

wml_actions.unstore_unit = function(cfg)
	local variable = cfg.variable or wml.error("[unstore_unit] missing required 'variable' attribute")
	local unit_cfg = wml.variables[variable] or wml.error("[unstore_unit]: variable '" .. variable .. "' doesn't exist")
	if type(unit_cfg) ~= "table" or unit_cfg.type == nil then
		wml.error("[unstore_unit]: variable '" .. variable .. "' doesn't contain unit data")
	end
	local unit = wesnoth.units.create(unit_cfg)
	local advance = cfg.advance ~= false
	local animate = cfg.animate ~= false
	local check_passability = cfg.check_passability ~= false or nil
	local x = cfg.x or unit.x or -1
	local y = cfg.y or unit.y or -1
	if cfg.location_id then
		x,y = table.unpack(wesnoth.current.map.special_locations[cfg.location_id])
	end
	wesnoth.add_known_unit(unit.type)
	if x ~= 'recall' and y ~= 'recall' and wesnoth.current.map:on_board(x, y) then
		if cfg.find_vacant then
			x,y = wesnoth.paths.find_vacant_hex(x, y, check_passability and unit)
		end
		unit:to_map(x, y, cfg.fire_event)
		local text
		if unit_cfg.gender == "female" then
			text = cfg.female_text or cfg.text
		else
			text = cfg.male_text or cfg.text
		end
		local color = cfg.color
		if color == nil and cfg.red and cfg.blue and cfg.green then
			color = cfg.red .. "," .. cfg.green .. "," .. cfg.blue
		end
		if text ~= nil and not wesnoth.interface.is_skipping_messages() then
			wesnoth.interface.float_label(x, y, text, color)
		end
		if advance then
			unit:advance(animate, cfg.fire_event)
		end
	else
		unit:to_recall()
	end
end

wml_actions.teleport = function(cfg)
	local context = wesnoth.current.event_context
	local filter = wml.get_child(cfg, "filter") or { x = context.x1, y = context.y1 }
	local unit = wesnoth.units.find_on_map(filter)[1]
	if not unit then
		-- No error if no unit matches.
		return
	end
	local x,y = cfg.x, cfg.y
	if cfg.location_id then
		x,y = table.unpack(wesnoth.current.map.special_locations[cfg.location_id])
	end
	unit:teleport(x, y, cfg.check_passability == false, cfg.clear_shroud ~= false, cfg.animate)
end

function wml_actions.remove_sound_source(cfg)
	local ids = cfg.id or wml.error("[remove_sound_source] missing required id= attribute")
	for _,id in ipairs(ids:split()) do
		wesnoth.audio.sources[id] = nil
	end
end

function wml_actions.sound_source(cfg)
	wesnoth.audio.sources[cfg.id] = cfg
end

function wml_actions.deprecated_message(cfg)
	if type(cfg.level) ~= "number" or cfg.level < 1 or cfg.level > 4 then
		local _ = wesnoth.textdomain "wesnoth"
		wml.error(_"Invalid deprecation level (should be 1-4)")
	end
	wesnoth.deprecated_message(cfg.what, cfg.level, cfg.version, cfg.message or '')
end

function wml_actions.wml_message(cfg)
	wesnoth.log(cfg.logger, cfg.message, cfg.to_chat)
end

local function parse_fog_cfg(cfg)
	-- Side filter
	local ssf = wml.get_child(cfg, "filter_side")
	local sides = wesnoth.sides.find(ssf or {})
	-- Location filter
	local locs = wesnoth.map.find(cfg)
	return locs, sides
end

function wml_actions.lift_fog(cfg)
	local locs, sides = parse_fog_cfg(cfg)
	for i = 1, #sides do
		sides[i]:remove_fog(locs, not cfg.multiturn)
	end
end

function wml_actions.reset_fog(cfg)
	local locs, sides = parse_fog_cfg(cfg)
	for i = 1, #sides do
		sides[i]:place_fog(locs, cfg.reset_view)
	end
end

function wesnoth.wml_actions.change_theme(cfg)
	local new_theme = cfg.theme

	if new_theme == nil then
		new_theme = ""
	end

	wesnoth.game_config.theme = new_theme
end

function wesnoth.wml_actions.zoom(cfg)
	wesnoth.interface.zoom(cfg.factor, cfg.relative)
end

function wesnoth.wml_actions.story(cfg)
	local title = cfg.title or wesnoth.scenario.name
	gui.show_story(cfg, title)
end

function wesnoth.wml_actions.cancel_action(cfg)
	wesnoth.cancel_action()
end

function wesnoth.wml_actions.store_unit_defense(cfg)
	wesnoth.deprecated_message("[store_unit_defense]", 3, "1.17.0", "This function returns the chance to be hit, high values represent bad defenses. Using [store_unit_defense_on] is recommended instead.")

	local unit = wesnoth.units.find_on_map(cfg)[1] or wml.error "[store_unit_defense]'s filter didn't match any unit"
	local terrain = cfg.terrain
	local defense

	if terrain then
		defense = unit:chance_to_be_hit(terrain)
	elseif cfg.loc_x and cfg.loc_y then
		defense = unit:chance_to_be_hit(wesnoth.current.map[{cfg.loc_x, cfg.loc_y}])
	else
		defense = unit:chance_to_be_hit(wesnoth.current.map[unit])
	end
	wml.variables[cfg.variable or "terrain_defense"] = defense
end

function wesnoth.wml_actions.store_unit_defense_on(cfg)
	local unit = wesnoth.units.find_on_map(cfg)[1] or wml.error "[store_unit_defense_on]'s filter didn't match any unit"
	local terrain = cfg.terrain
	local defense

	if terrain then
		defense = unit:defense_on(terrain)
	elseif cfg.loc_x and cfg.loc_y then
		defense = unit:defense_on(wesnoth.current.map[{cfg.loc_x, cfg.loc_y}])
	else
		defense = unit:defense_on(wesnoth.current.map[unit])
	end
	wml.variables[cfg.variable or "terrain_defense"] = defense
end

function wml_actions.terrain_mask(cfg)
	cfg = wml.parsed(cfg)

	local alignment = cfg.alignment
	local is_odd = false
	local border = cfg.border
	local mask = cfg.mask
	local x = cfg.x or wml.error("[terrain_mask] missing x attribute")
	local y = cfg.y or wml.error("[terrain_mask] missing y attribute")
	if alignment == "even" then
		is_odd = false
	elseif alignment == "odd" then
		is_odd = true
	elseif alignment == "raw" then
		--todo: maybe rename this value?
		is_odd = (x % 2 ~= 0)
	elseif border == false then
		is_odd = true
	else
		is_odd = false
		-- the old [terrain_mask] code would insert the terrain as one
		-- tile to the northwest from the specified hex.
		-- todo: deprecate this strange behaviour or at least not make it
		--       the default behaviour anymore.
		local new_loc = wesnoth.map.get_direction(x, y, "nw")
		x, y = new_loc.x, new_loc.y
	end
	local rules = {}
	for rule in wml.child_range(cfg, 'rule') do
		rules[#rules + 1] = rule
	end
	if cfg.mask_file then
		local resolved = filesystem.resolve_asset(filesystem.asset_type.MAP, cfg.mask_file)
		resolved = resolved:sub(6) -- strip off 'data/' prefix
		mask = filesystem.read_file(resolved)
	end
	wesnoth.current.map:terrain_mask(x, y, mask, {
		is_odd = is_odd,
		rules = rules,
		ignore_special_locations = cfg.ignore_special_locations,
	})
end

function wml_actions.remove_trait(cfg)
	local obj_id = cfg.trait_id
	for _,unit in ipairs(wesnoth.units.find_on_map(cfg)) do
		unit:remove_modifications({id = obj_id}, "trait")
	end
end

function wml_actions.set_achievement(cfg)
	wesnoth.achievements.set(cfg.content_for, cfg.id)
end

function wml_actions.set_sub_achievement(cfg)
	wesnoth.achievements.set_sub_achievement(cfg.content_for, cfg.id, cfg.sub_id)
end

function wml_actions.progress_achievement(cfg)
	if not tonumber(cfg.amount) then
		wml.error("[progress_achievement] amount attribute not a number for content '"..cfg.content_for.."' and achievement '"..cfg.id.."'")
		return
	end

	wesnoth.achievements.progress(cfg.content_for, cfg.id, cfg.amount, tonumber(cfg.limit) or 999999999)
end

function wml_actions.on_undo(cfg)
	if cfg.delayed_variable_substitution then
		wesnoth.experimental.game_events.add_undo_actions(wml.literal(cfg));
	else
		wesnoth.experimental.game_events.add_undo_actions(wml.parsed(cfg));
	end
end
