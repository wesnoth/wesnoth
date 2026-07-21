-- function for saying a particular message, depending on which companion the player has chosen
function wesnoth.wml_actions.companion_message(cfg)
	-- cfg.message_mari
	-- cfg.message_gerrick
	-- cfg.message_hylas
	-- cfg.message_generic
	-- cfg.speaker

	if wml.variables["companion_id"]=="Mari" and cfg.message_mari and wesnoth.units.find_on_map{id="Mari"}[1] then
		local speaker = cfg.speaker or "Mari"
		wesnoth.wml_actions.message{ speaker=speaker, message=cfg.message_mari }

	elseif wml.variables["companion_id"]=="Sir Gerrick" and cfg.message_gerrick and wesnoth.units.find_on_map{id="Sir Gerrick"}[1] then
		local speaker = cfg.speaker or "Sir Gerrick"
		wesnoth.wml_actions.message{ speaker=speaker, message=cfg.message_gerrick }

	elseif wml.variables["companion_id"]=="Minister Hylas" and cfg.message_hylas and wesnoth.units.find_on_map{id="Minister Hylas"}[1] then
		local speaker = cfg.speaker or "Minister Hylas"
		wesnoth.wml_actions.message{ speaker=speaker, message=cfg.message_hylas }

	elseif cfg.message_generic then
		if wesnoth.units.find_on_map{id="Mari"}[1] then
			wesnoth.wml_actions.message{ speaker="Mari", message=cfg.message_generic }
		elseif wesnoth.units.find_on_map{id="Sir Gerrick"}[1] then
			wesnoth.wml_actions.message{ speaker="Sir Gerrick", message=cfg.message_generic }
		elseif wesnoth.units.find_on_map{id="Minister Hylas"}[1] then
			wesnoth.wml_actions.message{ speaker="Minister Hylas", message=cfg.message_generic }
		end
	end
end
