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

wesnoth.require "lua/wml/objectives.lua"
wesnoth.require "lua/wml/items.lua"
wesnoth.require "lua/wml/message.lua"
wesnoth.require "lua/wml/object.lua"

local helper = wesnoth.require "lua/helper.lua"
local location_set = wesnoth.require "lua/location_set.lua"
local utils = wesnoth.require "lua/wml-utils.lua"
local wml_actions = wesnoth.wml_actions

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

	wesnoth.fire_event(cfg.name, x1, y1, x2, y2, w1, w2)
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

function wml_actions.wml_action(cfg)
	-- The new tag's name
	local name = cfg.name or
		helper.wml_error "[wml_action] missing required name= attribute."
	local code = cfg.lua_function or
		helper.wml_error "[wml_action] missing required lua_function= attribute."
	local bytecode, message = load(code)
	if not bytecode then
		helper.wml_error("[wml_action] failed to compile Lua code: " .. message)
	end
	-- The lua function that is executed when the tag is called
	local lua_function = bytecode() or
		helper.wml_error "[wml_action] expects a Lua code returning a function."
	wml_actions[name] = lua_function
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

wml_actions.command = utils.handle_event_commands

-- since if and while are Lua keywords, we can't create functions with such names
-- instead, we store the following anonymous functions directly into
-- the table, using the [] operator, rather than by using the point syntax
-- the same is true of for and repeat

wml_actions["if"] = function(cfg)
	if not (helper.get_child(cfg, 'then') or helper.get_child(cfg, 'elseif') or helper.get_child(cfg, 'else')) then
		helper.wml_error("[if] didn't find any [then], [elseif], or [else] children.")
	end

	if wesnoth.eval_conditional(cfg) then -- evaluate [if] tag
		for then_child in helper.child_range(cfg, "then") do
			utils.handle_event_commands(then_child)
		end
		return -- stop after executing [then] tags
	end

	for elseif_child in helper.child_range(cfg, "elseif") do
		if wesnoth.eval_conditional(elseif_child) then -- we'll evaluate the [elseif] tags one by one
			for then_tag in helper.child_range(elseif_child, "then") do
				utils.handle_event_commands(then_tag)
			end
			return -- stop on first matched condition
		end
	end

	-- no matched condition, try the [else] tags
	for else_child in helper.child_range(cfg, "else") do
		utils.handle_event_commands(else_child)
	end
end

wml_actions["while"] = function( cfg )
	-- execute [do] up to 65536 times
	for i = 1, 65536 do
		if wesnoth.eval_conditional( cfg ) then
			for do_child in helper.child_range( cfg, "do" ) do
				utils.handle_event_commands( do_child )
			end
		else return end
	end
end

wesnoth.wml_actions["for"] = function(cfg)
	local first, last, step
	if cfg.array then
		first = 0
		last = wesnoth.get_variable(cfg.array .. ".length") - 1
		step = 1
		if cfg.reverse == "yes" then
			first, last = last, first
			step = -1
		end
	else
		first = cfg.start or 0
		last = cfg["end"] or first
		step = cfg.step or ((last - first) / math.abs(last - first))
	end
	local i_var = cfg.variable or "i"
	local save_i = start_var_scope(i_var)
	wesnoth.set_variable(i_var, first)
	while wesnoth.get_variable(i_var) <= last do
		for do_child in helper.child_range( cfg, "do" ) do
			handle_event_commands( do_child )
		end
		wesnoth.set_variable(i_var, wesnoth.get_variable(i_var) + 1)
	end
	end_var_scope(i_var, save_i)
end

wml_actions["repeat"] = function(cfg)
	local times = cfg.times or 1
	for i = 1, times do
		handle_event_commands(cfg)
	end
end

