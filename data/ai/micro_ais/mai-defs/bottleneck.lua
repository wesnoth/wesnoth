
function wesnoth.micro_ais.bottleneck_defense(cfg)
	local required_keys = { "x", "y", "enemy_x", "enemy_y" }
	local optional_keys = { "healer_x", "healer_y", "leadership_x", "leadership_y", "active_side_leader" }
	local score = cfg.ca_score or 300000
	local CA_parms = {
		ai_id = 'mai_bottleneck',
		{ ca_id = 'move', location = 'ca_bottleneck_move.lua', score = score },
		{ ca_id = 'attack', location = 'ca_bottleneck_attack.lua', score = score - 1 }
	}
    return required_keys, optional_keys, CA_parms
end