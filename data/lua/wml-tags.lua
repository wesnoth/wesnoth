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

local helper = wesnoth.require "lua/helper.lua"
local location_set = wesnoth.require "lua/location_set.lua"
local wml_actions = wesnoth.wml_actions

local function trim(s)
	local r = string.gsub(s, "^%s*(.-)%s*$", "%1")
	return r
end

local function optional_side_filter(cfg, key_name, filter_name)
	local key_name = key_name or "side"
	local sides = cfg[key_name]
	local filter_name = filter_name or "filter_side"
	local filter_side = helper.get_child(cfg, filter_name)
	if filter_side then
		sides = wesnoth.get_sides(filter_side)
	elseif sides then
		local dummy_cfg = {side=sides}
		sides = wesnoth.get_sides(dummy_cfg)
	else
		return true
	end
	for index,side in ipairs(sides) do
		if side.controller == "human" then
			return true
		end
	end
	return false
end

local engine_message = wml_actions.message

function wml_actions.message(cfg)
	local show_if = helper.get_child(cfg, "show_if")
	if not show_if or wesnoth.eval_conditional(show_if) then
		engine_message(cfg)
	end
end

function wml_actions.chat(cfg)
	local side_list = wesnoth.get_sides(cfg)
	local message = tostring(cfg.message) or
		helper.wml_error "[chat] missing required message= attribute."

	local speaker = cfg.speaker
	if speaker then
		speaker = tostring(speaker)
	else
		speaker = "WML"
	end

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
	local sides = cfg.side
	local filter_side = helper.get_child(cfg, "filter_side")
	if filter_side then
		wesnoth.message("warning", "[gold][filter_side] is deprecated, use only an inline SSF")
		if sides then helper.wml_error("duplicate side information in [gold]") end
		sides = wesnoth.get_sides(filter_side)
	else
		sides = wesnoth.get_sides(cfg)
	end
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
	for w in string.gmatch(names, "[^%s,][^,]*") do
		wesnoth.set_variable(trim(w))
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
	local var = cfg.variable or "unit_type"
	local types = cfg.type or
		helper.wml_error "[store_unit_type] missing required type= attribute."
	wesnoth.set_variable(var)
	local i = 0
	for w in string.gmatch(types, "[^%s,][^,]*") do
		local unit_type = wesnoth.unit_types[w] or
			helper.wml_error(string.format("Attempt to store nonexistent unit type '%s'.", w))
		wesnoth.set_variable(string.format("%s[%d]", var, i), unit_type.__cfg)
		i = i + 1
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
		for type in string.gmatch(unit_types, "[^%s,][^,]*") do
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
		for recruit in string.gmatch(recruits, "[^%s,][^,]*") do
			table.insert(v, recruit)
			wesnoth.add_known_unit(recruit)
		end
		unit.extra_recruit = v
	end
end

function wml_actions.disallow_recruit(cfg)
	local unit_types = cfg.type or helper.wml_error("[disallow_recruit] missing required type= attribute")
	for index, team in ipairs(wesnoth.get_sides(cfg)) do
		local v = team.recruit
		for w in string.gmatch(unit_types, "[^%s,][^,]*") do
			for i, r in ipairs(v) do
				if r == w then
					table.remove(v, i)
					break
				end
			end
		end
		team.recruit = v
	end
end

function wml_actions.disallow_extra_recruit(cfg)
	local recruits = cfg.extra_recruit or helper.wml_error("[disallow_extra_recruit] missing required extra_recruit= attribute")
	for index, unit in ipairs(wesnoth.get_units(cfg)) do
		local v = unit.extra_recruit
		for w in string.gmatch(recruits, "[^%s,][^,]*") do
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
		for w in string.gmatch(recruit, "[^%s,][^,]*") do
			table.insert(v, w)
		end
		team.recruit = v
	end
end