function wml_actions.foreach(cfg)
	local array_name = cfg.variable or helper.wml_error "[foreach] missing required variable= attribute"
	local array = helper.get_variable_array(array_name)
	if #array == 0 then return end -- empty and scalars unwanted
	local item_name = cfg.item_var or "this_item"
	local this_item = start_var_scope(item_name) -- if this_item is already set
	local i_name = cfg.index_var or "i"
	local i = start_var_scope(i_name) -- if i is already set
	local array_length = wesnoth.get_variable(array_name .. ".length")
	
	for index, value in ipairs(array) do
		-- Some protection against external modification
		-- It's not perfect, though - it'd be nice if *any* change could be detected
		if array_length ~= wesnoth.get_variable(array_name .. ".length") then
			helper.wml_error("WML array length changed during [foreach] iteration")
		end
		wesnoth.set_variable(item_name, value)
		-- set index variable
		wesnoth.set_variable(i_name, index-1) -- here -1, because of WML array
		-- perform actions
		for do_child in helper.child_range(cfg, "do") do
			handle_event_commands(do_child, "loop")
		end
		-- set back the content, in case the author made some modifications
		if not cfg.readonly then
			array[index] = wesnoth.get_variable(item_name)
		end
	end
	
	-- house cleaning
	wesnoth.set_variable(item_name)
	wesnoth.set_variable(i)
	
	-- restore the array
	helper.set_variable_array(array_name, array)
end

function wml_actions.switch(cfg)
	local var_value = wesnoth.get_variable(cfg.variable)
	local found = false

	-- Execute all the [case]s where the value matches.
	for v in helper.child_range(cfg, "case") do
		for w in utils.split(v.value) do
			if w == tostring(var_value) then 
				utils.handle_event_commands(v)
				found = true
				break 
			end
		end
	end

	-- Otherwise execute [else] statements.
	if not found then
		for v in helper.child_range(cfg, "else") do
			utils.handle_event_commands(v)
		end
	end
end

function wml_actions.scroll_to(cfg)
	local loc = wesnoth.get_locations( cfg )[1]
	if not loc then return end
	if not utils.optional_side_filter(cfg) then return end
	wesnoth.scroll_to_tile(loc[1], loc[2], cfg.check_fogged, cfg.immediate)
end

function wml_actions.scroll_to_unit(cfg)
	local u = wesnoth.get_units(cfg)[1]
	if not u then return end
	if not utils.optional_side_filter(cfg, "for_side", "for_side") then return end
	wesnoth.scroll_to_tile(u.x, u.y, cfg.check_fogged, cfg.immediate)
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
	wesnoth.select_hex(u.x, u.y, cfg.highlight, cfg.fire_event)
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
		if kill_units then wesnoth.put_unit(u.x, u.y) end
	end

	if (not filter.x or filter.x == "recall") and (not filter.y or filter.y == "recall") then
		for i,u in ipairs(recall_units) do
			local ucfg = u.__cfg
			ucfg.x = "recall"
			ucfg.y = "recall"
			utils.vwriter.write(writer, ucfg)
			if kill_units then wesnoth.extract_unit(u) end
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

