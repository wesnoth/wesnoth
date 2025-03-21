local AH = wesnoth.require "ai/lua/ai_helper.lua"

function wesnoth.micro_ais.bottleneck_defense(cfg)
	if (cfg.action ~= 'delete') then
		AH.get_multi_named_locs_xy('', cfg, 'Bottleneck [micro_ai] tag')
		AH.get_multi_named_locs_xy('enemy', cfg, 'Bottleneck [micro_ai] tag')
	end

	local required_keys = {}
	local optional_keys = { location_id = 'string', x = 'integer_list', y = 'integer_list',
		enemy_loc = 'string', enemy_x = 'integer_list', enemy_y = 'integer_list',
		healer_loc = 'string', healer_x = 'integer_list', healer_y = 'integer_list',
		leadership_loc = 'string', leadership_x = 'integer_list', leadership_y = 'integer_list',
		filter = 'tag', active_side_leader = 'boolean'
	}
	local score = cfg.ca_score or 300000
	local CA_parms = {
		ai_id = 'mai_bottleneck',
		{ ca_id = 'move', location = 'ca_bottleneck_move.lua', score = score },
		{ ca_id = 'attack', location = 'ca_bottleneck_attack.lua', score = score - 1 }
	}
	return required_keys, optional_keys, CA_parms
end
