
function wesnoth.micro_ais.healer_support(cfg)
	local optional_keys = { aggression = 'float', injured_units_only = 'boolean',
		max_threats = 'integer', filter = 'tag', filter_second = 'tag'
	}
	-- Scores for this AI need to be hard-coded, it does not work otherwise
	local CA_parms = {
		ai_id = 'mai_healer',
		{ ca_id = 'initialize', location = 'ca_healer_initialize.lua', score = 999990 },
		{ ca_id = 'move', location = 'ca_healer_move.lua', score = 105000 },
	}

	-- The healers_can_attack CA is only added to the table if aggression ~= 0
	-- But: make sure we always try removal
	if (cfg.action == 'delete') or (tonumber(cfg.aggression) ~= 0) then
		table.insert(CA_parms, { ca_id = 'may_attack', location = 'ca_healer_may_attack.lua', score = 99900 })
	end
	return {}, optional_keys, CA_parms
end
