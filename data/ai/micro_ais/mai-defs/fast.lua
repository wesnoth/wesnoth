function wesnoth.micro_ais.fast_ai(cfg)
	local optional_keys = { attack_hidden_enemies = 'boolean', avoid = 'tag', dungeon_mode = 'boolean',
		filter = 'tag', filter_second = 'tag', include_occupied_attack_hexes = 'boolean',
		leader_additional_threat = 'float', leader_attack_max_units = 'integer', leader_weight = 'float',
		move_cost_factor = 'float', weak_units_first = 'boolean', skip_combat_ca = 'boolean',
		skip_move_ca = 'boolean', threatened_leader_fights = 'boolean'
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
		wesnoth.sides.add_ai_component(cfg.side, "stage[main_loop].candidate_action",
			{
				id="castle_switch",
				engine="lua",
				name="ai_default_rca::castle_switch",
				max_score=195000,
				location="ai/lua/ca_castle_switch.lua"
			}
		)

		wesnoth.sides.add_ai_component(cfg.side, "stage[main_loop].candidate_action",
			{
				id="retreat_injured",
				engine="lua",
				name="ai_default_rca::retreat_injured",
				max_score=192000,
				location="ai/lua/ca_retreat_injured.lua"
			}
		)

		wesnoth.sides.add_ai_component(cfg.side, "stage[main_loop].candidate_action",
			{
				id="spread_poison",
				engine="lua",
				name="ai_default_rca::spread_poison",
				max_score=190000,
				location="ai/lua/ca_spread_poison.lua"
			}
		)

		wesnoth.sides.add_ai_component(cfg.side, "stage[main_loop].candidate_action",
			{
				id="high_xp_attack",
				engine="lua",
				name="ai_default_rca::high_xp_attack",
				location="ai/lua/ca_high_xp_attack.lua",
				max_score=100010
			}
		)

		wesnoth.sides.add_ai_component(cfg.side, "stage[main_loop].candidate_action",
			{
				id="combat",
				engine="cpp",
				name="ai_default_rca::combat_phase",
				max_score=100000,
				score=100000
			}
		)

		wesnoth.sides.add_ai_component(cfg.side, "stage[main_loop].candidate_action",
			{
				id="place_healers",
				engine="lua",
				name="ai_default_rca::place_healers",
				max_score=96000,
				location="ai/lua/ca_place_healers.lua"
			}
		)

		wesnoth.sides.add_ai_component(cfg.side, "stage[main_loop].candidate_action",
			{
				id="villages",
				engine="cpp",
				name="ai_default_rca::get_villages_phase",
				max_score=60000,
				score=60000
			}
		)

		wesnoth.sides.add_ai_component(cfg.side, "stage[main_loop].candidate_action",
			{
				id="retreat",
				engine="cpp",
				name="ai_default_rca::retreat_phase",
				max_score=40000,
				score=40000
			}
		)

		wesnoth.sides.add_ai_component(cfg.side, "stage[main_loop].candidate_action",
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
			wesnoth.sides.delete_ai_component(cfg.side, "stage[main_loop].candidate_action[spread_poison]")
			wesnoth.sides.delete_ai_component(cfg.side, "stage[main_loop].candidate_action[high_xp_attack]")
			wesnoth.sides.delete_ai_component(cfg.side, "stage[main_loop].candidate_action[combat]")
		else
			for i,parm in ipairs(CA_parms) do
				if (parm.ca_id == 'combat') or (parm.ca_id == 'combat_leader') then
					table.remove(CA_parms, i)
				end
			end
		end

		if (not cfg.skip_move_ca) then
			wesnoth.sides.delete_ai_component(cfg.side, "stage[main_loop].candidate_action[castle_switch]")
			wesnoth.sides.delete_ai_component(cfg.side, "stage[main_loop].candidate_action[retreat_injured]")
			wesnoth.sides.delete_ai_component(cfg.side, "stage[main_loop].candidate_action[place_healers]")
			wesnoth.sides.delete_ai_component(cfg.side, "stage[main_loop].candidate_action[villages]")
			wesnoth.sides.delete_ai_component(cfg.side, "stage[main_loop].candidate_action[retreat]")
			wesnoth.sides.delete_ai_component(cfg.side, "stage[main_loop].candidate_action[move_to_targets]")
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
