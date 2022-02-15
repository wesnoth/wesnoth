function wesnoth.micro_ais.bloodlust(cfg)
	if (cfg.action ~= 'delete') then
	    if (not cfg.id) and (not wml.get_child(cfg, "filter")) then
			wml.error("Bloodlust [micro_ai] tag requires either id= key or [filter] tag")
        end
	end
	local required_keys = {}
	local optional_keys = { "id", "[filter]", "[filter_location]" }

	local CA_parms = {
		ai_id = 'mai_bloodlust',
		{ ca_id = "move", location = 'ca_bloodlust.lua', score = cfg.ca_score or 300000 }
	}
    return required_keys, optional_keys, CA_parms
end