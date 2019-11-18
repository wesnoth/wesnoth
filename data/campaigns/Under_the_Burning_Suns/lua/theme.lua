-- #textdomain wesnoth-utbs

local _ = wesnoth.textdomain "wesnoth-utbs"
local old_unit_status = wesnoth.interface.game_display.unit_status

function wesnoth.interface.game_display.unit_status()
	local u = wesnoth.interface.get_displayed_unit()
	if not u then return {} end
	local s = old_unit_status()

	if u.status.dehydrated then
		table.insert(s, { "element", {
			image = "misc/dehydration-status.png",
			tooltip = _ "dehydrated: This unit is dehydrated. It will lose 4 HP and have its damage reduced by 1 each turn during the day unless prevented by healers or cured by water at an oasis.\n\nUnits cannot be killed or deal no damage as a result of dehydration."
		} })
	end

	if u.status.stunned then
		table.insert(s, { "element", { image = "misc/stunned-status-icon.png",
			tooltip = _ "stunned: This unit is stunned. It cannot enforce its Zone of Control."
		} } )
	end

	if u.status.dazed then
		table.insert(s, { "element", { image = "misc/dazed-status-icon.png",
			tooltip = _ "dazed: This unit is dazed. It suffers a -10% penalty to both its defense and chance to hit (except for magical attacks)."
		} } )
	end

	return s
end
