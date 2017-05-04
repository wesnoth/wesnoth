local H = wesnoth.require "helper"

local ca_healer_initialize = {}

function ca_healer_initialize:evaluation()
    -- Set variables and aspects so that healers are excluded from attacks at beginning of turn
    -- This will be blacklisted after first execution each turn

    local score = 999990
    return score
end

function ca_healer_initialize:execution(cfg, data)
    wesnoth.delete_ai_component(wesnoth.current.side, "aspect[attacks].facet[no_healers_attack]")

    wesnoth.add_ai_component(wesnoth.current.side, "aspect[attacks].facet",
        {
            name = "ai_default_rca::aspect_attacks",
            id = "no_healers_attack",
            invalidate_on_gamestate_change = "yes",
            { "filter_own", {
               { "not", { ability = "healing", { "and", H.get_child(cfg, "filter") } } }
            } }
        }
    )

    -- We also need to set the score of healer moves to happen after
    -- combat (of other units) at beginning of turn
    data.HS_healer_move_score = 95000
end

return ca_healer_initialize
