local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local MAIUV = wesnoth.dofile "ai/micro_ais/micro_ai_unit_variables.lua"

return function(cfg)
    -- Calculate next waypoint and rating for all messengers
    -- Return next messenger to move and waypoint toward which it should move
    -- Also return the array of all messengers (for escort move evaluation,
    -- so that it only needs to be done in one place, in case the syntax is changed some more)
    -- Returns nil for first 3 arguments if no messenger has moves left
    -- Returns nil for all arguments if there are no messengers on the map

    local filter = cfg.filter or { id = cfg.id }
    local messengers = wesnoth.get_units { side = wesnoth.current.side, { "and", filter } }
    if (not messengers[1]) then return end

    local waypoint_x = AH.split(cfg.waypoint_x, ",")
    local waypoint_y = AH.split(cfg.waypoint_y, ",")
    for i,w in ipairs(waypoint_x) do
        waypoint_x[i] = tonumber(waypoint_x[i])
        waypoint_y[i] = tonumber(waypoint_y[i])
    end

    -- Set the next waypoint for all messengers
    -- Also find those with MP left and return the one to next move, together with the WP to move toward
    local max_rating, best_messenger, x, y = -9e99
    for i, m in ipairs(messengers) do
        -- To avoid code duplication and ensure consistency, we store some pieces of
        -- information in the messenger units, even though it could be calculated each time it is needed
        local wp_i = MAIUV.get_mai_unit_variables(m, cfg.ai_id, "wp_i") or 1
        local wp_x, wp_y = waypoint_x[wp_i], waypoint_y[wp_i]

        -- If this messenger is within 3 hexes of the next waypoint, we go on to the one after that
        -- except if that one's the last one already
        local dist_wp = H.distance_between(m.x, m.y, wp_x, wp_y)
        if (dist_wp <= 3) and (wp_i < #waypoint_x) then wp_i = wp_i + 1 end

        -- Also store the rating for each messenger
        -- For now, this is simply a "forward rating"
        local rating = wp_i - dist_wp / 1000.

        -- If invert_order= key is set, we want to move the rearmost messenger first.
        -- We still want to keep the rating value positive (mostly, this is not strict)
        -- and of the same order of magnitude.
        if cfg.invert_order then
            rating = #waypoint_x - rating
        end

        MAIUV.set_mai_unit_variables(m, cfg.ai_id, { wp_i = wp_i, wp_x = wp_x, wp_y = wp_y, wp_rating = rating })

        -- Find the messenger with the highest rating that has MP left
        if (m.moves > 0) and (rating > max_rating) then
            best_messenger, max_rating = m, rating
            x, y = wp_x, wp_y
        end
    end

    return best_messenger, x, y, messengers
end
