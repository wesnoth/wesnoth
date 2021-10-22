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
		commander.canrecruit = true
		commander:remove_modifications({ id = "wc2_commander_overlay" })
		wesnoth.wml_actions.message {
			id = commander.id,
			message = strings.promotion
		}
	elseif u.side <= wml.variables.wc2_player_count or 3 then
		if wml.variables.wc2_player_count or 3 > 3 and not wml.variables.wc2_defeated_side then
			-- For 4p, one player is allowed to be defeated, without all players losing the scenario.
			wml.variables.wc2_defeated_side = u.side
			wml.variables.wc2_player_count = wml.variables.wc2_player_count - 1
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
