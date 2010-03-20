--! #textdomain wesnoth

local helper = wesnoth.require "lua/helper.lua"

local function trim(s)
	local r = string.gsub(s, "^%s*(.-)%s*$", "%1")
	return r
end

local function color_prefix(r, g, b)
	return string.format('<span foreground="#%02x%02x%02x">', r, g, b)
end

local function insert_before_nl(s, t)
	return string.gsub(tostring(s), "[^\n]*", "%0" .. t, 1)
end

local function generate_objectives(cfg)
	local _ = wesnoth.textdomain("wesnoth")
	local objectives = ""
	local win_objectives = ""
	local lose_objectives = ""

	local win_string = cfg.victory_string or _ "Victory:"
	local lose_string = cfg.defeat_string or _ "Defeat:"

	for obj in helper.child_range(cfg, "objective") do
		local show_if = helper.get_child(obj, "show_if")
		if not show_if or wesnoth.eval_conditional(show_if) then
			local condition = obj.condition
			if condition == "win" then
				win_objectives = win_objectives .. color_prefix(0, 255, 0) ..
					insert_before_nl(obj.description, "</span>") .. "\n"
			elseif condition == "lose" then
				lose_objectives = lose_objectives .. color_prefix(255, 0 ,0) ..
					insert_before_nl(obj.description, "</span>") .. "\n"
			else
				wesnoth.message "Unknown condition, ignoring."
			end
		end
	end

	local summary = cfg.summary
	if summary then
		objectives = "<big>" .. insert_before_nl(summary, "</big>") .. "\n"
	end
	if win_objectives ~= "" then
		objectives = objectives .. "<big>" .. win_string .. "</big>\n" .. win_objectives
	end
	if lose_objectives ~= "" then
		objectives = objectives .. "<big>" .. lose_string .. "</big>\n" .. lose_objectives
	end
	local note = cfg.note
	if note then
		objectives = objectives .. note .. "\n"
	end

	return string.sub(tostring(objectives), 1, -2)
end

local function wml_objectives(cfg)
	cfg = cfg.__parsed
	local side = cfg.side or 0
	local silent = cfg.silent

	-- Save the objectives in a WML variable in case they have to be regenerated later.
	cfg.side = nil
	cfg.silent = nil
	wesnoth.set_variable("__scenario_objectives_" .. side, cfg)

	-- Generate objectives for the given sides
	local objectives = generate_objectives(cfg)
	if side == 0 then
		for team in helper.all_teams() do
			team.objectives = objectives
			if not silent then team.objectives_changed = true end
		end
	else
		local team = wesnoth.get_side(side)
		team.objectives = objectives
		if not silent then team.objectives_changed = true end
	end

	-- Prepare an event for removing objectives variables on output.
	if not wesnoth.get_variable("__scenario_objectives_gc") then
		wesnoth.set_variable("__scenario_objectives_gc", true)
		local vars = "__scenario_objectives_gc,__scenario_objectives_0"
		local side = 1
		for team in helper.all_teams() do
			vars = vars .. ",__scenario_objectives_" .. side
			side = side + 1
		end
		wesnoth.fire("event", { name="victory", { "clear_variable", { name = vars }}})
	end
end

local function wml_show_objectives(cfg)
	local side = cfg.side or 0
	local cfg0 = wesnoth.get_variable("__scenario_objectives_0")
	if side == 0 then
		local objectives0 = cfg0 and generate_objectives(cfg0)
		for team in helper.all_teams() do
			side = side + 1
			cfg = wesnoth.get_variable("__scenario_objectives_" .. side)
			local objectives = (cfg and generate_objectives(cfg)) or objectives0
			if objectives then team.objectives = objectives end
			team.objectives_changed = true
		end
	else
		local team = wesnoth.get_side(side)
		cfg = wesnoth.get_variable("__scenario_objectives_" .. side) or cfg0
		local objectives = cfg and generate_objectives(cfg)
		if objectives then team.objectives = objectives end
		team.objectives_changed = true
	end
end

local engine_message

local function wml_message(cfg)
	local show_if = helper.get_child(cfg, "show_if")
	if not show_if or wesnoth.eval_conditional(show_if) then
		engine_message(cfg)
	end
end

local function wml_gold(cfg)
	local side = tonumber(cfg.side or 1) or
		helper.wml_error("[gold] given a noninteger side= attribute.")
	local team = wesnoth.get_side(side)
	local amount = tonumber(cfg.amount) or
		helper.wml_error("[gold] missing required amount= attribute.")
	team.gold = team.gold + amount
end

local function wml_store_gold(cfg)
	local side = tonumber(cfg.side or 1) or
		helper.wml_error("[store_gold] given a noninteger side= attribute.")
	local team = wesnoth.get_side(side)
	wesnoth.set_variable(cfg.variable or "gold", team.gold)
end

local function wml_clear_variable(cfg)
	local names = cfg.name or
		helper.wml_error("[clear_variable] missing required name= attribute.")
	for w in string.gmatch(names, "[^%s,][^,]*") do
		wesnoth.set_variable(trim(w))
	end
end

local function wml_store_unit_type_ids(cfg)
	local types = table.concat(wesnoth.get_unit_type_ids(), ',')
	wesnoth.set_variable(cfg.variable or "unit_type_ids", types)
end

local function wml_store_unit_type(cfg)
	local var = cfg.variable or "unit_type"
	local types = cfg.type or
		helper.wml_error("[store_unit_type] missing required type= attribute.")
	wesnoth.set_variable(var)
	local i = 0
	for w in string.gmatch(types, "[^%s,][^,]*") do
		local unit_type = wesnoth.get_unit_type(w) or
			helper.wml_error("Attempt to store nonexistent unit type '" .. w .. "'.")
		wesnoth.set_variable(var .. '[' .. i .. ']', unit_type.__cfg)
		i = i + 1
	end
end

local function wml_action_tag(cfg)
	-- The new tag's name
	local name = cfg.name or
		helper.wml_error("[wml_action] missing required name= attribute.")
	local code = cfg.lua_function or
		helper.wml_error("[wml_action] missing required lua_function= attribute.")
	local bytecode, message = loadstring(code)
	if not bytecode then
		helper.wml_error("[wml_action] failed to compile Lua code: " .. message)
	end
	-- The lua function that is executed when the tag is called
	local lua_function = bytecode() or
		helper.wml_error("[wml_action] expects a Lua code returning a function.")
	wesnoth.register_wml_action(name, lua_function)
end

wesnoth.register_wml_action("objectives", wml_objectives)
wesnoth.register_wml_action("show_objectives", wml_show_objectives)
engine_message = wesnoth.register_wml_action("message", wml_message)
wesnoth.register_wml_action("gold", wml_gold)
wesnoth.register_wml_action("store_gold", wml_store_gold)
wesnoth.register_wml_action("clear_variable", wml_clear_variable)
wesnoth.register_wml_action("store_unit_type", wml_store_unit_type)
wesnoth.register_wml_action("store_unit_type_ids", wml_store_unit_type_ids)
wesnoth.register_wml_action("wml_action", wml_action_tag)