function wml_actions.modify_unit(cfg)
	local unit_variable = "LUA_modify_unit"

	local function handle_attributes(cfg, unit_path, toplevel)
		for current_key, current_value in pairs(helper.shallow_parsed(cfg)) do
			if type(current_value) ~= "table" and (not toplevel or current_key ~= "type") then
				wesnoth.set_variable(string.format("%s.%s", unit_path, current_key), current_value)
			end
		end
	end

	local function handle_child(cfg, unit_path)
		local children_handled = {}
		local cfg = helper.shallow_parsed(cfg)
		handle_attributes(cfg, unit_path)

		for current_index, current_table in ipairs(cfg) do
			local current_tag = current_table[1]
			local tag_index = children_handled[current_tag] or 0
			handle_child(current_table[2], string.format("%s.%s[%u]",
				unit_path, current_tag, tag_index))
			children_handled[current_tag] = tag_index + 1
		end
	end

	local filter = helper.get_child(cfg, "filter") or helper.wml_error "[modify_unit] missing required [filter] tag"
	local function handle_unit(unit_num)
		local children_handled = {}
		local unit_path = string.format("%s[%u]", unit_variable, unit_num)
		local this_unit = wesnoth.get_variable(unit_path)
		wesnoth.set_variable("this_unit", this_unit)
		handle_attributes(cfg, unit_path, true)

		for current_index, current_table in ipairs(helper.shallow_parsed(cfg)) do
			local current_tag = current_table[1]
			if current_tag == "filter" then
				-- nothing
			elseif current_tag == "object" or current_tag == "trait" or current_tag == "advancement" then
				local unit = wesnoth.get_variable(unit_path)
				unit = wesnoth.create_unit(unit)
				wesnoth.add_modification(unit, current_tag, current_table[2])
				unit = unit.__cfg;
				wesnoth.set_variable(unit_path, unit)
			else
				local tag_index = children_handled[current_tag] or 0
				handle_child(current_table[2], string.format("%s.%s[%u]",
					unit_path, current_tag, tag_index))
				children_handled[current_tag] = tag_index + 1
			end
		end

		if cfg.type then
			if cfg.type ~= "" then wesnoth.set_variable(unit_path .. ".advances_to", cfg.type) end
			wesnoth.set_variable(unit_path .. ".experience", wesnoth.get_variable(unit_path .. ".max_experience"))
		end
		wml_actions.kill({ id = this_unit.id, animate = false })
		wml_actions.unstore_unit { variable = unit_path }
	end

	wml_actions.store_unit { {"filter", filter}, variable = unit_variable }
	local max_index = wesnoth.get_variable(unit_variable .. ".length") - 1

	local this_unit = utils.start_var_scope("this_unit")
	for current_unit = 0, max_index do
		handle_unit(current_unit)
	end
	utils.end_var_scope("this_unit", this_unit)

	wesnoth.set_variable(unit_variable)
end

function wml_actions.move_unit(cfg)
	local coordinate_error = "invalid coordinate in [move_unit]"
	local to_x = tostring(cfg.to_x or helper.wml_error(coordinate_error))
	local to_y = tostring(cfg.to_y or helper.wml_error(coordinate_error))
	local fire_event = cfg.fire_event
	local muf_force_scroll = cfg.force_scroll
	local check_passability = cfg.check_passability or true
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
		wesnoth.float_label(loc[1], loc[2], text)
	end
end

function wml_actions.petrify(cfg)
	for index, unit in ipairs(wesnoth.get_units(cfg)) do
		unit.status.petrified = true
		-- Extract unit and put it back to update animation (not needed for recall units)
		wesnoth.extract_unit(unit)
		wesnoth.put_unit(unit, unit.x, unit.y)
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
		wesnoth.put_unit(unit, unit.x, unit.y)
	end

	for index, unit in ipairs(wesnoth.get_recall_units(cfg)) do
		unit.status.petrified = false
	end
end

