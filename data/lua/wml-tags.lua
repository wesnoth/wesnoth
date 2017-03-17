--! #textdomain wesnoth

function wesnoth.game_events.on_load(cfg)
	if #cfg == 0 then return end
	local t = {}
	for i = 1,#cfg do t[i] = string.format("[%s]", cfg[i][1]) end
	error(string.format("~wml:%s not supported at scenario toplevel", table.concat(t, ', ')), 0)
end

function wesnoth.game_events.on_save()
	return {}
end

wesnoth.require "lua/wml-flow.lua"
wesnoth.require "lua/wml/objectives.lua"
wesnoth.require "lua/wml/animate_unit.lua"
wesnoth.require "lua/wml/items.lua"
wesnoth.require "lua/wml/message.lua"
wesnoth.require "lua/wml/object.lua"
wesnoth.require "lua/wml/modify_unit.lua"
wesnoth.require "lua/wml/harm_unit.lua"
wesnoth.require "lua/wml/find_path.lua"
wesnoth.require "lua/wml/endlevel.lua"
wesnoth.require "lua/wml/random_placement.lua"

local helper = wesnoth.require "lua/helper.lua"
local location_set = wesnoth.require "lua/location_set.lua"
local utils = wesnoth.require "lua/wml-utils.lua"
local wml_actions = wesnoth.wml_actions
local T = helper.set_wml_tag_metatable {}

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
	wesnoth.set_music(cfg)
end

-- This is mainly for use in unit test macros, but maybe it can be useful elsewhere too
function wml_actions.test_condition(cfg)
	local logger = cfg.logger or "warning"

	-- This function returns true if it managed to explain the failure
	local function explain(current_cfg, expect)
		for i,t in ipairs(current_cfg) do
			local tag, this_cfg = t[1], t[2]
			-- Some special cases
			if tag == "or" or tag == "and" then
				if explain(this_cfg, expect) then
					return true
				end
			elseif tag == "not" then
				if explain(this_cfg, not expect) then
					return true
				end
			elseif tag == "true" or tag == "false" then
				-- We don't explain these ones.
				return true
			elseif wesnoth.eval_conditional{t} == expect then
				local explanation = "The following conditional test %s:"
				if expect then
					explanation = explanation:format("passed")
				else
					explanation = explanation:format("failed")
				end
				explanation = string.format("%s\n\t[%s]", explanation, tag)
				for k,v in pairs(this_cfg) do
					if type(k) ~= "number" then
						local format = "%s\n\t\t%s=%s"
						local literal = tostring(helper.literal(this_cfg)[k])
						if literal ~= v then
							format = format .. "=%s"
						end
						explanation = string.format(format, explanation, k, literal, tostring(v))
					end
				end
				explanation = string.format("%s\n\t[/%s]", explanation, tag)
				if tag == "variable" then
					explanation = string.format("%s\n\tNote: The variable %s currently has the value %q.", explanation, this_cfg.name, tostring(wesnoth.get_variable(this_cfg.name)))
				end
				wesnoth.log(logger, explanation, true)
				return true
			end
		end
	end

	-- Use not twice here to convert nil to false
	explain(cfg, not not cfg.result)
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

