local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local function get_dog(cfg)
    local dogs = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", H.get_child(cfg, "filter") },
    }
    return dogs[1]
end

-- This CA simply takes moves away from all dogs with moves left. This is done
-- at the end of the AI moves in order to keep dogs adjacent to sheep where
-- they are and not have the default AI take over.

local ca_herding_dog_stopmove = {}

function ca_herding_dog_stopmove:evaluation(cfg)
    if get_dog(cfg) then return cfg.ca_score end
    return 0
end

function ca_herding_dog_stopmove:execution(cfg)
    local dog = get_dog(cfg)

    AH.checked_stopunit_moves(ai, dog)
end

return ca_herding_dog_stopmove
