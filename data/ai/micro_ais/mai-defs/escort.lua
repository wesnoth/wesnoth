local H = wesnoth.require "helper"

function wesnoth.micro_ais.messenger_escort(cfg)
	if (cfg.action ~= 'delete') and (not cfg.id) and (not H.get_child(cfg, "filter")) then
		H.wml_error("Messenger [micro_ai] tag requires either id= key or [filter] tag")
	end
	local required_keys = { "waypoint_x", "waypoint_y" }
	local optional_keys = { "id", "enemy_death_chance", "[filter]", "[filter_second]", "invert_order", "messenger_death_chance" }
	local score = cfg.ca_score or 300000
	local CA_parms = {
		ai_id = 'mai_messenger',
		{ ca_id = 'attack', location = 'ca_messenger_attack.lua', score = score },
		{ ca_id = 'move', location = 'ca_messenger_move.lua', score = score - 1 },
		{ ca_id = 'escort_move', location = 'ca_messenger_escort_move.lua', score = score - 2 }
	}
    return required_keys, optional_keys, CA_parms
end