function wml_actions.move_unit(cfg)
	local coordinate_error = "invalid coordinate in [move_unit]"
	local to_x = tostring(cfg.to_x or helper.wml_error(coordinate_error))
	local to_y = tostring(cfg.to_y or helper.wml_error(coordinate_error))
	local fire_event = cfg.fire_event
	local muf_force_scroll = cfg.force_scroll
	local check_passability = cfg.check_passability
	if check_passability == nil then check_passability = true end
	cfg = helper.literal(cfg)
	cfg.to_x, cfg.to_y, cfg.fire_event = nil, nil, nil
	local units = wesnoth.get_units(cfg)

	local pattern = "[^%s,]+"
	for current_unit_index, current_unit in ipairs(units) do
		if not fire_event or current_unit.valid then
			local xs, ys = string.gmatch(to_x, pattern), string.gmatch(to_y, pattern)
			local move_string_x = current_unit.x
			local move_string_y = current_unit.y
			local pass_check = nil
			if check_passability then pass_check = current_unit end

			local x, y = xs(), ys()
			local prevX, prevY = tonumber(current_unit.x), tonumber(current_unit.y)
			while true do
				x = tonumber(x) or helper.wml_error(coordinate_error)
				y = tonumber(y) or helper.wml_error(coordinate_error)
				if not (x == prevX and y == prevY) then x, y = wesnoth.find_vacant_tile(x, y, pass_check) end
				if not x or not y then helper.wml_error("Could not find a suitable hex near to one of the target hexes in [move_unit].") end
				move_string_x = string.format("%s,%u", move_string_x, x)
				move_string_y = string.format("%s,%u", move_string_y, y)
				local next_x, next_y = xs(), ys()
				if not next_x and not next_y then break end
				prevX, prevY = x, y
				x, y = next_x, next_y
			end

			if current_unit.x < x then current_unit.facing = "se"
			elseif current_unit.x > x then current_unit.facing = "sw"
			end

			wesnoth.extract_unit(current_unit)
			local current_unit_cfg = current_unit.__cfg
			wml_actions.move_unit_fake {
				type = current_unit_cfg.type,
				gender = current_unit_cfg.gender,
				variation = current_unit_cfg.variation,
				image_mods = current_unit.image_mods,
				side = current_unit_cfg.side,
				x = move_string_x,
				y = move_string_y,
				force_scroll = muf_force_scroll
			}
			local x2, y2 = current_unit.x, current_unit.y
			current_unit.x, current_unit.y = x, y
			wesnoth.put_unit(current_unit)

			if fire_event then
				wesnoth.fire_event("moveto", x, y, x2, y2)
			end
		end
	end
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

function wml_actions.heal_unit(cfg)
	local healers = helper.get_child(cfg, "filter_second")
	if healers then
		healers = wesnoth.get_units{
			ability_type = "heals",
			T["and"](healers)
		}
	else
		healers = {}
	end

	local who = helper.get_child(cfg, "filter")
	if who then
		who = wesnoth.get_units(who)
	else
		who = wesnoth.get_units{
			x = wesnoth.current.event_context.x1,
			y = wesnoth.current.event_context.y1
		}
	end

	local heal_full = cfg.amount == "full" or cfg.amount == nil
	local moves_full = cfg.moves == "full"
	local heal_amount_set = false
	for i,u in ipairs(who) do
		local heal_amount = u.max_hitpoints - u.hitpoints
		if heal_full then
			u.hitpoints = u.max_hitpoints
		else
			heal_amount = math.min(math.max(1, cfg.amount), heal_amount)
			u.hitpoints = u.hitpoints + heal_amount
		end

		if moves_full then
			u.moves = u.max_moves
		else
			u.moves = math.min(u.max_moves, u.moves + (cfg.moves or 0))
		end

		if cfg.restore_attacks then
			u.attacks_left = u.max_attacks
		end

		if cfg.restore_statuses == true or cfg.restore_statuses == nil then
			u.status.poisoned = false
			u.status.petrified = false
			u.status.slowed = false
			u.status.unhealable = false
		end

		if not heal_amount_set then
			heal_amount_set = true
			wesnoth.set_variable("heal_amount", heal_amount)
		end

		if cfg.animate then
			-- TODO: Make this use the new animation API
			wesnoth.animate_unit{
				T.filter(healers),
				flag = "healing"
			}
		end
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
		container.expenses = t.expenses
		container.total_upkeep = t.total_upkeep
		container.num_units = t.num_units
		container.num_villages = t.num_villages
		container.side = side_number
		utils.vwriter.write(writer, container)
	end
end

