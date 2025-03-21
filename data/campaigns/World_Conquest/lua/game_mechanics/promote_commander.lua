local _ = wesnoth.textdomain 'wesnoth-wc'
local on_event = wesnoth.require("on_event")

local strings = {
	defeat = _ "No! This is the end!",
	promotion = _ "Don’t lose heart comrades, we can still win this battle."
}

-- when a leader dies, take a commander and make him the leader.
on_event("die", function(cx)
	local u = wesnoth.units.get(cx.x1, cx.y1)
	if (not u) or (not u:matches({ canrecruit = true })) then
		return
	end
	local commander = wesnoth.units.find_on_map {
		side = u.side,
		role = "commander",
		canrecruit = false
	}
	commander = commander[1]
	if commander then
		commander:extract()
		commander.id = u.id
		commander.canrecruit = true
		commander:remove_modifications({ id = "wc2_commander_overlay" })
		commander:to_map()
		wesnoth.wml_actions.message {
			id = commander.id,
			message = strings.promotion
		}
	elseif u.side <= (wml.variables.wc2_highest_player_side or wml.variables.wc2_player_count) then

		-- For 4p, one player is allowed to be defeated, without all players losing the scenario.
		-- By setting wc2_min_players (i.e. by a player), this behaviour could also be enabled for the other scenarios.
		-- For that to work, all scenarios need to be loaded in WC_II_scenario.cfg (only by the hosting player).
		if wml.variables.wc2_player_count > (wml.variables.wc2_min_players or 3) and not wml.variables.wc2_defeated_side then
			wml.variables.wc2_defeated_side = u.side
			-- Still need to know the original player count to detect when another player leader dies.
			wml.variables.wc2_highest_player_side = wml.variables.wc2_player_count
			-- Decrease player count for the next scenario.
			wml.variables.wc2_player_count = wml.variables.wc2_player_count - 1

			if wesnoth.scenario.id == "WC_II_4p" then
				wesnoth.scenario.next = "WC_II_3p"
			elseif wesnoth.scenario.id == "WC_II_3p" then
				wesnoth.scenario.next = "WC_II_2p"
			elseif wesnoth.scenario.id == "WC_II_2p" then
				wesnoth.scenario.next = "WC_II_1p"
			end

			wesnoth.wml_actions.item {
				x = u.x,
				y = u.y,
				image = "items/bones.png",
				z_order = 15,
			}
		else
			wesnoth.wml_actions.message {
				side = "1,2,3",
				message = strings.defeat
			}
			wesnoth.wml_actions.endlevel {
				result = "defeat"
			}
		end
	end
end)

-- clear variables from previous scenario
on_event("start", function()
	wml.variables.wc2_defeated_side = nil
	wml.variables.wc2_highest_player_side = nil
end)
