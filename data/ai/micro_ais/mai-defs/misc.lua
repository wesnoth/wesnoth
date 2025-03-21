function wesnoth.micro_ais.assassin(cfg)
	local required_keys = { filter = 'tag', filter_second = 'tag' }
	local optional_keys = { prefer = 'tag' }
	local CA_parms = {
		ai_id = 'mai_assassin',
		{ ca_id = 'attack', location = 'ca_simple_attack.lua', score = 110001 },
		{ ca_id = 'move', location = 'ca_assassin_move.lua', score = 110000 }
	}
	return required_keys, optional_keys, CA_parms
end

function wesnoth.micro_ais.lurkers(cfg)
	local required_keys = { filter = 'tag', filter_location = 'tag' }
	local optional_keys = { stationary = 'boolean', filter_location_wander = 'tag' }
	local CA_parms = {
		ai_id = 'mai_lurkers',
		{ ca_id = 'move', location = 'ca_lurkers.lua', score = cfg.ca_score or 300000 }
	}
	return required_keys, optional_keys, CA_parms
end

-- goto is a keyword, so need to use index operator directly
wesnoth.micro_ais["goto"] = function(cfg)
	local required_keys = { filter_location = 'tag' }
	local optional_keys = { avoid = 'tag', avoid_enemies = 'float', filter = 'tag', ignore_units = 'boolean',
		ignore_enemy_at_goal = 'boolean', release_all_units_at_goal = 'boolean', release_unit_at_goal = 'boolean',
		remove_movement = 'boolean', unique_goals = 'boolean', use_straight_line = 'boolean'
	}
	local CA_parms = {
		ai_id = 'mai_goto',
		{ ca_id = 'move', location = 'ca_goto.lua', score = cfg.ca_score or 300000 }
	}
	return required_keys, optional_keys, CA_parms
end

function wesnoth.micro_ais.hang_out(cfg)
	local optional_keys = { filter = 'tag', filter_location = 'tag', avoid = 'tag',
		mobilize_condition = 'tag', mobilize_on_gold_less_than = 'integer' }
	local CA_parms = {
		ai_id = 'mai_hang_out',
		{ ca_id = 'move', location = 'ca_hang_out.lua', score = cfg.ca_score or 170000 }
	}
	return {}, optional_keys, CA_parms
end

function wesnoth.micro_ais.simple_attack(cfg)
	local optional_keys = { filter = 'tag', filter_second = 'tag', weapon = 'integer' }
	local CA_parms = {
		ai_id = 'mai_simple_attack',
		{ ca_id = 'move', location = 'ca_simple_attack.lua', score = cfg.ca_score or 110000 }
	}
	return {}, optional_keys, CA_parms
end