function wml_actions.set_extra_recruit(cfg)
	local recruits = cfg.extra_recruit or helper.wml_error("[set_extra_recruit] missing required extra_recruit= attribute")
	local v = {}

	for w in string.gmatch(recruits, "[^%s,][^,]*") do
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
	local recall_cost = unit.recall_cost
	local best_adv = ut.cost
	for w in string.gmatch(ut.__cfg.advances_to, "[^%s,][^,]*") do
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

local function handle_event_commands(cfg)
	-- The WML might be modifying the currently executed WML by mixing
	-- [insert_tag] with [set_variables] and [clear_variable], so we
	-- have to be careful not to get confused by tags vanishing during
	-- the execution, hence the manual handling of [insert_tag].
	local cmds = helper.shallow_literal(cfg)
	for i = 1,#cmds do
		local v = cmds[i]
		local cmd = v[1]
		local arg = v[2]
		local insert_from
		if cmd == "insert_tag" then
			cmd = arg.name
			local from = arg.variable
			arg = wesnoth.get_variable(from)
			if type(arg) ~= "table" then
				-- Corner case: A missing variable is replaced
				-- by an empty container rather than being ignored.
				arg = {}
			elseif string.sub(from, -1) ~= ']' then
				insert_from = from
			end
			arg = wesnoth.tovconfig(arg)
		end
		if not string.find(cmd, "^filter") then
			cmd = wml_actions[cmd] or
				helper.wml_error(string.format("[%s] not supported", cmd))
			if insert_from then
				local j = 0
				repeat
					cmd(arg)
					j = j + 1
					if j >= wesnoth.get_variable(insert_from .. ".length") then break end
					arg = wesnoth.tovconfig(wesnoth.get_variable(string.format("%s[%d]", insert_from, j)))
				until false
			else
				cmd(arg)
			end
		end
	end
	-- Apply music alterations once all the commands have been processed.
	wesnoth.set_music()
end

wml_actions.command = handle_event_commands

-- since if and while are Lua keywords, we can't create functions with such names
-- instead, we store the following anonymous functions directly into
-- the table, using the [] operator, rather than by using the point syntax

wml_actions["if"] = function( cfg )
	-- raise error if [then] is missing
	--if not helper.get_child( cfg, "then" ) then
	--	helper.wml_error "[if] missing required [then] tag"
	--end

	if wesnoth.eval_conditional( cfg ) then -- evalutate [if] tag
		for then_child in helper.child_range ( cfg, "then" ) do
			handle_event_commands( then_child )
		end
		return -- stop after executing [then] tags
	else
		for elseif_child in helper.child_range ( cfg, "elseif" ) do
			-- there's no point in executing [elseif] without [then]
			--if not helper.get_child( elseif_child, "then" ) then
			--	helper.wml_error "[elseif] missing required [then] tag"
			--end
			if wesnoth.eval_conditional( elseif_child ) then -- we'll evalutate the [elseif] tags one by one
				for then_tag in helper.child_range( elseif_child, "then" ) do
					handle_event_commands( then_tag )
				end
				return -- stop on first matched condition
			end
		end
		-- no matched condition, try the [else] tags
		for else_child in helper.child_range ( cfg, "else" ) do
			handle_event_commands( else_child )
		end
	end
end

wml_actions["while"] = function( cfg )
	-- check if the [do] sub-tag is missing, and raise error if so
	if not helper.get_child( cfg, "do" ) then
		helper.wml_error "[while] missing required [do] tag"
	end
	-- we have at least one [do], execute them up to 65536 times
	for i = 1, 65536 do
		if wesnoth.eval_conditional( cfg ) then
			for do_child in helper.child_range( cfg, "do" ) do
				handle_event_commands( do_child )
			end
		else return end
	end
end

