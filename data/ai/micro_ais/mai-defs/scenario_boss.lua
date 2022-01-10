function wesnoth.micro_ais.boss(cfg)
	if (cfg.action ~= 'delete') then
	    if (not cfg.id) and (not wml.get_child(cfg, "filter")) then
			wml.error("Boss [micro_ai] tag requires either id= key or [filter] tag")
        end
	end
	local required_keys = {}
	local optional_keys = { "id", "[filter]", "[filter_location]" }

	local CA_parms = {
		ai_id = 'mai_boss',
		{ ca_id = "move", location = '~add-ons/Hero_of_Irdya_I/ai/micro_ais/cas/ca_boss.lua', score = cfg.ca_score or 300000 }
	}
    return required_keys, optional_keys, CA_parms
end