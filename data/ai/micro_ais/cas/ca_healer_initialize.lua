local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}

local ca_healer_initialize = {}

function ca_healer_initialize:evaluation(ai)
    -- Set variables and aspects so that healers are excluded from attacks at beginning of turn
    -- This will be blacklisted after first execution each turn

    local score = 999990
    return score
end

function ca_healer_initialize:execution(ai, cfg, self)
    W.modify_ai {
        side = wesnoth.current.side,
        action = "try_delete",
        path = "aspect[attacks].facet[no_healers_attack]"
    }

    W.modify_ai {
        side = wesnoth.current.side,
        action = "add",
        path = "aspect[attacks].facet",
        { "facet", {
            name = "ai_default_rca::aspect_attacks",
            id = "no_healers_attack",
            invalidate_on_gamestate_change = "yes",
            { "filter_own", {
               { "not", { ability = "healing", { "and", cfg.filter } } }
            } }
        } }
    }

    -- We also need to set the score of healer moves to happen after
    -- combat (of other units) at beginning of turn
    self.data.HS_healer_move_score = 95000
end

return ca_healer_initialize