function wml_actions.modify_ai(cfg)
	local sides = utils.get_sides(cfg)
	local component, final
	if cfg.action == "add" or cfg.action == "change" then
		local start = string.find(cfg.path, "[a-z_]+%[[a-z0-9_*]*%]$")
		final = start and (string.find(cfg.path, '[', start, true) - 1) or -1
		start = start or string.find(cfg.path, "[^.]*$") or 1
		local comp_type = string.sub(cfg.path, start, final)
		component = helper.get_child(cfg, comp_type)
		if component == nil then
			helper.wml_error("Missing component definition in [modify_ai]")
		end
		component = helper.parsed(component)
	end
	for i = 1, #sides do
		if cfg.action == "add" then
			wesnoth.add_ai_component(sides[i].side, cfg.path, component)
		elseif cfg.action == "delete" or cfg.action == "try_delete" then
			wesnoth.delete_ai_component(sides[i].side, cfg.path)
		elseif cfg.action == "change" then
			local id_start = final + 2
			local id_final = string.len(cfg.path) - 1
			local id = string.sub(cfg.path, id_start, id_final)
			if id == "*" then
				helper.wml_error("[modify_ai] can only change one component at a time")
			elseif not component.id and not id:match("[0-9]+") then
				component.id = id
			end
			wesnoth.change_ai_component(sides[i].side, cfg.path, component)
		end
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
	wesnoth.scroll(cfg)
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

function wml_actions.kill(cfg)
	wesnoth.kill(cfg)
end

function wml_actions.label( cfg )
	local new_cfg = helper.parsed( cfg )
	for index, location in ipairs( wesnoth.get_locations( cfg ) ) do
		new_cfg.x, new_cfg.y = location[1], location[2]
		wesnoth.label( new_cfg )
	end
end

local side_changes_needing_redraw = {
	'shroud', 'fog', 'reset_map', 'reset_view', 'shroud_data',
	'share_vision', 'share_maps', 'share_view',
	'color', 'flag',
}
function wml_actions.modify_side(cfg)
	local sides = utils.get_sides(cfg)
	for i,side in ipairs(sides) do
		if cfg.team_name then
			side.team_name = cfg.team_name
		end
		if cfg.user_team_name then
			side.user_team_name = cfg.user_team_name
		end
		if cfg.controller then
			side.controller = cfg.controller
		end
		if cfg.defeat_condition then
			side.defeat_condition = cfg.defeat_condition
		end
		if cfg.recruit then
			local recruits = {}
			for recruit in utils.split(cfg.recruit) do
				table.insert(recruits, recruit)
			end
			side.recruit = recruits
		end
		if cfg.village_support then
			side.village_support = cfg.village_support
		end
		if cfg.village_gold then
			side.village_gold = cfg.village_gold
		end
		if cfg.income then
			side.base_income = cfg.income
		end
		if cfg.gold then
			side.gold = cfg.gold
		end

		if cfg.hidden ~= nil then
			side.hidden = cfg.hidden
		end
		if cfg.color or cfg.flag then
			wesnoth.set_side_id(side.side, cfg.flag, cfg.color)
		end
		if cfg.flag_icon then
			side.flag_icon = cfg.flag_icon
		end
		if cfg.suppress_end_turn_confirmation ~= nil then
			side.suppress_end_turn_confirmation = cfg.suppress_end_turn_confirmation
		end
		if cfg.scroll_to_leader ~= nil then
			side.scroll_to_leader = cfg.scroll_to_leader
		end

		if cfg.shroud ~= nil then
			side.shroud = cfg.shroud
		end
		if cfg.reset_maps then
			wesnoth.remove_shroud(side.side, "all")
		end
		if cfg.fog ~= nil then
			side.fog = cfg.fog
		end
		if cfg.reset_view then
			wesnoth.add_fog(side.side, {}, true)
		end
		if cfg.shroud_data then
			wesnoth.remove_shroud(side.side, cfg.shroud_data)
		end

		if cfg.share_vision then
			side.share_vision = cfg.share_vision
		end
		-- Legacy support
		if cfg.share_view ~= nil or cfg.share_maps ~= nil then
			if cfg.share_view then
				side.share_vision = 'all'
			elseif cfg.share_maps then
				side.share_vision = 'shroud'
			else
				side.share_vision = 'none'
			end
		end

		if cfg.switch_ai then
			wesnoth.switch_ai(side.side, cfg.switch_ai)
		end
		local ai = {}
		for next_ai in helper.child_range(cfg, "ai") do
			table.insert(ai, T.ai(next_ai))
		end
		if #ai > 0 then
			wesnoth.append_ai(side.side, ai)
		end
	end
	for i,key in ipairs(side_changes_needing_redraw) do
		if cfg[key] ~= nil then
			wml_actions.redraw{}
			return
		end
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