function wml_actions.harm_unit(cfg)
	local filter = helper.get_child(cfg, "filter") or helper.wml_error("[harm_unit] missing required [filter] tag")
	-- we need to use shallow_literal field, to avoid raising an error if $this_unit (not yet assigned) is used
	if not cfg.__shallow_literal.amount then helper.wml_error("[harm_unit] has missing required amount= attribute") end
	local variable = cfg.variable -- kept out of the way to avoid problems
	local _ = wesnoth.textdomain "wesnoth"
	-- #textdomain wesnoth
	local harmer

	local function toboolean( value ) -- helper for animate fields
		-- units will be animated upon leveling or killing, even
		-- with special attacker and defender values
		if value then return true
		else return false end
	end

	local this_unit = utils.start_var_scope("this_unit")

	for index, unit_to_harm in ipairs(wesnoth.get_units(filter)) do
		if unit_to_harm.valid then
			-- block to support $this_unit
			wesnoth.set_variable ( "this_unit" ) -- clearing this_unit
			wesnoth.set_variable("this_unit", unit_to_harm.__cfg) -- cfg field needed
			local amount = tonumber(cfg.amount)
			local animate = cfg.animate -- attacker and defender are special values
			local delay = cfg.delay or 500
			local kill = cfg.kill
			local fire_event = cfg.fire_event
			local primary_attack = helper.get_child(cfg, "primary_attack")
			local secondary_attack = helper.get_child(cfg, "secondary_attack")
			local harmer_filter = helper.get_child(cfg, "filter_second")
			local experience = cfg.experience
			local resistance_multiplier = tonumber(cfg.resistance_multiplier) or 1
			if harmer_filter then harmer = wesnoth.get_units(harmer_filter)[1] end
			-- end of block to support $this_unit

			if animate then
				if animate ~= "defender" and harmer and harmer.valid then
					wesnoth.scroll_to_tile(harmer.x, harmer.y, true)
					wesnoth.animate_unit({ flag = "attack", hits = true, { "filter", { id = harmer.id } },
						{ "primary_attack", primary_attack },
						{ "secondary_attack", secondary_attack }, with_bars = true,
						{ "facing", { x = unit_to_harm.x, y = unit_to_harm.y } } })
				end
				wesnoth.scroll_to_tile(unit_to_harm.x, unit_to_harm.y, true)
			end

			-- the two functions below are taken straight from the C++ engine, utils.cpp and actions.cpp, with a few unuseful parts removed
			-- may be moved in helper.lua in 1.11
			local function round_damage( base_damage, bonus, divisor )
				local rounding
				if base_damage == 0 then return 0
				else
					if bonus < divisor or divisor == 1 then
						rounding = divisor / 2 - 0
					else
						rounding = divisor / 2 - 1
					end
					return math.max( 1, math.floor( ( base_damage * bonus + rounding ) / divisor ) )
				end
			end

			local function calculate_damage( base_damage, alignment, tod_bonus, resistance, modifier )
				local damage_multiplier = 100
				if alignment == "lawful" then
					damage_multiplier = damage_multiplier + tod_bonus
				elseif alignment == "chaotic" then
					damage_multiplier = damage_multiplier - tod_bonus
				elseif alignment == "liminal" then
					damage_multiplier = damage_multiplier - math.abs( tod_bonus )
				else -- neutral, do nothing
				end
				local resistance_modified = resistance * modifier
				damage_multiplier = damage_multiplier * resistance_modified
				local damage = round_damage( base_damage, damage_multiplier, 10000 ) -- if harmer.status.slowed, this may be 20000 ?
				return damage
			end

			local damage = calculate_damage( amount,
							 ( cfg.alignment or "neutral" ),
							 wesnoth.get_time_of_day( { unit_to_harm.x, unit_to_harm.y, true } ).lawful_bonus,
							 wesnoth.unit_resistance( unit_to_harm, cfg.damage_type or "dummy" ),
							 resistance_multiplier
						       )

			if unit_to_harm.hitpoints <= damage then
				if kill == false then damage = unit_to_harm.hitpoints - 1
				else damage = unit_to_harm.hitpoints
				end
			end

			unit_to_harm.hitpoints = unit_to_harm.hitpoints - damage
			local text = string.format("%d%s", damage, "\n")
			local add_tab = false
			local gender = unit_to_harm.__cfg.gender

			local function set_status(name, male_string, female_string, sound)
				if not cfg[name] or unit_to_harm.status[name] then return end
				if gender == "female" then
					text = string.format("%s%s%s", text, tostring(female_string), "\n")
				else
					text = string.format("%s%s%s", text, tostring(male_string), "\n")
				end

				unit_to_harm.status[name] = true
				add_tab = true

				if animate and sound then -- for unhealable, that has no sound
					wesnoth.play_sound (sound)
				end
			end

			if not unit_to_harm.status.unpoisonable then
				set_status("poisoned", _"poisoned", _"female^poisoned", "poison.ogg")
			end
			set_status("slowed", _"slowed", _"female^slowed", "slowed.wav")
			set_status("petrified", _"petrified", _"female^petrified", "petrified.ogg")
			set_status("unhealable", _"unhealable", _"female^unhealable")

			-- Extract unit and put it back to update animation if status was changed
			wesnoth.extract_unit(unit_to_harm)
			wesnoth.put_unit(unit_to_harm, unit_to_harm.x, unit_to_harm.y)

			if add_tab then
				text = string.format("%s%s", "\t", text)
			end

			if animate and animate ~= "attacker" then
				if harmer and harmer.valid then
					wesnoth.animate_unit({ flag = "defend", hits = true, { "filter", { id = unit_to_harm.id } },
						{ "primary_attack", primary_attack },
						{ "secondary_attack", secondary_attack }, with_bars = true },
						{ "facing", { x = harmer.x, y = harmer.y } })
				else
					wesnoth.animate_unit({ flag = "defend", hits = true, { "filter", { id = unit_to_harm.id } },
						{ "primary_attack", primary_attack },
						{ "secondary_attack", secondary_attack }, with_bars = true })
				end
			end

			wesnoth.float_label( unit_to_harm.x, unit_to_harm.y, string.format( "<span foreground='red'>%s</span>", text ) )

			local function calc_xp( level ) -- to calculate the experience in case of kill
				if level == 0 then return 4
				else return level * 8 end
			end

			if experience ~= false and harmer and harmer.valid and wesnoth.is_enemy( unit_to_harm.side, harmer.side ) then -- no XP earned for harming friendly units
				if kill ~= false and unit_to_harm.hitpoints <= 0 then
					harmer.experience = harmer.experience + calc_xp( unit_to_harm.__cfg.level )
				else
					unit_to_harm.experience = unit_to_harm.experience + harmer.__cfg.level
					harmer.experience = harmer.experience + unit_to_harm.__cfg.level
				end
			end

			if kill ~= false and unit_to_harm.hitpoints <= 0 then
				wml_actions.kill({ id = unit_to_harm.id, animate = toboolean( animate ), fire_event = fire_event })
			end

			if animate then
				wesnoth.delay(delay)
			end

			if variable then
				wesnoth.set_variable(string.format("%s[%d]", variable, index - 1), { harm_amount = damage })
			end

			-- both may no longer be alive at this point, so double check
			-- this blocks handles the harmed units advancing
			if experience ~= false and harmer and unit_to_harm.valid and unit_to_harm.experience >= unit_to_harm.max_experience then
				wml_actions.store_unit { { "filter", { id = unit_to_harm.id } }, variable = "Lua_store_unit", kill = true }
				wml_actions.unstore_unit { variable = "Lua_store_unit",
								find_vacant = false,
								advance = true,
								animate = toboolean( animate ),
								fire_event = fire_event }
				wesnoth.set_variable ( "Lua_store_unit", nil )
			end

			-- this block handles the harmer advancing
			if experience ~= false and harmer and harmer.valid and harmer.experience >= harmer.max_experience then
				wml_actions.store_unit { { "filter", { id = harmer.id } }, variable = "Lua_store_unit", kill = true }
				wml_actions.unstore_unit { variable = "Lua_store_unit",
								find_vacant = false,
								advance = true,
								animate = toboolean( animate ),
								fire_event = fire_event }
				wesnoth.set_variable ( "Lua_store_unit", nil )
			end
		end

		wml_actions.redraw {}
	end

	wesnoth.set_variable ( "this_unit" ) -- clearing this_unit
	utils.end_var_scope("this_unit", this_unit)
