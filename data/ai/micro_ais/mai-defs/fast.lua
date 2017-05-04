local H = wesnoth.require "helper"

function wesnoth.micro_ais.fast_ai(cfg)
	local optional_keys = {
		"attack_hidden_enemies", "[avoid]", "dungeon_mode",
		"[filter]", "[filter_second]", "include_occupied_attack_hexes",
		"leader_additional_threat", "leader_attack_max_units", "leader_weight", "move_cost_factor",
		"weak_units_first", "skip_combat_ca", "skip_move_ca", "threatened_leader_fights"
	}
	local CA_parms = {
		ai_id = 'mai_fast',
		{ ca_id = 'combat', location = 'ca_fast_combat.lua', score = 100000 },
		{ ca_id = 'move', location = 'ca_fast_move.lua', score = 20000 },
		{ ca_id = 'combat_leader', location = 'ca_fast_combat_leader.lua', score = 19900 }
	}

	-- Also need to delete/add some default CAs
	if (cfg.action == 'delete') then
		-- This can be done independently of whether these were removed earlier
		wesnoth.add_ai_component(cfg.side, "stage[main_loop].candidate_action",
			{
				id="combat",
				engine="cpp",
				name="ai_default_rca::combat_phase",
				max_score=100000,
				score=100000
			}
		)

		wesnoth.add_ai_component(cfg.side, "stage[main_loop].candidate_action",
			{
				id="villages",
				engine="cpp",
				name="ai_default_rca::get_villages_phase",
				max_score=60000,
				score=60000
			}
		)

		wesnoth.add_ai_component(cfg.side, "stage[main_loop].candidate_action",
			{
				id="retreat",
				engine="cpp",
				name="ai_default_rca::retreat_phase",
				max_score=40000,
				score=40000
			}
		)

		wesnoth.add_ai_component(cfg.side, "stage[main_loop].candidate_action",
			{
				id="move_to_targets",
				engine="cpp",
				name="ai_default_rca::move_to_targets_phase",
				max_score=20000,
				score=20000
			}
		)
	else
		if (not cfg.skip_combat_ca) then
			wesnoth.delete_ai_component(cfg.side, "stage[main_loop].candidate_action[high_xp_attack]")
			wesnoth.delete_ai_component(cfg.side, "stage[main_loop].candidate_action[combat]")
		else
			for i,parm in ipairs(CA_parms) do
				if (parm.ca_id == 'combat') or (parm.ca_id == 'combat_leader') then
					table.remove(CA_parms, i)
				end
			end
		end

		if (not cfg.skip_move_ca) then
			wesnoth.delete_ai_component(cfg.side, "stage[main_loop].candidate_action[villages]")

			wesnoth.delete_ai_component(cfg.side, "stage[main_loop].candidate_action[retreat]")

			wesnoth.delete_ai_component(cfg.side, "stage[main_loop].candidate_action[move_to_targets]")
		else
			for i,parm in ipairs(CA_parms) do
				if (parm.ca_id == 'move') then
					table.remove(CA_parms, i)
					break
				end
			end
		end
	end
    return {}, optional_keys, CA_parms
end
