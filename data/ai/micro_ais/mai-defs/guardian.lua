local AH = wesnoth.require "ai/lua/ai_helper.lua"

function wesnoth.micro_ais.stationed_guardian(cfg)
	if (cfg.action ~= 'delete') then
		if (not cfg.id) and (not wml.get_child(cfg, "filter")) then
			wml.error("Stationed Guardian [micro_ai] tag requires either id= key or [filter] tag")
		end
		AH.get_named_loc_xy('station', cfg, 'Stationed guardian [micro_ai] tag')
	end
	local required_keys = { distance = 'integer' }
	local optional_keys = { id = 'string', filter = 'tag',
		guard_loc = 'string', guard_x = 'integer', guard_y = 'integer',
		station_loc = 'string', station_x = 'integer', station_y = 'integer'
	}
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
	local required_keys = { filter_location = 'tag' }
	local optional_keys = { id = 'string', filter = 'tag', filter_location_enemy = 'tag',
		station_loc = 'string', station_x = 'integer', station_y = 'integer'
	}
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
	local optional_keys = { id = 'string', filter = 'tag',
		return_loc = 'string', return_x = 'integer', return_y = 'integer'
	}
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
	local required_keys = { distance = 'integer' }
	local optional_keys = { attack_if_trapped = 'boolean', id = 'string', filter = 'tag',
		filter_second = 'tag', seek_loc = 'string', seek_x = 'integer', seek_y = 'integer',
		avoid_loc = 'string', avoid_x = 'integer', avoid_y = 'integer'
	}
	local CA_parms = {
		ai_id = 'mai_coward',
		{ ca_id = 'move', location = 'ca_coward.lua', score = cfg.ca_score or 300000 }
	}
	return required_keys, optional_keys, CA_parms
end
