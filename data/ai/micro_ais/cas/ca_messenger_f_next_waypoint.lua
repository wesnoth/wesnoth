local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"

return function(messenger, cfg)
    -- Store which waypoint to go to next in the unit
    local wp = MAIUV.get_mai_unit_variables(messenger, cfg.ai_id, "wp") or 1

    local waypoint_x = AH.split(cfg.waypoint_x, ",")
    local waypoint_y = AH.split(cfg.waypoint_y, ",")
    for i,_ in ipairs(waypoint_x) do
        waypoint_x[i] = tonumber(waypoint_x[i])
        waypoint_y[i] = tonumber(waypoint_y[i])
    end

    -- If we're within 3 hexes of the next waypoint, we go on to the one
    -- after that except if it's the last one
    local dist_wp = H.distance_between(messenger.x, messenger.y, waypoint_x[wp], waypoint_y[wp])
    if (dist_wp <= 3) and (wp < #waypoint_x) then wp = wp + 1 end

    MAIUV.set_mai_unit_variables(messenger, cfg.ai_id, { wp = wp })

    return waypoint_x[wp], waypoint_y[wp]
end
