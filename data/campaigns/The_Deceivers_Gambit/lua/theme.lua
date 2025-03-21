-- #textdomain wesnoth-tdg

local _ = wesnoth.textdomain "wesnoth-tdg"
local old_unit_status = wesnoth.interface.game_display.unit_status

function wesnoth.interface.game_display.unit_status()
	local u = wesnoth.interface.get_displayed_unit()
	if not u then return {} end
	local s = old_unit_status()

	if u.status.dazed then
		table.insert(s, { "element", { image = "misc/dazed-status-icon.png",
			tooltip = _ "dazed: This unit is dazed. It suffers a -10% penalty to both its defense and chance to hit (except for magical attacks)."
		} } )
	end

	return s
end
