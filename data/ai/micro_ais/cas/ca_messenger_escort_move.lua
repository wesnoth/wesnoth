local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "lua/location_set.lua"
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"

local messenger_next_waypoint = wesnoth.require "ai/micro_ais/cas/ca_messenger_f_next_waypoint.lua"

local function get_escorts(cfg)
    local escorts = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", cfg.filter_second }
    }
    return escorts
end

local ca_messenger_escort_move = {}

function ca_messenger_escort_move:evaluation(ai, cfg)
    -- Move escort units close to messengers, and in between messengers and enemies
    -- The messengers have moved at this time, so we don't need to exclude them,
    -- but we check that there are messengers left

    if (not get_escorts(cfg)[1]) then return 0 end

    local _, _, _, messengers = messenger_next_waypoint(cfg)
    if (not messengers) or (not messengers[1]) then return 0 end

    return cfg.ca_score
end

function ca_messenger_escort_move:execution(ai, cfg)
    local escorts = get_escorts(cfg)
    local _, _, _, messengers = messenger_next_waypoint(cfg)

    local enemies = wesnoth.get_units {
        { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } }
    }

    local base_rating_map = LS.create()
    local max_rating, best_unit, best_hex = -9e99
    for _,unit in ipairs(escorts) do
        -- Only considering hexes unoccupied by other units is good enough for this
        local reach_map = AH.get_reachable_unocc(unit)

        -- Minor rating for the fastest and strongest unit to go first
        local unit_rating = unit.max_moves / 100. + unit.hitpoints / 1000.

        reach_map:iter( function(x, y, v)
            local base_rating = base_rating_map:get(x, y)

            if (not base_rating) then
                base_rating = 0

                -- Distance from messenger is most important; only closest messenger counts for this
                -- Give somewhat of a bonus for the messenger that has moved the farthest through the waypoints
                local max_messenger_rating = -9e99
                for _,m in ipairs(messengers) do
                    local messenger_rating = 1. / (H.distance_between(x, y, m.x, m.y) + 2.)
                    local wp_rating = MAIUV.get_mai_unit_variables(m, cfg.ai_id, "wp_rating")
                    messenger_rating = messenger_rating * 10. * (1. + wp_rating * 2.)

                    if (messenger_rating > max_messenger_rating) then
                        max_messenger_rating = messenger_rating
                    end
                end

                base_rating = base_rating + max_messenger_rating

                -- Distance from (sum of) enemies is important too
                -- This favors placing escort units between the messenger and close enemies
                for _,e in ipairs(enemies) do
                    base_rating = base_rating + 1. / (H.distance_between(x, y, e.x, e.y) + 2.)
                end

                base_rating_map:insert(x, y, base_rating)
            end

            local rating = base_rating + unit_rating

            if (rating > max_rating) then
                max_rating = rating
                best_unit, best_hex = unit, { x, y }
            end
        end)
    end

    -- This will always find at least the hex the unit is on -> no check necessary
    AH.movefull_stopunit(ai, best_unit, best_hex)
end

return ca_messenger_escort_move