function wml_actions.switch(cfg)
	-- check if variable= is missing
	if not cfg.variable then
		helper.wml_error "[switch] missing required variable= attribute"
	end 
	local variable = wesnoth.get_variable(cfg.variable)
	local found = false

	-- check if the [case] sub-tag is missing, and raise error if so
	if not helper.get_child( cfg, "case" ) then
		helper.wml_error "[switch] missing required [case] tag"
	end

	-- Execute all the [case]s where the value matches.
	for case_child in helper.child_range(cfg, "case") do
		-- warn if value= isn't present; it may be false, so check only for nil
		if case_child.value == nil then
			helper.wml_error "[case] missing required value= attribute"
		end
		local match = false
		for w in string.gmatch(case_child.value, "[^%s,][^,]*") do
			if w == tostring(variable) then match = true ; break end
		end
		if match then
			handle_event_commands(case_child)
			found = true
		end
	end
	-- Otherwise execute [else] statements.
	if not found then
		for else_child in helper.child_range(cfg, "else") do
			handle_event_commands(else_child)
		end
	end
end

function wml_actions.scroll_to(cfg)
	local loc = wesnoth.get_locations( cfg )[1]
	if not loc then return end
	if not optional_side_filter(cfg) then return end
	wesnoth.scroll_to_tile(loc[1], loc[2], cfg.check_fogged, cfg.immediate)
end

