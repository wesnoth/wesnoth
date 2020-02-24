local AH = wesnoth.require "ai/lua/ai_helper.lua"

function wesnoth.micro_ais.stationed_guardian(cfg)
	if (cfg.action ~= 'delete') then
		if (not cfg.id) and (not wml.get_child(cfg, "filter")) then
			wml.error("Stationed Guardian [micro_ai] tag requires either id= key or [filter] tag")
		end
		AH.get_named_loc_xy('station', cfg, 'Stationed guardian [micro_ai] tag')
	end
	local required_keys = { "distance" }
	local optional_keys = { "id", "[filter]", "guard_loc", "guard_x", "guard_y", "station_loc", "station_x", "station_y" }
	local CA_parms = {
		ai_id = 'mai_stationed_guardian',
		{ ca_id = 'move', location = 'ca_stationed_guardian.lua', score = cfg.ca_score or 300000 }
	}
	return required_keys, optional_keys, CA_parms
end

function wesnoth.micro_ais.zone_guardian(cfg)
	if (cfg.action ~= 'delete') and (not cfg.id) and (not wml.get_child(cfg, "filter")) then
		wml.error("Zone Guardian [micro_ai] tag requires either id= key or [filter] tag")
	end
	local required_keys = { "[filter_location]" }
	local optional_keys = { "id", "[filter]", "[filter_location_enemy]", "station_loc", "station_x", "station_y" }
	local CA_parms = {
		ai_id = 'mai_zone_guardian',
		{ ca_id = 'move', location = 'ca_zone_guardian.lua', score = cfg.ca_score or 300000 }
	}
	return required_keys, optional_keys, CA_parms
end

function wesnoth.micro_ais.return_guardian(cfg)
	if (cfg.action ~= 'delete') then
		if (not cfg.id) and (not wml.get_child(cfg, "filter")) then
			wml.error("Return Guardian [micro_ai] tag requires either id= key or [filter] tag")
		end
		AH.get_named_loc_xy('return', cfg, 'Return guardian [micro_ai] tag')
	end
	local required_keys = {}
	local optional_keys = { "id", "[filter]", "return_loc", "return_x", "return_y" }
	local CA_parms = {
		ai_id = 'mai_return_guardian',
		{ ca_id = 'move', location = 'ca_return_guardian.lua', score = cfg.ca_score or 100100 }
	}
	return required_keys, optional_keys, CA_parms
end

function wesnoth.micro_ais.coward(cfg)
	if (cfg.action ~= 'delete') and (not cfg.id) and (not wml.get_child(cfg, "filter")) then
		wml.error("Coward [micro_ai] tag requires either id= key or [filter] tag")
	end
	local required_keys = { "distance" }
	local optional_keys = { "attack_if_trapped", "id", "[filter]", "[filter_second]", "seek_loc", "seek_x", "seek_y", "avoid_loc", "avoid_x", "avoid_y" }
	local CA_parms = {
		ai_id = 'mai_coward',
		{ ca_id = 'move', location = 'ca_coward.lua', score = cfg.ca_score or 300000 }
	}
	return required_keys, optional_keys, CA_parms
end
