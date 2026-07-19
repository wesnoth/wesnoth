
--###############################
-- DEFINE REGROUP MAI
--###############################
function wesnoth.micro_ais.regroup(cfg)
	-- not using the word "caution" for any of these parameters, because "caution" already has a distinct meaning for the AI
	local required_keys = { retreat_tod='string' };
	local optional_keys = { filter_retreat_target='tag', filter_guards='tag', leader_protect_radius='integer', leader_prudence='boolean', leader_frustration='boolean' };
	local ca_params = {
		ai_id = 'mai_regroup',
		{ ca_id='main',          location='ca_regroup.lua',               score = cfg.ca_score or 290000 },
		{ ca_id='always_attack', location='ca_regroup_always_attack.lua', score =                    500 },
	};
	-- The Spread Poison CA can sometimes be more hurtful than helpful, as it'll often suicidally expose poisoners to try and poison an enemy, even when that enemy has easy access to villages/healers
	-- Leaning on poison also exacerbates the difference between a player with healers and one without (and IMO healers are OP already).
	-- So, disable the Spread Poison CA while the Regroup AI is active.
	wesnoth.wml_actions.modify_ai({ side=wesnoth.current.side, action='delete', path='stage[main_loop].candidate_action[spread_poison]'     });
	return required_keys, optional_keys, ca_params;
end
