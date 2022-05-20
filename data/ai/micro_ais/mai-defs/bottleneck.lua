local AH = wesnoth.require "ai/lua/ai_helper.lua"

function wesnoth.micro_ais.bottleneck_defense(cfg)
	if (cfg.action ~= 'delete') then
		AH.get_multi_named_locs_xy('', cfg, 'Bottleneck [micro_ai] tag')
		AH.get_multi_named_locs_xy('enemy', cfg, 'Bottleneck [micro_ai] tag')
	end

	local required_keys = {}
	local optional_keys = { "location_id", "x", "y", "enemy_loc", "enemy_x", "enemy_y", "[filter]",
		"healer_loc", "healer_x", "healer_y", "leadership_loc","leadership_x", "leadership_y", "active_side_leader"
	}
	local score = cfg.ca_score or 300000
	local CA_parms = {
		ai_id = 'mai_bottleneck',
		{ ca_id = 'move', location = 'ca_bottleneck_move.lua', score = score },
		{ ca_id = 'attack', location = 'ca_bottleneck_attack.lua', score = score - 1 }
	}
    return required_keys, optional_keys, CA_parms
end
