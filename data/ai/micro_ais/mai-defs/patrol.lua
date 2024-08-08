local AH = wesnoth.require "ai/lua/ai_helper.lua"

function wesnoth.micro_ais.patrol(cfg)
	if (cfg.action ~= 'delete') then
		if (not cfg.id) and (not wml.get_child(cfg, "filter")) then
			wml.error("Patrol [micro_ai] tag requires either id= key or [filter] tag")
		end
		AH.get_multi_named_locs_xy('waypoint', cfg, 'Patrol [micro_ai] tag')
	end
	local required_keys = {}
	local optional_keys = { id = 'string', filter = 'tag', attack = 'string',
		attack_range = 'integer', attack_invisible_enemies = 'boolean', one_time_only = 'boolean',
		out_and_back = 'boolean', waypoint_loc = 'string', waypoint_x = 'integer_list', waypoint_y = 'integer_list'
	}
	local CA_parms = {
		ai_id = 'mai_patrol',
		{ ca_id = "move", location = 'ca_patrol.lua', score = cfg.ca_score or 300000 }
	}
	return required_keys, optional_keys, CA_parms
end
