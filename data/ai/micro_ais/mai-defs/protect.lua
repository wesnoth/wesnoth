local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}
local MAIH = wesnoth.require("ai/micro_ais/micro_ai_helper.lua")

function wesnoth.micro_ais.protect_unit(cfg)
	local required_keys = { "id", "goal_x", "goal_y" }
	-- Scores for this AI need to be hard-coded, it does not work otherwise
	local CA_parms = {
		ai_id = 'mai_protect_unit',
		{ ca_id = 'finish', location = 'ca_protect_unit_finish.lua',  score = 300000 },
		{ ca_id = 'attack', location = 'ca_protect_unit_attack.lua', score = 95000 },
		{ ca_id = 'move', location = 'ca_protect_unit_move.lua', score = 94999 }
	}

	-- [unit] tags need to be dealt with separately
	cfg.id, cfg.goal_x, cfg.goal_y = {}, {}, {}
	if (cfg.action ~= 'delete') then
		for unit in H.child_range(cfg, "unit") do
			if (not unit.id) then
				H.wml_error("Protect Unit Micro AI [unit] tag is missing required id= key")
			end
			if (not unit.goal_x) then
				H.wml_error("Protect Unit Micro AI [unit] tag is missing required goal_x= key")
			end
			if (not unit.goal_y) then
				H.wml_error("Protect Unit Micro AI [unit] tag is missing required goal_y= key")
			end
			table.insert(cfg.id, unit.id)
			table.insert(cfg.goal_x, unit.goal_x)
			table.insert(cfg.goal_y, unit.goal_y)
		end

		if (not cfg.id[1]) then
			H.wml_error("Protect Unit Micro AI is missing required [unit] tag")
		end
	end

	-- Optional key disable_move_leader_to_keep: needs to be dealt with
	-- separately as it affects a default CA
	if cfg.disable_move_leader_to_keep then
		W.modify_ai {
			side = cfg.side,
			action = "try_delete",
			path = "stage[main_loop].candidate_action[move_leader_to_keep]"
		}
	end

	-- attacks aspects also needs to be set separately
	local unit_ids_str = 'dummy'
	for _,id in ipairs(cfg.id) do
		unit_ids_str = unit_ids_str .. ',' .. id
	end
	local aspect_parms = {
		{
			aspect = "attacks",
			facet = {
				name = "ai_default_rca::aspect_attacks",
				ca_id = "dont_attack",
				invalidate_on_gamestate_change = "yes",
				{ "filter_own", {
					{ "not", {
						id = unit_ids_str
					} }
				} }
			}
		}
	}

	if (cfg.action == "delete") then
		MAIH.delete_aspects(cfg.side, aspect_parms)
		-- We also need to add the move_leader_to_keep CA back in
		-- This works even if it was not removed, it simply overwrites the existing CA
		W.modify_ai {
			side = side,
			action = "add",
			path = "stage[main_loop].candidate_action",
			{ "candidate_action", {
				id="move_leader_to_keep",
				engine="cpp",
				name="ai_default_rca::move_leader_to_keep_phase",
				max_score=160000,
				score=160000
			} }
		}
	else
		MAIH.add_aspects(cfg.side, aspect_parms)
	end
    return required_keys, {}, CA_parms
end
