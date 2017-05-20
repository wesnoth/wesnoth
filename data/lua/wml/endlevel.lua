local helper = wesnoth.require "helper"
local utils = wesnoth.require "wml-utils"
local already_ended = false

function wesnoth.wml_actions.endlevel(cfg)
	local parsed = helper.parsed(cfg)
	if already_ended then
		wesnoth.message("Repeated [endlevel] execution, ignoring")
		return
	end
	already_ended = true

	local next_scenario = cfg.next_scenario
	if next_scenario then
		wesnoth.game_config.next_scenario = next_scenario
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
	
	local side_results = {}
	for result in helper.child_range(parsed, "result") do
		local side = result.side or helper.wml_error("[result] in [endlevel] missing required side= key")
		side_results[side] = result
	end
	local there_is_a_human_victory = false
	local there_is_a_human_defeat = false
	local there_is_a_local_human_victory = false
	local there_is_a_local_human_defeat = false
	local bool_int = function(b)
		if b == true then
			return 1
		elseif b == false then
			return 0
		else
			return b
		end
	end
	for k,v in ipairs(wesnoth.sides) do
		local side_result = side_results[v.side] or {}
		local victory_or_defeat = side_result.result or cfg.result or "victory"
		local victory = victory_or_defeat == "victory"
		if victory_or_defeat ~= "victory" and victory_or_defeat ~= "defeat" then
			return helper.wml_error("invalid result= key in [endlevel] '" .. victory_or_defeat .."'")
		end
		if v.controller == "human" or v.controller == "network" then
			if victory then
				there_is_a_human_victory = true
			else
				there_is_a_human_defeat = true
			end
		end
		if v.controller == "human" then
			if victory then
				there_is_a_local_human_victory = true
			else
				there_is_a_local_human_defeat = true
			end
		end
		if side_result.bonus ~= nil then
			v.carryover_bonus = bool_int(side_result.bonus)
		elseif cfg.bonus ~= nil then
			v.carryover_bonus = bool_int(cfg.bonus)
		end
		if side_result.carryover_add ~= nil then
			v.carryover_add = side_result.carryover_add
		elseif cfg.carryover_add ~= nil then
			v.carryover_add = cfg.carryover_add
		end
		if side_result.carryover_percentage ~= nil then
			v.carryover_percentage = side_result.carryover_percentage
		elseif cfg.carryover_percentage ~= nil then
			v.carryover_percentage = cfg.carryover_percentage
		end
	end

	local proceed_to_next_level = there_is_a_human_victory or (not there_is_a_human_defeat and cfg.result ~= "defeat") 
	local victory = there_is_a_local_human_victory or (not there_is_a_local_human_defeat and proceed_to_next_level)

	if cfg.music then
		local music = {}
		for track in utils.split(cfg.music) do
			table.insert(music, track)
		end
		if victory then
			wesnoth.game_config.victory_music = music
		else
			wesnoth.game_config.defeat_music = music
		end
	end

	wesnoth.end_level {
		carryover_report = cfg.carryover_report,
		save = cfg.save,
		replay_save = cfg.replay_save,
		linger_mode = cfg.linger_mode,
		reveal_map = cfg.reveal_map,
		proceed_to_next_level = proceed_to_next_level,
		result = victory and "victory" or "defeat",
	}
end