end

function wml_actions.heal_unit(cfg)
	wesnoth.heal_unit(cfg)
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
			wml_actions.advance_unit(unit, false, false)

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
		local container = {
				controller = t.controller,
				recruit = table.concat(t.recruit, ","),
				fog = t.fog,
				shroud = t.shroud,
				hidden = t.hidden,
				income = t.total_income,
				village_gold = t.village_gold,
				village_support = t.village_support,
				name = t.name,
				team_name = t.team_name,
				user_team_name = t.user_team_name,
				color = t.color,
				gold = t.gold,
				scroll_to_leader = t.scroll_to_leader,
				flag = t.flag,
				flag_icon = t.flag_icon,
				side = side_number
			}
		utils.vwriter.write(writer, container)
	end
end

-- This is the port of the old [modify_ai] into lua. It is different from wesnoth.modify_ai in that it uses a standard side filter.
-- I don't know why these functions were made to behave differently, but this seems to be the more powerful and useful one according
-- to mattsc's comments 
function wml_actions.modify_ai(cfg)
	wesnoth.modify_ai_wml(cfg)
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

	local conf = {
		["action"] = "add",
		["engine"] = "lua",
		["path"] = path,

		{"candidate_action", {
			["id"] = id,
			["name"] = id,
			["engine"] = "lua",
			["sticky"] = sticky,
			["unit_x"] = ux,
			["unit_y"] = uy,
			["evaluation"] = eval,
			["execution"] = exec
		}},

		["side"] = side
	}
	wesnoth.wml_actions.modify_ai(conf)
	--wesnoth.message("Adding a behavior")
