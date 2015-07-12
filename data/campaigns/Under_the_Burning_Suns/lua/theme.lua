-- #textdomain wesnoth-utbs

local _ = wesnoth.textdomain "wesnoth-utbs"
local old_unit_status = wesnoth.theme_items.unit_status

function wesnoth.theme_items.unit_status()
	local u = wesnoth.get_displayed_unit()
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

	return s
end
