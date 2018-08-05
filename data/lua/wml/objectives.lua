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

local _ = wesnoth.textdomain("wesnoth")
local objectives_meta = {
	-- Attempting to read an nonexistent value yields a new empty table.
	-- This is to enable syntax like "function wesnoth.wml_actions.objectives.new_category.generate(cfg)",
	-- without previously defining new_category.
	__index = function(self, key)
		rawset(self, key, {})
		return rawget(self, key)
	end
}

wml_actions.objectives = setmetatable({
	win = {priority = 1, default_title = _ "Victory:"},
	lose = {priority = 2, default_title = _ "Defeat:"},
	gold_carryover = {priority = 9000, max = 1, default_title = _ "Gold carryover:"},
	note = {priority = 9999, default_title = _ "Notes:"},
}, objectives_meta)

function wml_actions.objectives.win.generate(cfg, default_bullet)
	local objective_bullet = obj.bullet or default_bullet
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

	local caption = obj.caption
	local r = obj.red or 0
	local g = obj.green or 255
	local b = obj.blue or 0

	local objective = color_prefix(r, g, b) .. objective_bullet
		.. description .. turn_counter .. "</span>" .. "\n"

	if caption then
		objective = caption .. "\n" .. objective
	end
	
	return objective
end

function wml_actions.objectives.lose.generate(cfg, default_bullet)
	local objective_bullet = obj.bullet or default_bullet
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
	
	local caption = obj.caption
	local r = obj.red or 255
	local g = obj.green or 0
	local b = obj.blue or 0
	
	local objective =  color_prefix(r, g, b) .. objective_bullet.. description
		.. turn_counter .. "</span>" .. "\n"

	if caption then
		objective = caption .. "\n" .. objective
	end
	
	return objective
end

function wml_actions.objectives.gold_carryover.generate(cfg, default_bullet)
	local gold_carryover_bullet = obj.bullet or default_bullet
	local r = obj.red or 255
	local g = obj.green or 255
	local b = obj.blue or 192
	local gold_carryover = ""

	if obj.bonus ~= nil then
		if obj.bonus == true then
			gold_carryover = color_prefix(r, g, b) .. gold_carryover_bullet
				.. "<small>" .. _"Early finish bonus." .. "</small></span>\n"
		elseif obj.bonus == false then
			gold_carryover = color_prefix(r, g, b) .. gold_carryover_bullet
			.. "<small>" .. _"No early finish bonus." .. "</small></span>\n"
		elseif type(obj.bonus) == 'number' then
			local bonus_string = wesnoth.format(_"Early finish bonus &times;$multiple.", {multiple = obj.bonus})
			gold_carryover = color_prefix(r, g, b) .. gold_carryover_bullet
				.. "<small>" ..  bonus_string .. "</small></span>\n"
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
	
	if obj.carryover_add ~- nil then
		if obj.carryover_add then
			gold_carryover =  gold_carryover .. color_prefix(r, g, b) .. gold_carryover_bullet
				.. "<small>" .. _"Carryover gold will be added to the next scenario's starting gold." .. "</small></span>\n"
		else
			gold_carryover =  gold_carryover .. color_prefix(r, g, b) .. gold_carryover_bullet
				.. "<small>" .. _"Carryover gold will replace the next scenario's starting gold, if larger." .. "</small></span>\n"
		end
	end
	
	return gold_carryover
end

function wml_actions.note.generate(cfg, default_bullet)
	local note_bullet = note.bullet or default_bullet
	local r = note.red or 255
	local g = note.green or 255
	local b = note.blue or 255

	if note.description then
		return color_prefix(r, g, b) .. note_bullet .. "<small>" .. note.description .. "</small></span>\n"
	end
	
	return ""
end

local function generate_objectives(cfg)
	-- Note: when changing the text formatting, remember to check if you also
	-- need to change the hardcoded default multiplayer objective text in
	-- multiplayer_connect.cpp.

	local objectives = ""
	local conditions = {}
	local bullet = cfg.bullet or "&#8226; "
	local showed_deprecation = false
	
	for i = 1, #cfg do
		local tag, obj = cfg[i][1], cfg[i][2]
		local show_if = wml.get_child(obj, "show_if")
		if not show_if or wesnoth.eval_conditional(show_if) then
			if tag == "objective" then
				if not showed_deprecation then
					showed_deprecation = true
					deprecated_message("[objectives][objective]", 1, "1.17", "Use [win] or [lose] instead (without the condition= key)")
				end
				if obj.condition ~= "lose" and obj.condition ~= "win" then
					wesnoth.message "Unknown condition, ignoring."
					goto next
				end
				tag = obj.condition
			end
			if conditions[tag] == nil then
				conditions[tag] = {}
			end
			local max = wml_actions.objectives[tag].max
			if max ~= nil and #conditions[tag] >= max then
				wesnoth.message "Two many [" .. tag .. "], ignoring."
				goto next
			end
			table.insert(conditions[tag], wml_actions.objectives[tag].generate(obj, bullet))
		end
		::next::
	end
	
	local priority_ordered = {}
	for k,v in pairs(conditions) do
		table.insert(priority_ordered, k)
		local joined = ""
		local num_conditions, min = 0, wml_actions.objectives[k].min
		for i = 1, #v do
			joined = joined .. v[i]
			num_conditions = num_conditions + 1
		end
		if min ~= nil and num_conditions < min then
			wesnoth.message "Not enough [" .. k .. "]"
		end
		conditions[k] = joined
	end
	table.sort(priority_ordered, function(a,b)
		return wml_actions.objectives[a].priority < wml_actions.objectives[b].priority
	end)

	local summary = cfg.summary
	if summary then
		objectives = "<big>" .. insert_before_nl(summary, "</big>") .. "\n"
	end
	for i = 1, #priority_ordered do
		local tag = priority_ordered[i]
		local caption = wml_actions.objectives[tag].default_title
		if objectives ~= "" then objectives = objectives .. "\n" end
		objectives = objectives .. "<big>" .. caption .. "</big>\n" .. conditions[tag]
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

function objectives_meta:__call(cfg)
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
