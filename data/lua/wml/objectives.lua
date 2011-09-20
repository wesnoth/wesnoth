local helper = wesnoth.require "lua/helper.lua"
local wml_actions = wesnoth.wml_actions
local game_events = wesnoth.game_events

local function color_prefix(r, g, b)
	return string.format('<span foreground="#%02x%02x%02x">', r, g, b)
end

local function insert_before_nl(s, t)
	return string.gsub(tostring(s), "[^\n]*", "%0" .. tostring(t), 1)
end

local scenario_objectives = {}

local old_on_save = game_events.on_save
function game_events.on_save()
	local custom_cfg = old_on_save()
	for i,v in pairs(scenario_objectives) do
		v.side = i
		table.insert(custom_cfg, { "objectives", v })
	end
	return custom_cfg
end

local old_on_load = game_events.on_load
function game_events.on_load(cfg)
	for i = #cfg,1,-1 do
		local v = cfg[i]
		if helper.check_equal( v[1], "objectives" ) then
			local v2 = v[2]
			scenario_objectives[v2.side or 0] = v2
			table.remove(cfg, i)
		end
	end
	old_on_load(cfg)
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

	local win_string = tostring(cfg.victory_string or _ "Victory:")
	local lose_string = tostring(cfg.defeat_string or _ "Defeat:")
	local gold_carryover_string = tostring(cfg.gold_carryover_string or _ "Gold carryover:")
	local notes_string = tostring(cfg.notes_string or _ "Notes:")

	local bullet = "&#8226; "

	for obj in helper.child_range(cfg, "objective") do
		local show_if = helper.get_child(obj, "show_if")
		if not show_if or wesnoth.eval_conditional(show_if) then
			local condition = tostring(obj.condition)
			local description = tostring(obj.description or "")
			local turn_counter = ""

			if obj.show_turn_counter then
				local current_turn = wesnoth.current.turn
				local turn_limit = wesnoth.game_config.last_turn

				if turn_limit >= current_turn then
					if turn_limit - current_turn + 1 > 1 then
						turn_counter = "<small> " .. string.format(tostring(_"(%d turns left)"), turn_limit - current_turn + 1) .. "</small>"
					else
						turn_counter = "<small> " .. _"(this turn left)" .. "</small>"
					end
				end
			end

			if helper.check_equal( condition, "win" ) then
				local caption = obj.caption

				if caption then
					win_objectives = tostring(win_objectives) .. tostring(caption) .. "\n"
				end

				win_objectives = tostring(win_objectives) .. tostring(color_prefix(0, 255, 0)) .. tostring(bullet) .. tostring(description) .. tostring(turn_counter) .. "</span>" .. "\n"
			elseif helper.check_equal( condition, "lose" ) then
				local caption = obj.caption

				if caption then
					lose_objectives = tostring(lose_objectives) .. tostring(caption) .. "\n"
				end

				lose_objectives = tostring(lose_objectives) .. tostring(color_prefix(255, 0, 0)) .. tostring(bullet) .. tostring(description) .. tostring(turn_counter) .. "</span>" .. "\n"
			else
				wesnoth.message "Unknown condition, ignoring."
			end
		end
	end

	for obj in helper.child_range(cfg, "gold_carryover") do
		if obj.bonus ~= nil then
			if obj.bonus then
				gold_carryover = tostring(color_prefix(255, 255, 192)) .. tostring(bullet) .. "<small>" .. _"Early finish bonus." .. "</small></span>\n"
			else
				gold_carryover = tostring(color_prefix(255, 255, 192)) .. tostring(bullet) .. "<small>" .. _"No early finish bonus." .. "</small></span>\n"
			end
		end

		if obj.carryover_percentage then
			local carryover_amount_string = ""

			if tonumber(tostring(obj.carryover_percentage or 0)) == 0 then
				carryover_amount_string = _"No gold carried over to the next scenario."
			else
				carryover_amount_string = string.format(tostring(_ "%d%% of gold carried over to the next scenario."), tostring(obj.carryover_percentage))
			end

			gold_carryover = tostring(gold_carryover) .. tostring(color_prefix(255, 255, 192)) .. tostring(bullet) .. "<small>" .. tostring(carryover_amount_string) .. "</small></span>\n"
		end
	end

	for note in helper.child_range(cfg, "note") do
		if note.description then
			notes = tostring(notes) .. tostring(color_prefix(255, 255, 255)) .. tostring(bullet) .. "<small>" .. tostring(note.description) .. "</small></span>\n"
		end
	end

	local summary = cfg.summary
	if summary then
		objectives = "<big>" .. insert_before_nl(summary, "</big>") .. "\n"
	end
	if helper.check_not_equal( win_objectives, "" ) then
		objectives = tostring(objectives) .. "<big>" .. tostring(win_string) .. "</big>\n" .. tostring(win_objectives)
	end
	if helper.check_not_equal( lose_objectives, "" ) then
		objectives = tostring(objectives) .. "<big>" .. tostring(lose_string) .. "</big>\n" .. tostring(lose_objectives)
	end
	if helper.check_not_equal( gold_carryover, "" ) then
		objectives = tostring(objectives) .. tostring(gold_carryover_string) .. "\n" .. tostring(gold_carryover)
	end
	if helper.check_not_equal( notes, "" ) then
		objectives = tostring(objectives) .. tostring(notes_string) .. "\n" .. tostring(notes)
	end
	local note = cfg.note
	if note then
		objectives = tostring(objectives) .. tostring(note) .. "\n"
	end

	return string.sub(tostring(objectives), 1, -2)
end

function wml_actions.objectives(cfg)
	cfg = helper.parsed(cfg)
	local side = tonumber(tostring(cfg.side or 0)) or 0
	local silent = tostring(cfg.silent)

	-- Save the objectives in a WML variable in case they have to be regenerated later.
	cfg.side = nil
	cfg.silent = nil
	scenario_objectives[side] = cfg

	-- Generate objectives for the given sides
	local objectives = generate_objectives(cfg)
	if side == 0 then
		for side, team in ipairs(wesnoth.sides) do
			team.objectives = objectives
			if not silent then team.objectives_changed = true end
		end
	else
		local team = wesnoth.sides[side]
		team.objectives = objectives
		if not silent then team.objectives_changed = true end
	end
end

function wml_actions.show_objectives(cfg)
	local side = tonumber(tostring(cfg.side or 0)) or 0
	local cfg0 = scenario_objectives[0]
	if side == 0 then
		local objectives0 = cfg0 and generate_objectives(cfg0)
		for side, team in ipairs(wesnoth.sides) do
			cfg = scenario_objectives[side]
			local objectives = (cfg and generate_objectives(cfg)) or objectives0
			if objectives then team.objectives = objectives end
			team.objectives_changed = true
		end
	else
		local team = wesnoth.sides[side]
		cfg = scenario_objectives[side] or cfg0
		local objectives = cfg and generate_objectives(cfg)
		if objectives then team.objectives = objectives end
		team.objectives_changed = true
	end
end
