local function all_teams()
	local function f(s)
		local i = s.i
		local team = wesnoth.get_side(i)
		s.i = i + 1
		return team
	end
	return f, { i = 1 }
end

local function child_range(cfg, tag)
	local function f(s)
		local c
		repeat
			local i = s.i
			c = cfg[i]
			if not c then return end
			s.i = i + 1
		until c[1] == tag
		return c[2]
	end
	return f, { i = 1 }
end

local function trim(s)
	return string.gsub(s, "^%s*(.-)%s*$", "%1")
end

local function generate_objectives(cfg, team, silent)
	local _ = wesnoth.textdomain("wesnoth")
	local objectives = ""
	local win_objectives = ""
	local lose_objectives = ""

	local win_string = cfg.victory_string or _ "Victory:"
	local lose_string = cfg.defeat_string or _ "Defeat:"

	for obj in child_range(cfg, "objective") do
		-- Check if the display condition is fulfilled
		local show_if = obj[1]
		if show_if and show_if[1] == "show_if" then
			local test = show_if[2]
			table.insert(test, { "then", {{ "lua", { code = "wesnoth.dummy_var = true" }}}})
			wesnoth.dummy_var = nil
			wesnoth.fire("if", test)
			show_if = wesnoth.dummy_var
			wesnoth.dummy_var = nil
		else
			show_if = true
		end

		if show_if then
			local condition = obj.condition
			if condition == "win" then
				win_objectives = win_objectives .. "\n@" .. obj.description
			elseif condition == "lose" then
				lose_objectives = lose_objectives .. "\n#" .. obj.description
			else
				wesnoth.message "Unknown condition, ignoring."
			end
		end
	end

	local summary = cfg.summary
	if summary then
		objectives = "*" .. summary .. "\n"
	end
	if win_objectives ~= "" then
		objectives = objectives .. "*" .. win_string .. "\n" .. win_objectives .. "\n"
	end
	if lose_objectives ~= "" then
		objectives = objectives .. "*" .. lose_string .. "\n" .. lose_objectives .. "\n"
	end

	return objectives
end

local function wml_objectives(cfg)
	local side = cfg.side or 0
	local silent = cfg.silent

	-- Save the objectives in a WML variable in case they have to be regenerated later.
	cfg.side = nil
	cfg.silent = nil
	wesnoth.set_variable("__scenario_objectives_" .. side, cfg)

	-- Generate objectives for the given sides
	local objectives = generate_objectives(cfg)
	if side == 0 then
		for team in all_teams() do
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
		for team in all_teams() do
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
		for team in all_teams() do
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

local function wml_gold(cfg)
	local team = wesnoth.get_side(cfg.side)
	team.gold = team.gold + cfg.amount
end

local function wml_store_gold(cfg)
	local team = wesnoth.get_side(cfg.side)
	local var = cfg.variable
	if not var then var = "gold" end
	wesnoth.set_variable(var, team.gold)
end

local function wml_clear_variable(cfg)
	for w in string.gmatch(cfg.name, "[^%s,][^,]*") do
		wesnoth.set_variable(trim(w))
	end
end

local function wml_action_tag(cfg)
   -- The new tag's name 
   -- TODO make this attribute mandatory
   local name = cfg.name
   -- The lua function that is executed when the tag is called
   -- TODO make this attribute mandatory

   -- debug output
   for tag in child_range(cfg, "tag") do
      wesnoth.message(tag.name)
   end

   -- debug output
   for attribute in child_range(cfg, "attribute") do
      wesnoth.message(attribute.name)
   end

   local lua_function = assert(loadstring(cfg.lua_function)())
   wesnoth.register_wml_action(name, lua_function)
end

wesnoth.register_wml_action("objectives", wml_objectives)
wesnoth.register_wml_action("show_objectives", wml_show_objectives)
wesnoth.register_wml_action("gold", wml_gold)
wesnoth.register_wml_action("store_gold", wml_store_gold)
wesnoth.register_wml_action("clear_variable", wml_clear_variable)
wesnoth.register_wml_action("wml_action", wml_action_tag)

