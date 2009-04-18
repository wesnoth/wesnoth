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

local function wml_objectives(cfg)
	local _ = wesnoth.textdomain("wesnoth")
	local objectives = ""
	local win_objectives = ""
	local lose_objectives = ""

	local win_string = cfg.victory_string
	if not win_string then
		win_string = _ "Victory:"
	end
	local lose_string = cfg.defeat_string
	if not lose_string then
		lose_string = _ "Defeat:"
	end

	for obj in child_range(cfg, "objective") do
		local condition = obj.condition
		if condition == "win" then
			win_objectives = win_objectives .. "\n@" .. obj.description
		elseif condition == "lose" then
			lose_objectives = lose_objectives .. "\n#" .. obj.description
		else
			wesnoth.message "Unknown condition, ignoring."
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

	local silent = cfg.silent
	local side = cfg.side or 0
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
end

wesnoth.register_wml_action("objectives", wml_objectives)
