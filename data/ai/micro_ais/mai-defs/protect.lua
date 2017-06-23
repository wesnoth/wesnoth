local H = wesnoth.require "helper"
local MAIH = wesnoth.require("ai/micro_ais/micro_ai_helper.lua")

function wesnoth.micro_ais.protect_unit(cfg)
	-- Scores for this AI need to be hard-coded, it does not work otherwise
	local CA_parms = {
		ai_id = 'mai_protect_unit',
		{ ca_id = 'finish', location = 'ca_protect_unit_finish.lua',  score = 300000 },
		{ ca_id = 'attack', location = 'ca_protect_unit_attack.lua', score = 95000 },
		{ ca_id = 'move', location = 'ca_protect_unit_move.lua', score = 94999 }
	}

	local unit_ids = {}
	for u in H.child_range(cfg, "unit") do
		if not u.id then
			H.wml_error("Protect Unit Micro AI missing id key in [unit] tag")
		end
		if not u.goal_x then
			H.wml_error("Protect Unit Micro AI missing goal_x key in [unit] tag")
		end
		if not u.goal_y then
			H.wml_error("Protect Unit Micro AI missing goal_y key in [unit] tag")
		end
		table.insert(unit_ids, u.id)
	end

	-- Optional key disable_move_leader_to_keep: needs to be dealt with
	-- separately as it affects a default CA
	if cfg.disable_move_leader_to_keep then
		wesnoth.delete_ai_component(cfg.side, "stage[main_loop].candidate_action[move_leader_to_keep]")
	end

	-- attacks aspects also needs to be set separately
	local aspect_parms = {
		{
			aspect = "attacks",
			facet = {
				name = "ai_default_rca::aspect_attacks",
				id = "mai_protect_" .. (cfg.ca_id or "default") .. "_dont_attack",
				invalidate_on_gamestate_change = "yes",
				{ "filter_own", {
					{ "not", {
						id = unit_ids
					} }
				} }
			}
		}
	}

	if (cfg.action == "delete") then
		MAIH.delete_aspects(cfg.side, aspect_parms)
		-- We also need to add the move_leader_to_keep CA back in
		-- This works even if it was not removed, it simply overwrites the existing CA
		wesnoth.add_ai_component(side, "stage[main_loop].candidate_action",
			{
				id="move_leader_to_keep",
				engine="cpp",
				name="ai_default_rca::move_leader_to_keep_phase",
				max_score=160000,
				score=160000
			}
		)
	else
		MAIH.add_aspects(cfg.side, aspect_parms)
	end
    return {"[unit]"}, {}, CA_parms
end
