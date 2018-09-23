local wml_actions = wesnoth.wml_actions

local function color_prefix(r, g, b)
	return string.format('<span foreground="#%02x%02x%02x">', r, g, b)
end

local function insert_before_nl(s, t)
	return string.gsub(tostring(s), "[^\n]*", "%0" .. t, 1)
end

local scenario_objectives = {}

function wesnoth.persistent_tags.objectives.write(add)
	for i,v in pairs(scenario_objectives) do
		v.side = i
		add(v)
	end
end

function wesnoth.persistent_tags.objectives.read(cfg)
	scenario_objectives[cfg.side or 0] = cfg
end

local function generate_objectives(cfg)
	-- Note: when changing the text formatting, remember to check if you also
	-- need to change the hardcoded default multiplayer objective text in
	-- multiplayer_connect.cpp.

	local _ = wesnoth.textdomain("wesnoth")
	local objectives = ""
	local win_objectives = ""
	local lose_objectives = ""
	local gold_carryover = ""
	local notes = ""

	local win_string = cfg.victory_string or _ "Victory:"
	local lose_string = cfg.defeat_string or _ "Defeat:"
	local gold_carryover_string = cfg.gold_carryover_string or _ "Gold carryover:"
	local notes_string = cfg.notes_string or _ "Notes:"

	local bullet = cfg.bullet or "&#8226; "

	for obj in wml.child_range(cfg, "objective") do
		local show_if = wml.get_child(obj, "show_if")
		if not show_if or wesnoth.eval_conditional(show_if) then
			local objective_bullet = obj.bullet or bullet
			local condition = obj.condition
			local description = obj.description or ""
			local turn_counter = ""

			if obj.show_turn_counter then
				local current_turn = wesnoth.current.turn
				local turn_limit = wesnoth.game_config.last_turn

				if turn_limit >= current_turn then
					local turn_count = turn_limit - current_turn + 1
					turn_counter = _("(this turn left)", "($remaining_turns turns left)", turn_count)
					turn_counter = wesnoth.format(turn_counter, {remaining_turns = turn_count})
					turn_counter = "<span foreground='white'><small> " .. turn_counter .. "</small></span>"
				end
			end

			if condition == "win" then
				local caption = obj.caption
				local r = obj.red or 0
				local g = obj.green or 255
				local b = obj.blue or 0

				if caption then
					win_objectives = win_objectives .. caption .. "\n"
				end

				win_objectives = win_objectives
					.. color_prefix(r, g, b) .. objective_bullet
					.. description .. turn_counter .. "</span>" .. "\n"
			elseif condition == "lose" then
				local caption = obj.caption
				local r = obj.red or 255
				local g = obj.green or 0
				local b = obj.blue or 0

				if caption then
					lose_objectives = lose_objectives .. caption .. "\n"
				end

				lose_objectives = lose_objectives
					.. color_prefix(r, g, b) .. objective_bullet.. description
					.. turn_counter .. "</span>" .. "\n"
			else
				wesnoth.message "Unknown condition, ignoring."
			end
		end
	end

	for obj in wml.child_range(cfg, "gold_carryover") do
		local show_if = wml.get_child(obj, "show_if")
		if not show_if or wesnoth.eval_conditional(show_if) then
			local gold_carryover_bullet = obj.bullet or bullet
			local r = obj.red or 255
			local g = obj.green or 255
			local b = obj.blue or 192

			if obj.bonus ~= nil then
				if obj.bonus then
					gold_carryover = color_prefix(r, g, b) .. gold_carryover_bullet
						.. "<small>" .. _"Early finish bonus." .. "</small></span>\n"
				else
					gold_carryover = color_prefix(r, g, b) .. gold_carryover_bullet
					.. "<small>" .. _"No early finish bonus." .. "</small></span>\n"
				end
			end

			if obj.carryover_percentage then
				local carryover_amount_string

				if obj.carryover_percentage == 0 then
					carryover_amount_string = _"No gold carried over to the next scenario."
				else
					carryover_amount_string = wesnoth.format(_ "$percent|% of gold carried over to the next scenario.", {percent = obj.carryover_percentage})
				end

				gold_carryover = gold_carryover
					.. color_prefix(r, g, b) .. gold_carryover_bullet
					.. "<small>" .. carryover_amount_string .. "</small></span>\n"
			end
		end
	end

	for note in wml.child_range(cfg, "note") do
		local show_if = wml.get_child(note, "show_if")
		if not show_if or wesnoth.eval_conditional(show_if) then
			local note_bullet = note.bullet or bullet
			local r = note.red or 255
			local g = note.green or 255
			local b = note.blue or 255

			if note.description then
				notes = notes .. color_prefix(r, g, b) .. note_bullet .. "<small>" .. note.description .. "</small></span>\n"
			end
		end
	end

	local summary = cfg.summary
	if summary then
		objectives = "<big>" .. insert_before_nl(summary, "</big>") .. "\n"
	end
	if win_objectives ~= "" then
		if objectives ~= "" then objectives = objectives .. "\n" end
		objectives = objectives .. "<big>" .. win_string .. "</big>\n" .. win_objectives
	end
	if lose_objectives ~= "" then
		if objectives ~= "" then objectives = objectives .. "\n" end
		objectives = objectives .. "<big>" .. lose_string .. "</big>\n" .. lose_objectives
	end
	if gold_carryover ~= "" then
		if objectives ~= "" then objectives = objectives .. "\n" end
		objectives = objectives .. gold_carryover_string .. "\n" .. gold_carryover
	end
	if notes ~= "" then
		if objectives ~= "" then objectives = objectives .. "\n" end
		objectives = objectives .. notes_string .. "\n" .. notes
	end
	local note = cfg.note
	if note then
		if objectives ~= "" then objectives = objectives .. "\n" end
		objectives = objectives .. note .. "\n"
	end

	return string.sub(tostring(objectives), 1, -2)
