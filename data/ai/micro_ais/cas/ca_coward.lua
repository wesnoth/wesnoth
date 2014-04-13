local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local function get_coward(cfg)
    local filter = cfg.filter or { id = cfg.id }
    local coward = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", filter }
    }[1]

    return coward
end

local ca_coward = {}

function ca_coward:evaluation(ai, cfg)
    if get_coward(cfg) then return cfg.ca_score end
    return 0
end

function ca_coward:execution(ai, cfg)
    local coward = get_coward(cfg)
    local reach = wesnoth.find_reach(coward)

    local filter_second =
        cfg.filter_second
        or { { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } } }
    local enemies = wesnoth.get_units {
        { "and", filter_second },
        { "filter_location", { x = coward.x, y = coward.y, radius = cfg.distance } }
    }

    -- If no enemies are close: keep unit from doing anything and exit
    if not enemies[1] then
        AH.checked_stopunit_all(ai, coward)
        return
    end

    for i,r in ipairs(reach) do
        -- Only consider unoccupied hexes
        local occ_hex = wesnoth.get_units { x = r[1], y = r[2], { "not", { id = coward.id } } }[1]
        if not occ_hex then
            local rating = 0
            for _,e in ipairs(enemies) do
                local dist = H.distance_between(r[1], r[2], e.x, e.y)
                rating = rating - 1 / dist^2
            end

            -- Store this weighting in the third field of each 'reach' element
            reach[i][3] = rating
        else
            reach[i][3] = -9e99
        end
    end

    -- Select those within factor 2 of the maximum (note: ratings are negative)
    table.sort(reach, function(a, b) return a[3] > b[3] end )
    local best_pos = AH.filter(reach, function(tmp) return tmp[3] > reach[1][3] * 2 end)

    -- Now take 'seek' and 'avoid' into account
    for i,b in ipairs(best_pos) do
        -- Weighting based on distance from 'seek' and 'avoid'
        local dist_seek = AH.generalized_distance(b[1], b[2], cfg.seek_x, cfg.seek_y)
        local dist_avoid = AH.generalized_distance(b[1], b[2], cfg.avoid_x, cfg.avoid_y)
        local rating = 1 / (dist_seek + 1) - 1 / (dist_avoid + 1)^2 * 0.75

        best_pos[i][4] = rating
    end

    -- Select all those that have the maximum score
    table.sort(best_pos, function(a, b) return a[4] > b[4] end)
    local best_overall = AH.filter(best_pos, function(tmp) return tmp[4] == best_pos[1][4] end)

    -- As final step, if there are more than one remaining locations,
    -- we take the one with the minimum score in the distance-from-enemy criterion
    local max_rating, best_hex = -9e99
    for _,b in ipairs(best_overall) do
        if (b[3] > max_rating) then
            max_rating, best_hex = b[3], b
        end
    end

    AH.movefull_stopunit(ai, coward, best_hex[1], best_hex[2])
    if (not coward) or (not coward.valid) then return end

    AH.checked_stopunit_all(ai, coward)
end

return ca_coward
