
local _ = wesnoth.textdomain "wesnoth-help"
local on_event = wesnoth.require("on_event")
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

local function on_hit(weapon, opponent)
    local text
    if opponent.gender == "female" then
        text = _ "female^stunned"
    else
        text = _ "stunned"
    end
    local color = stringx.join(',', {'196', '196', '128'})
    if not wesnoth.interface.is_skipping_messages() then
        wesnoth.interface.float_label(opponent.x, opponent.y, text, color)
    end
    opponent:add_modification('object', {
        duration = 'turn end',
        wml.tag.effect{
            apply_to = 'image_mod',
            replace = 'CS(50,50,0)'
        },
        wml.tag.effect{
            apply_to = 'zoc',
            value = false
        },
        wml.tag.effect{
            apply_to = 'status',
            add = 'stunned'
        }
    })
end

local weapon_filter = {special_type_active = 'stun'}
local unit_filter = {formula = 'zoc'}

wesnoth.game_events.add{
    name = 'attacker_hits',
    first_time_only = false,
    filter = {
        attack = weapon_filter,
        second_unit = unit_filter
    },
    action = function()
        local ctx = wesnoth.current.event_context
        local weapon = wesnoth.units.create_weapon(wml.get_child(ctx, 'weapon'))
        local opponent = wesnoth.units.get(ctx.x2, ctx.y2)
        on_hit(weapon, opponent)
    end
}

wesnoth.game_events.add{
    name = 'defender_hits',
    first_time_only = false,
    filter = {
        second_attack = weapon_filter,
        unit = unit_filter
    },
    action = function()
        local ctx = wesnoth.current.event_context
        local weapon = wesnoth.units.create_weapon(wml.get_child(ctx, 'second_weapon'))
        local opponent = wesnoth.units.get(ctx.x1, ctx.y1)
        on_hit(weapon, opponent)
    end
}

-- Removing a status via object doesn't work apparently?
-- This uses on_event as a workaround for missing github#7146
wesnoth.game_events.add_repeating("side turn end", function(ctx)
    local all_my_units = wesnoth.units.find{side = wesnoth.current.side}
    for i = 1, #all_my_units do
        all_my_units[i].status.stunned = false
    end
end)
