
local _ = wesnoth.textdomain "wesnoth-units"
local old_unit_status = wesnoth.interface.game_display.unit_status

function wesnoth.interface.game_display.unit_status()
    local u = wesnoth.interface.get_displayed_unit()
    if not u then return {} end
    local s = old_unit_status()

    if u.status.stunned then
        table.insert(s, wml.tag.element{
            image = "misc/stunned-status-icon.png",
            tooltip = _"stunned: This unit is stunned. It cannot enforce its Zone of Control."
        })
    end

    return s
end
