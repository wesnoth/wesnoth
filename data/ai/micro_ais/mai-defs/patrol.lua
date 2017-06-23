local H = wesnoth.require "helper"

function wesnoth.micro_ais.patrol(cfg)
	if (cfg.action ~= 'delete') and (not cfg.id) and (not H.get_child(cfg, "filter")) then
		H.wml_error("Patrol [micro_ai] tag requires either id= key or [filter] tag")
	end
	local required_keys = { "waypoint_x", "waypoint_y" }
	local optional_keys = { "id", "[filter]", "attack", "one_time_only", "out_and_back" }
	local CA_parms = {
		ai_id = 'mai_patrol',
		{ ca_id = "move", location = 'ca_patrol.lua', score = cfg.ca_score or 300000 }
	}
    return required_keys, optional_keys, CA_parms
end
