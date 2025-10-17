--###########################
-- COMPANION MESSAGE
--###########################
-- function for saying a particular message, depending on which companion(s) are available
function wesnoth.wml_actions.companion_message(cfg)
	-- cfg.priority  e.g. "Delfador,Harper" - first we check Delfador, then we check Harper, then we check the other companions
	-- cfg.message_Delfador  - names must be capitalized, because we use units' IDs to select that unit's message
	-- cfg.message_Kalenz
	-- cfg.message_Chantal
	-- cfg.message_Moremirmu
	-- cfg.message_Harper
	-- cfg.message_Ulfdain
	-- cfg.message_Jeniver
	-- cfg.message_Seimus
	-- cfg.message_Dosh
	-- cfg.fallback_Konrad

	--###########################
	-- INITIALIZE SPEAKERS LIST
	--###########################
	-- use this list to track which companion last spoke, so we can spread lines around evenly
	-- initialize this list once per campaign, and rotate through the companions each time one speaks
	if next(wml.array_variables["companion_ids"])==nil then
		wml.array_variables["companion_ids"] = { {id="Delfador"}, {id="Kalenz"}, {id="Chantal"}, {id="Moremirmu"}, {id="Harper"}, {id="Jeniver"}, {id="Seimus"}, {id="Ulfdain"}, {id="Dosh"} }
	end

	--###########################
	-- PICK COMPANION AND SPEAK
	--###########################
	function find_companion_and_speak(ignore_priority)
		-- iterate in order of $companion_ids, so that whoever spoke last time is the lowest-priority this time
		-- this helps balance out each companion's messages, stops randomness from preventing undo, and ensures that players get the same messages if they reload a save
		--
		-- assign companion_ids to a lua variable so that table.remove() and table.insert() work properly
		local companion_ids = wml.array_variables["companion_ids"]
		for i,companion in pairs(companion_ids) do
			--------------------------
			-- PRIORITIZE
			--------------------------
			-- if we have a list of priority companions, and this companion isn't on the list, move on
			if (not ignore_priority and cfg.priority and not cfg.priority:find(companion.id)) then goto continue end

			--------------------------
			-- SAY MESSAGE
			--------------------------
			-- if we have a living matching companion AND a non-blank message for them to speak, speak
			if (wesnoth.units.find_on_map({id=companion.id}))[1] and cfg["message_"..companion.id] then
				wesnoth.wml_actions.message{ speaker=companion.id, message=cfg["message_"..companion.id] }
				-- shuffle the speaker to the back of the list, so they're deprioritized next time
				table.remove( companion_ids, i )
				table.insert( companion_ids, companion )
				wml.array_variables["companion_ids"] = companion_ids
				return true
			end
			::continue::
		end
	end
	if find_companion_and_speak() then return end

	--###########################
	-- RETRY WITHOUT PRIORITY
	--###########################
	-- if we get here and haven't yet returned, try again without priority
	if (cfg.priority) then
		if find_companion_and_speak(true) then return end
	end

	--###########################
	-- FALLBACK TO KONRAD
	--###########################
	if (cfg["fallback_Konrad"]) then wesnoth.wml_actions.message{ speaker="Konrad", message=cfg["fallback_Konrad"] } end
	return
end