function wml_actions.role(cfg)
	-- role= and type= are handled differently than in other tags,
	-- so we need to remove them from the filter
	local role = cfg.role
	local filter = helper.shallow_literal(cfg)

	if role == nil then
		helper.wml_error("missing role= in [role]")
	end

	local types = {}

	if cfg.type then
		for value in utils.split(cfg.type) do
			table.insert(types, utils.trim(value))
		end
	end

	filter.role, filter.type = nil, nil
	local search_map, search_recall, reassign = true, true, true
	if cfg.search_recall_list == "only" then
		search_map = false
	elseif cfg.search_recall_list ~= nil then
		search_recall = not not cfg.search_recall_list
	end
	if cfg.reassign ~= nil then
		reassign = not not cfg.reassign
	end

	-- pre-build a new [recall] from the [auto_recall]
	-- copy only recall-specific attributes, no SUF at all
	-- the SUF will be id= which we will add in a moment
	-- keep this in sync with the C++ recall function!!!
	local recall = nil
	local child = helper.get_child(cfg, "auto_recall")
	if child ~= nil then
		if helper.get_nth_child(cfg, "auto_recall", 2) ~= nil then
			wesnoth.log("debug", "More than one [auto_recall] found within [role]", true)
		end
		local original = helper.shallow_literal(child)
		recall = {}
		recall.x = original.x
		recall.y = original.y
		recall.show = original.show
		recall.fire_event = original.fire_event
		recall.check_passability = original.check_passability
	end

	if not reassign then
		if search_map then
			local unit = wesnoth.get_units{role=role}[1]
			if unit then
				return
			end
		end
		if recall and search_recall then
			local unit = wesnoth.get_recall_units{role=role}[1]
			if unit then
				recall.id = unit.id
				wml_actions.recall(recall)
				return
			end
		end
	end

	if search_map then
		-- first attempt to match units on the map
		local i = 1
		repeat
			-- give precedence based on the order specified in type=
			if #types > 0 then
				filter.type = types[i]
			end
			local unit = wesnoth.get_units(filter)[1]
			if unit then
				unit.role = role
				return
			end
			i = i + 1
		until #types == 0 or i > #types
	end

	if search_recall then
		-- then try to match units on the recall lists
		i = 1
		repeat
			if #types > 0 then
				filter.type = types[i]
			end
			local unit = wesnoth.get_recall_units(filter)[1]
			if unit then
				unit.role = role
				if recall then
					recall.id = unit.id
					wml_actions.recall(recall)
				end
				return
			end
			i = i + 1
		until #types == 0 or i > #types
	end

	-- no matching unit found, try the [else] tags
	for else_child in helper.child_range(cfg, "else") do
		local action = utils.handle_event_commands(else_child, "conditional")
		if action ~= "none" then return end
	end
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