end

function wml_actions.find_path(cfg)
	local filter_unit = (helper.get_child(cfg, "traveler")) or helper.wml_error("[find_path] missing required [traveler] tag")
	-- only the first unit matching
	local unit = wesnoth.get_units(filter_unit)[1] or helper.wml_error("[find_path]'s filter didn't match any unit")
	local filter_location = (helper.get_child(cfg, "destination")) or helper.wml_error( "[find_path] missing required [destination] tag" )
	-- support for $this_unit
	local this_unit = utils.start_var_scope("this_unit")

	wesnoth.set_variable ( "this_unit" ) -- clearing this_unit
	wesnoth.set_variable("this_unit", unit.__cfg) -- cfg field needed

	local variable = cfg.variable or "path"
	local ignore_units = false
	local ignore_teleport = false

	if cfg.check_zoc == false then --if we do not want to check the ZoCs, we must ignore units
		ignore_units = true
	end
	if cfg.check_teleport == false then --if we do not want to check teleport, we must ignore it
		ignore_teleport = true
	end

	local allow_multiple_turns = cfg.allow_multiple_turns
	local viewing_side

	if not cfg.check_visibility then viewing_side = 0 end -- if check_visiblity then shroud is taken in account

	local locations = wesnoth.get_locations(filter_location) -- only the location with the lowest distance and lowest movement cost will match. If there will still be more than 1, only the 1st maching one.
	local max_cost = nil
	if not allow_multiple_turns then max_cost = unit.moves end --to avoid wrong calculation on already moved units
	local current_distance, current_cost = math.huge, math.huge
	local current_location = {}

	local width,heigth,border = wesnoth.get_map_size() -- data for test below

	for index, location in ipairs(locations) do
		-- we test if location passed to pathfinder is invalid (border); if is, do nothing, do not return and continue the cycle
		if location[1] == 0 or location[1] == ( width + 1 ) or location[2] == 0 or location[2] == ( heigth + 1 ) then
		else
			local distance = helper.distance_between ( unit.x, unit.y, location[1], location[2] )
			-- if we pass an unreachable locations an high value will be returned
			local path, cost = wesnoth.find_path( unit, location[1], location[2], { max_cost = max_cost, ignore_units = ignore_units, ignore_teleport = ignore_teleport, viewing_side = viewing_side } )

			if ( distance < current_distance and cost <= current_cost ) or ( cost < current_cost and distance <= current_distance ) then -- to avoid changing the hex with one with less distance and more cost, or vice versa
				current_distance = distance
				current_cost = cost
				current_location = location
			end
		end
	end

	if #current_location == 0 then wesnoth.message("WML warning","[find_path]'s filter didn't match any location")
	else
		local path, cost = wesnoth.find_path( unit, current_location[1], current_location[2], { max_cost = max_cost, ignore_units = ignore_units, ignore_teleport = ignore_teleport, viewing_side = viewing_side } )
		local turns

		if cost == 0 then -- if location is the same, of course it doesn't cost any MP
			turns = 0
		else
			turns = math.ceil( ( ( cost - unit.moves ) / unit.max_moves ) + 1 )
		end

		if cost >= 42424242 then -- it's the high value returned for unwalkable or busy terrains
			wesnoth.set_variable ( string.format("%s", variable), { hexes = 0 } ) -- set only length, nil all other values
			-- support for $this_unit
			wesnoth.set_variable ( "this_unit" ) -- clearing this_unit
			utils.end_var_scope("this_unit", this_unit)
		return end

		if not allow_multiple_turns and turns > 1 then -- location cannot be reached in one turn
			wesnoth.set_variable ( string.format("%s", variable), { hexes = 0 } )
			-- support for $this_unit
			wesnoth.set_variable ( "this_unit" ) -- clearing this_unit
			utils.end_var_scope("this_unit", this_unit)
		return end -- skip the cycles below

		wesnoth.set_variable ( string.format( "%s", variable ), { hexes = current_distance, from_x = unit.x, from_y = unit.y, to_x = current_location[1], to_y = current_location[2], movement_cost = cost, required_turns = turns } )

		for index, path_loc in ipairs(path) do
			local sub_path, sub_cost = wesnoth.find_path( unit, path_loc[1], path_loc[2], { max_cost = max_cost, ignore_units = ignore_units, ignore_teleport = ignore_teleport, viewing_side = viewing_side } )
			local sub_turns

			if sub_cost == 0 then
				sub_turns = 0
			else
				sub_turns = math.ceil( ( ( sub_cost - unit.moves ) / unit.max_moves ) + 1 )
			end

			wesnoth.set_variable ( string.format( "%s.step[%d]", variable, index - 1 ), { x = path_loc[1], y = path_loc[2], terrain = wesnoth.get_terrain( path_loc[1], path_loc[2] ), movement_cost = sub_cost, required_turns = sub_turns } ) -- this structure takes less space in the inspection window
		end
	end

	-- support for $this_unit
	wesnoth.set_variable ( "this_unit" ) -- clearing this_unit
	utils.end_var_scope("this_unit", this_unit)
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
		wesnoth.put_unit(unit.x, unit.y)
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
	wesnoth.place_shroud(cfg)
