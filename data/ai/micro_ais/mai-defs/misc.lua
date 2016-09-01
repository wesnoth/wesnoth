function wesnoth.micro_ais.assassin(cfg)
	local required_keys = { "[filter]", "[filter_second]" }
	local optional_keys = { "[prefer]" }
	local CA_parms = {
		ai_id = 'mai_assassin',
		{ ca_id = 'attack', location = 'ca_simple_attack.lua', score = 110001 },
		{ ca_id = 'move', location = 'ca_assassin_move.lua', score = 110000 }
	}
    return required_keys, optional_keys, CA_parms
end

function wesnoth.micro_ais.lurkers(cfg)
	local required_keys = { "[filter]", "[filter_location]" }
	local optional_keys = { "stationary", "[filter_location_wander]" }
	local CA_parms = {
		ai_id = 'mai_lurkers',
		{ ca_id = 'move', location = 'ca_lurkers.lua', score = cfg.ca_score or 300000 }
	}
    return required_keys, optional_keys, CA_parms
end

-- goto is a keyword, so need to use index operator directly
wesnoth.micro_ais["goto"] = function(cfg)
	local required_keys = { "[filter_location]" }
	local optional_keys = {
		"avoid_enemies", "[filter]", "ignore_units", "ignore_enemy_at_goal",
		"release_all_units_at_goal", "release_unit_at_goal", "unique_goals", "use_straight_line"
	}
	local CA_parms = {
		ai_id = 'mai_goto',
		{ ca_id = 'move', location = 'ca_goto.lua', score = cfg.ca_score or 300000 }
	}
    return required_keys, optional_keys, CA_parms
end

function wesnoth.micro_ais.hang_out(cfg)
	local optional_keys = { "[filter]", "[filter_location]", "[avoid]", "[mobilize_condition]", "mobilize_on_gold_less_than" }
	local CA_parms = {
		ai_id = 'mai_hang_out',
		{ ca_id = 'move', location = 'ca_hang_out.lua', score = cfg.ca_score or 170000 }
	}
    return {}, optional_keys, CA_parms
end

function wesnoth.micro_ais.simple_attack(cfg)
	local optional_keys = { "[filter]", "[filter_second]", "weapon" }
	local CA_parms = {
		ai_id = 'mai_simple_attack',
		{ ca_id = 'move', location = 'ca_simple_attack.lua', score = cfg.ca_score or 110000 }
	}
    return {}, optional_keys, CA_parms
end