function wml_actions.set_variable(cfg)
	local name = cfg.name or helper.wml_error "trying to set a variable with an empty name"

	if cfg.value ~= nil then -- check for nil because user may try to set a variable as false
		wesnoth.set_variable(name, cfg.value)
	end

	if cfg.literal ~= nil then
		wesnoth.set_variable(name, helper.shallow_literal(cfg).literal)
	end

	if cfg.to_variable then
		wesnoth.set_variable(name, wesnoth.get_variable(cfg.to_variable))
	end

	if cfg.add then
		wesnoth.set_variable(name, (tonumber(wesnoth.get_variable(name)) or 0) + (tonumber(cfg.add) or 0))
	end

	if cfg.sub then
		wesnoth.set_variable(name, (tonumber(wesnoth.get_variable(name)) or 0) - (tonumber(cfg.sub) or 0))
	end

	if cfg.multiply then
		wesnoth.set_variable(name, (tonumber(wesnoth.get_variable(name)) or 0) * (tonumber(cfg.multiply) or 0))
	end

	if cfg.divide then
		local divide = tonumber(cfg.divide) or 0
		if divide == 0 then helper.wml_error("division by zero on variable " .. name) end
		wesnoth.set_variable(name, (tonumber(wesnoth.get_variable(name)) or 0) / divide)
	end

	if cfg.modulo then
		local modulo = tonumber(cfg.modulo) or 0
		if modulo == 0 then helper.wml_error("division by zero on variable " .. name) end
		wesnoth.set_variable(name, (tonumber(wesnoth.get_variable(name)) or 0) % modulo)
	end

	if cfg.abs then
		wesnoth.set_variable(name, math.abs(tonumber(wesnoth.get_variable(name)) or 0))
	end

	if cfg.root then
		if cfg.root == "square" then
			local radicand = tonumber(wesnoth.get_variable(name)) or 0
			if radicand < 0 then helper.wml_error("square root of negative number on variable " .. name) end
			wesnoth.set_variable(name, math.sqrt(radicand))
		end
	end

	if cfg.power then
		wesnoth.set_variable(name, (tonumber(wesnoth.get_variable(name)) or 0) ^ (tonumber(cfg.power) or 0))
	end

	if cfg.round then
		local var = tonumber(wesnoth.get_variable(name) or 0)
		local round_val = cfg.round
		if round_val == "ceil" then
			wesnoth.set_variable(name, math.ceil(var))
		elseif round_val == "floor" then
			wesnoth.set_variable(name, math.floor(var))
		else
			local decimals, discarded = math.modf(tonumber(round_val) or 0)
			local value = var * (10 ^ decimals)
			value = helper.round(value)
			value = value * (10 ^ -decimals)
			wesnoth.set_variable(name, value)
		end
	end

	-- unlike the other math operations, ipart and fpart do not act on
	-- the value already contained in the variable
	-- but on the value assigned to the respective key
	if cfg.ipart then
		local ivalue, fvalue = math.modf(tonumber(cfg.ipart) or 0)
		wesnoth.set_variable(name, ivalue)
	end

	if cfg.fpart then
		local ivalue, fvalue = math.modf(tonumber(cfg.fpart) or 0)
		wesnoth.set_variable(name, fvalue)
	end

	if cfg.string_length ~= nil then
		wesnoth.set_variable(name, string.len(tostring(cfg.string_length)))
	end

	if cfg.time then
		if cfg.time == "stamp" then
			wesnoth.set_variable(name, wesnoth.get_time_stamp())
		end
	end

	if cfg.rand then
		wesnoth.set_variable(name, helper.rand(tostring(cfg.rand)))
	end

	local join_child = helper.get_child(cfg, "join")
	if join_child then
		local array_name = join_child.variable or helper.wml_error "missing variable= attribute in [join]"
		local separator = join_child.separator
		local key_name = join_child.key or "value"
		local remove_empty = join_child.remove_empty

		local string_to_join = {}

		for i, element in ipairs(helper.get_variable_array(array_name)) do
			if element[key_name] ~= nil or (not remove_empty) then
				table.insert(string_to_join, tostring(element[key_name]))
			end
		end

		wesnoth.set_variable(name, table.concat(string_to_join, separator))
	end
end

function wesnoth.wml_conditionals.proceed_to_next_scenario(cfg)
	local endlevel_data = wesnoth.get_end_level_data()
	if not endlevel_data then
		return false
	else
		return endlevel_data.proceed_to_next_level
	end
end