function wml_actions.scroll_to_unit(cfg)
	local u = wesnoth.get_units(cfg)[1]
	if not u then return end
	if not optional_side_filter(cfg, "for_side", "for_side") then return end
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
		for w in string.gmatch(ucfg.overlays, "[^%s,][^,]*") do
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

	-- Splits the string argument on commas, excepting those commas that occur
	-- within paired parentheses. The result is returned as a (non-empty) table.
	-- (The table might have a single entry that is an empty string, though.)
	-- Spaces around splitting commas are stripped (as in the C++ version).
	-- Empty strings are not removed (unlike the C++ version).
	local function parenthetical_split(str)
		local t = {""}
		-- To simplify some logic, end the string with paired parentheses.
		local formatted = (str or "") .. ",()"

		-- Isolate paired parentheses.
		for prefix,paren in string.gmatch(formatted, "(.-)(%b())") do
			-- Separate on commas
			for comma,text in string.gmatch(prefix, "(,?)([^,]*)") do
				if comma == "" then
					-- We are continuing the last string found.
					t[#t] = t[#t] .. text
				else
					-- We are starting the next string.
					-- (Now that we know the last string is complete,
					-- strip leading and trailing spaces from it.)
					t[#t] = string.match(t[#t], "^%s*(.-)%s*$")
					table.insert(t, text)
				end
			end
			-- Add the parenthetical part to the last string found.
			t[#t] = t[#t] .. paren
		end
		-- Remove the empty parentheses we had added to the end.
		table.remove(t)
		return t
	end

	-- Loop through all matching units.
	for i,u in ipairs(wesnoth.get_units(cfg)) do
		local ucfg = u.__cfg
		local t = parenthetical_split(ucfg.overlays)
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
	local mode = cfg.mode

	local var = cfg.variable or "unit"
	local idx = 0
	--cache the needed units here, since the filter might reference the to-be-cleared variable(s)
	local units = wesnoth.get_units(filter)
	local recall_units = wesnoth.get_recall_units(filter)
	if mode == "append" then
		idx = wesnoth.get_variable(var .. ".length")
	elseif mode ~= "replace" then
		wesnoth.set_variable(var)
	end

	for i,u in ipairs(units) do
		wesnoth.set_variable(string.format("%s[%d]", var, idx), u.__cfg)
		idx = idx + 1
		if kill_units then wesnoth.put_unit(u.x, u.y) end
	end

	if (not filter.x or filter.x == "recall") and (not filter.y or filter.y == "recall") then
		for i,u in ipairs(recall_units) do
			local ucfg = u.__cfg
			ucfg.x = "recall"
			ucfg.y = "recall"
			wesnoth.set_variable(string.format("%s[%d]", var, idx), ucfg)
			idx = idx + 1
			if kill_units then wesnoth.extract_unit(u) end
		end
	end
end

function wml_actions.sound(cfg)
	local name = cfg.name or helper.wml_error("[sound] missing required name= attribute")
	wesnoth.play_sound(name, cfg["repeat"])
end

function wml_actions.store_locations(cfg)
	local var = cfg.variable or "location"
	-- the variable can be mentioned in a [find_in] subtag, so it
	-- cannot be cleared before the locations are recovered
	local locs = wesnoth.get_locations(cfg)
	wesnoth.set_variable(var)
	for i, loc in ipairs(locs) do
		local x, y = loc[1], loc[2]
		local t = wesnoth.get_terrain(x, y)
		local res = { x = x, y = y, terrain = t }
		if wesnoth.get_terrain_info(t).village then
			res.owner_side = wesnoth.get_village_owner(x, y) or 0
		end
		wesnoth.set_variable(string.format("%s[%d]", var, i - 1), res)
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

--note: when using these, make sure that nothing can throw over the call to end_var_scope
local function start_var_scope(name)
	local var = helper.get_variable_array(name) --containers and arrays
	if #var == 0 then var = wesnoth.get_variable(name) end --scalars (and nil/empty)
	wesnoth.set_variable(name)
	return var
end

local function end_var_scope(name, var)
	wesnoth.set_variable(name)
	if type(var) == "table" then
		helper.set_variable_array(name, var)
	else
		wesnoth.set_variable(name, var)
	end
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
			elseif current_tag == "object" or current_tag == "trait" then
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

	local this_unit = start_var_scope("this_unit")
	for current_unit = 0, max_index do
		handle_unit(current_unit)
	end
	end_var_scope("this_unit", this_unit)

	wesnoth.set_variable(unit_variable)
end

function wml_actions.move_unit(cfg)
	local coordinate_error = "invalid coordinate in [move_unit]"
	local to_x = tostring(cfg.to_x) or helper.wml_error(coordinate_error)
	local to_y = tostring(cfg.to_y) or helper.wml_error(coordinate_error)
	local fire_event = cfg.fire_event
	local muf_force_scroll = cfg.force_scroll
	local check_passability = cfg.check_passability; if check_passability == nil then check_passability = true end
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
			while true do
				x = tonumber(x) or helper.wml_error(coordinate_error)
				y = tonumber(y) or helper.wml_error(coordinate_error)
				x, y = wesnoth.find_vacant_tile(x, y, pass_check)
				if not x or not y then helper.wml_error("Could not find a suitable hex near to one of the target hexes in [move_unit].") end
				move_string_x = string.format("%s,%u", move_string_x, x)
				move_string_y = string.format("%s,%u", move_string_y, y)
				local next_x, next_y = xs(), ys()
				if not next_x and not next_y then break end
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
	cfg = helper.shallow_literal(cfg)
	cfg.terrain = nil
	for i, loc in ipairs(wesnoth.get_locations(cfg)) do
		wesnoth.set_terrain(loc[1], loc[2], terrain, cfg.layer, cfg.replace_if_failed)
	end
end

function wml_actions.delay(cfg)
	local delay = tonumber(cfg.time) or
		helper.wml_error "[delay] missing required time= attribute."
	wesnoth.delay(delay)
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

	local this_unit = start_var_scope("this_unit")

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
					wml_actions.animate_unit( { flag = "attack", hits = true, { "filter", { id = harmer.id } },
						{ "primary_attack", primary_attack },
						{ "secondary_attack", secondary_attack }, with_bars = true,
						{ "facing", { x = unit_to_harm.x, y = unit_to_harm.y } } } )
				end
				wesnoth.scroll_to_tile(unit_to_harm.x, unit_to_harm.y, true)
			end

			-- the two functions below are taken straight from the C++ engine, util.cpp and actions.cpp, with a few unuseful parts removed
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
					wml_actions.animate_unit( { flag = "defend", hits = true, { "filter", { id = unit_to_harm.id } },
						{ "primary_attack", primary_attack },
						{ "secondary_attack", secondary_attack }, with_bars = true },
						{ "facing", { x = harmer.x, y = harmer.y } } )
				else
					wml_actions.animate_unit( { flag = "defend", hits = true, { "filter", { id = unit_to_harm.id } },
						{ "primary_attack", primary_attack },
						{ "secondary_attack", secondary_attack }, with_bars = true } )
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
	end_var_scope("this_unit", this_unit)
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
			wml_actions.store_unit { { "filter", { id = unit.id } }, variable = "Lua_store_unit", kill = true }
			wml_actions.unstore_unit { variable = "Lua_store_unit", find_vacant = false, advance = true, fire_event = false, animate = false }
			wesnoth.set_variable ( "Lua_store_unit")

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
	local variable = cfg.variable or "side"
	local index = 0
	wesnoth.set_variable(variable)
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
		wesnoth.set_variable(string.format("%s[%u]", variable, index), container)
		index = index + 1
	end
end

function wml_actions.add_ai_behavior(cfg)
	local unit = wesnoth.get_units(helper.get_child(cfg, "filter"))[1]

	if not unit then
		helper.wml_error("[add_ai_behavior]: no unit specified")
	end

	local side = cfg.side
	local sticky = cfg.sticky or false
	local loop_id = cfg.loop_id or "main_loop"
	local eval = cfg.evaluation
	local exec = cfg.execution
	local id = cfg.bca_id or ("bca-" .. unit.__cfg.underlying_id)

	local ux = unit.x -- @note: did I get it right that coordinates in C++ differ by 1 from thos in-game(and in Lua)?
	local uy = unit.y

	if not side then
		helper.wml_error("[add_ai_behavior]: no side attribute given")
	end

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
	if not helper.get_child(cfg, "destination") then helper.wml_error( "[find_path] missing required [destination] tag" ) end
	-- support for $this_unit
	local this_unit = start_var_scope("this_unit")
	wesnoth.set_variable ( "this_unit" ) -- clearing this_unit
	wesnoth.set_variable("this_unit", unit.__cfg) -- cfg field needed

	local filter_location = (helper.get_child(cfg, "destination")) or helper.wml_error("[find_path] missing required [destination] tag")
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
	if not allow_multiple_turns then local max_cost = unit.moves end --to avoid wrong calculation on already moved units
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
			end_var_scope("this_unit", this_unit)
		return end

		if not allow_multiple_turns and turns > 1 then -- location cannot be reached in one turn
			wesnoth.set_variable ( string.format("%s", variable), { hexes = 0 } )
			-- support for $this_unit
			wesnoth.set_variable ( "this_unit" ) -- clearing this_unit
			end_var_scope("this_unit", this_unit)
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
	end_var_scope("this_unit", this_unit)
end

function wml_actions.store_starting_location(cfg)
	local variable = cfg.variable or "location"
	wesnoth.set_variable(variable)
	local index = 0
	for possibly_wrong_index, side in ipairs(wesnoth.get_sides(cfg)) do
		local loc = wesnoth.get_starting_location(side.side)
		if loc then
			local terrain = wesnoth.get_terrain(loc[1], loc[2])
			local result = { x = loc[1], y = loc[2], terrain = terrain }
			if wesnoth.get_terrain_info(terrain).village then
				result.owner_side = wesnoth.get_village_owner(loc[1], loc[2]) or 0
			end
			wesnoth.set_variable(string.format("%s[%u]", variable, index), result)
			index = index + 1
		end
	end
end

function wml_actions.store_villages( cfg )
	local villages = wesnoth.get_villages( cfg )
	local variable = cfg.variable or "location"

	for index, village in ipairs( villages ) do
		wesnoth.set_variable( string.format( "%s[%d]", variable, index-1 ),
				      { x = village[1],
				        y = village[2],
				        terrain = wesnoth.get_terrain( village[1], village[2] ),
				        owner_side = wesnoth.get_village_owner( village[1], village[2] ) or 0
				      }
				    )
	end
end
