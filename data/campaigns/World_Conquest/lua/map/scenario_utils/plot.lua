
----------------------------------------------------------
---- Adds [event]s to the scenario that contain the   ----
---- talk at the end and at the start of each         ----
---- scenario                                         ----
----------------------------------------------------------

local _ = wesnoth.textdomain "wesnoth-wc"


-- speechs need be rewritten
function add_plot(scenario, scenario_num, nplayers)
	scenario.event = scenario.event or {}
	local start_event = { name = "start" }
	local vicroy_event = { name = "enemies defeated" }
	table.insert(scenario.event, start_event)
	table.insert(scenario.event, vicroy_event)
	local function start_message(side, canrecruit, message)
		table.insert(start_event, wml.tag.message {
			message = message,
			canrecruit = canrecruit,
			side = side
		})
	end
	local function end_message(side, canrecruit, message)
		table.insert(vicroy_event, wml.tag.message {
			message = message,
			canrecruit = canrecruit,
			side = side
		})
	end
	if scenario_num == 1 then
		if nplayers > 1 then
			start_message("2", true, _ "To war! Prepare to defend yourselves!")
			start_message("1", true, _ "Why do we fight amongst ourselves? We should band together and take the coast with our combined forces!")
			start_message("3", true, _ "You speak truly; together we can overpower the rest. After that, who knows? Together we could go on to conquer other lands across the sea!")
		else
			start_message("1", true, _ "To war! Prepare to defend yourselves!")
		end
		start_message("4", true, _ "Foolish upstarts. You will never defeat us! Come, my allies. Let us crush them!")
		start_message("4", false, _ "Yes! We will drive them into the sea!")
		start_message("1,2,3", false, _ "Never fear! Your trusted friends stand beside you!")

		end_message("1,2,3", true, _ "Victory is ours! Let us set sail in search of new lands to conquer!")
	elseif scenario_num == 2 then
		local r = mathx.random(nplayers)
		start_message(r, true, _ "Ahhh. Just look at these fertile lands, ripe for conquest!")
		start_message("4", true, _ "What’s this?! Foreign invaders have set foot upon our shores!")
		start_message("5", true, _ "We’ll send them back to where they came from quick enough. All troops, to me!")

		end_message("1,2,3", false, _ "We win! Truly ours will be a glorious empire!")
		end_message("1,2,3", true, _ "Do not get hasty! To reach the great continents of the east we must subdue the savage coast of Moragdu, notorious for its corsairs, before we can advance further.")
	elseif scenario_num == 3 then
		start_message("4", true, _ "What’s this? It is a long time since strangers have set foot upon Moragdu — and in such great numbers too!")
		start_message("5", true, _ "The time of prophecies is upon us! The destroyers have arrived from beyond the sea!")
		start_message("6", true, _ "I don’t know what the lizard is babbling about, but we’ll sort these ‘destroyers’ out quickly enough.")

		end_message("1,2,3", true, _ "Victory is ours! Let us set sail in search of new lands to conquer!")
	elseif scenario_num == 4 then
		local r = mathx.random(nplayers)
		start_message("5", true, _ "Ready yourselves, men! The conquering hordes are upon us!")
		start_message("4", true, _ "Word has come to our island of your victories. Your army is impressive, but it will avail you not.")
		start_message("6", true, _ "Indeed. You come this far, and no farther!")
		start_message(r, true, _ "Hah! We didn’t come all this way just to give up and go home. We will destroy you as we have destroyed all that have stood before us!")
		start_message("7", true, _ "So be it. We stand squarely behind our allies.")
		start_message("2,3", true, _ "You are not the only ones with allies! We’ll push on till the end!")

		end_message("1,2,3", true, _ "Onwards to the ships!")
		end_message("1,2,3", false, _ "We have travelled great oceans and brought low mighty empires. Should we not wait until our forces regain their strength?")
		end_message("1,2,3", true, _ "One last effort! We better finish our campaign when we hold initiative, before Winter arrives.")
	else
		local r = mathx.random(nplayers)

		start_message(r, true, _ "Finally we come to the shores of the last continent. Here at the edge of the world our empire shall be made complete.")
		start_message("4", true, _ "Never! Your mad quest ends here, tyrant.")
		start_message("5", false, _ "All the peoples of the world stand against you. Though we would be enemies otherwise, we are united in our cause. We must stop you!")
		start_message("1,2,3", false, _ "No, not all of them. Today our full strength stands behind our allies!")

		end_message("1", true, _ "That’s it, our work is done here!")
		end_message("2", true, _ "The whole world is now ours to command!")
		end_message("3", true, _ "It was hard, but I almost regret the fun is over.")
		table.insert(vicroy_event, wml.tag.endlevel {
			result = "victory",
			music = "sad.ogg",
			end_text = _"The End",
			next_scenario = "",
		})
	end
end
