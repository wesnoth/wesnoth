local AH = wesnoth.require "ai/lua/ai_helper.lua"

function wesnoth.micro_ais.messenger_escort(cfg)
	if (cfg.action ~= 'delete') then
		if (not cfg.id) and (not wml.get_child(cfg, "filter")) then
			wml.error("Messenger [micro_ai] tag requires either id= key or [filter] tag")
		end
		AH.get_multi_named_locs_xy('waypoint', cfg, 'Messenger [micro_ai] tag')
	end
	local required_keys = {}
	local optional_keys = { "[avoid]", "id", "enemy_death_chance", "[filter]", "[filter_second]", "invert_order", "messenger_death_chance", "waypoint_loc", "waypoint_x", "waypoint_y" }
	local score = cfg.ca_score or 300000
	local CA_parms = {
		ai_id = 'mai_messenger',
		{ ca_id = 'attack', location = 'ca_messenger_attack.lua', score = score },
		{ ca_id = 'move', location = 'ca_messenger_move.lua', score = score - 1 },
		{ ca_id = 'escort_move', location = 'ca_messenger_escort_move.lua', score = score - 2 }
	}
    return required_keys, optional_keys, CA_parms
end
