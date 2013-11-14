local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}

local ca_healer_may_attack = {}

-- After attacks by all other units are done, reset things so that healers can attack, if desired
-- This will be blacklisted after first execution each turn
function ca_healer_may_attack:evaluation(ai)
    local score = 99990
    return score
end

function ca_healer_may_attack:execution(ai, cfg, self)
    --print(' Letting healers participate in attacks from now on')

    --local leader = wesnoth.get_units { side = wesnoth.current.side, canrecruit = 'yes' }[1]
    --W.message { speaker = leader.id, message = "I'm done with the RCA AI combat CA for all other units, letting healers participate now (if they cannot find a support position)." }

    -- Delete the attacks aspect
    --print("Deleting attacks aspect")
    W.modify_ai {
        side = wesnoth.current.side,
        action = "try_delete",
        path = "aspect[attacks].facet[no_healers_attack]"
    }

    -- We also reset the variable containing the return score of the healers CA
    -- This will make it use its default value
    self.data.HS_return_score = nil
end

return ca_healer_may_attack