end

local function remove_ssf_info_from(cfg)
	cfg.side = nil
	cfg.team_name = nil
	cfg.silent = nil -- Not technically SSF info, but still unwanted
	for i, v in ipairs(cfg) do
		if v[1] == "has_unit" or v[1] == "enemy_of" or v[1] == "allied_with" then
			table.remove(cfg, i)
		end
	end
end

function wml_actions.objectives(cfg)
	if cfg.delayed_variable_substitution ~= true then
		cfg = wml.parsed(cfg)
	end

	local sides_cfg = wesnoth.get_sides(cfg)
	local silent = cfg.silent

	local objectives = generate_objectives(cfg)
	local function set_objectives(sides, save)
		local cfg2 = wml.literal(cfg)
		remove_ssf_info_from(cfg2)
		for i, team in ipairs(sides) do
			if save then scenario_objectives[team.side] = cfg2 end
			team.objectives = objectives
			team.objectives_changed = not silent
		end
	end
	if #sides_cfg == #wesnoth.sides or #sides_cfg == 0 then
		scenario_objectives[0] = wml.literal(cfg)
		remove_ssf_info_from(scenario_objectives[0])
		set_objectives(wesnoth.sides)
	else
		set_objectives(sides_cfg, true)
	end
end

local function maybe_parsed(cfg)
	if cfg == nil then return nil end
	if cfg.delayed_variable_substitution == true then
		return wml.tovconfig(cfg)
	end
	return cfg
end

function wml_actions.show_objectives(cfg)
	local cfg0 = maybe_parsed(scenario_objectives[0])
	local function local_show_objectives(sides)
		local objectives0 = cfg0 and generate_objectives(cfg0)
		for i, team in ipairs(sides) do
			cfg = maybe_parsed(scenario_objectives[team.side])
			local objectives = (cfg and generate_objectives(cfg)) or objectives0
			if objectives then team.objectives = objectives end
			team.objectives_changed = true
		end
	end
	local sides = wesnoth.get_sides(cfg)
	if #sides == 0 then
		local_show_objectives(wesnoth.sides)
	else
		local_show_objectives(sides)
	end
end