end

function wml_actions.remove_shroud(cfg)
	wesnoth.remove_shroud(cfg)
end

function wml_actions.time_area(cfg)
	local remove = cfg.remove
	if remove then
		wesnoth.remove_time_area(cfg.id)
	else
		wesnoth.add_time_area(cfg)
	end
end

function wml_actions.replace_schedule(cfg)
	wesnoth.replace_schedule(cfg)
end

function wml_actions.scroll(cfg)
	wesnoth.scroll(cfg)
end

function wml_actions.animate_unit(cfg)
	wesnoth.animate_unit(cfg)
end

function wml_actions.color_adjust(cfg)
	wesnoth.color_adjust(cfg)
end

function wml_actions.end_turn(cfg)
	wesnoth.end_turn()
end

function wml_actions.endlevel(cfg)
	if wesnoth.check_end_level_disabled() then
		wesnoth.message("Repeated [endlevel] execution, ignoring")
		return
	end

	local next_scenario = cfg.next_scenario
	if next_scenario then
		wesnoth.set_next_scenario(next_scenario)
	end

	local end_text = cfg.end_text
	local end_text_duration = cfg.end_text_duration
	if end_text or end_text_duration then
		wesnoth.set_end_campaign_text(end_text or "", end_text_duration)
	end

	local end_credits = cfg.end_credits
	if end_credits ~= nil then
		wesnoth.set_end_campaign_credits(end_credits)
	end

	wesnoth.end_level(cfg)
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

function wml_actions.modify_side(cfg)
	wesnoth.modify_side(cfg)
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

	local types = {}
	for value in utils.split(cfg.type) do
		table.insert(types, utils.trim(value))
	end

	filter.role, filter.type = nil, nil

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

	-- then try to match units on the recall lists
	i = 1
	repeat
		if #types > 0 then
			filter.type = types[i]
		end
		local unit = wesnoth.get_recall_units(filter)[1]
		if unit then
			unit.role = role
			return
		end
		i = i + 1
	until #types == 0 or i > #types

	-- no matching unit found, fail silently
end

function wml_actions.unsynced(cfg)
	wesnoth.unsynced(function ()
		wml_actions.command(cfg)
	end)
end