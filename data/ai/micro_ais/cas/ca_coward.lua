local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local function get_coward(cfg)
    local filter = H.get_child(cfg, "filter") or { id = cfg.id }
    local coward = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", filter }
    }[1]

    return coward
end

local ca_coward = {}

function ca_coward:evaluation(cfg)
    if get_coward(cfg) then return cfg.ca_score end
    return 0
end

function ca_coward:execution(cfg)
    local coward = get_coward(cfg)
    local reach = wesnoth.find_reach(coward)

    local filter_second =
        H.get_child(cfg, "filter_second")
        or { { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } } }
    local enemies = AH.get_live_units {
        { "and", filter_second },
        { "filter_location", { x = coward.x, y = coward.y, radius = cfg.distance } },
        { "filter_vision", { side = wesnoth.current.side, visible = 'yes' } }
    }

    -- If no enemies are close: keep unit from doing anything and exit
    if not enemies[1] then
        AH.checked_stopunit_all(ai, coward)
        return
    end

    for i,hex in ipairs(reach) do
        -- Only consider unoccupied hexes
        local unit_in_way = wesnoth.get_unit(hex[1], hex[2])
        if (not AH.is_visible_unit(wesnoth.current.side, unit_in_way))
            or (unit_in_way == coward)
        then
            local rating = 0
            for _,enemy in ipairs(enemies) do
                local dist = H.distance_between(hex[1], hex[2], enemy.x, enemy.y)
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
    for i,pos in ipairs(best_pos) do
        -- Weighting based on distance from 'seek' and 'avoid'
        local dist_seek = AH.generalized_distance(pos[1], pos[2], cfg.seek_x, cfg.seek_y)
        local dist_avoid = AH.generalized_distance(pos[1], pos[2], cfg.avoid_x, cfg.avoid_y)
        local rating = 1 / (dist_seek + 1) - 1 / (dist_avoid + 1)^2 * 0.75

        best_pos[i][4] = rating
    end

    -- Select all those that have the maximum score
    table.sort(best_pos, function(a, b) return a[4] > b[4] end)
    local best_overall = AH.filter(best_pos, function(tmp) return tmp[4] == best_pos[1][4] end)

    -- As final step, if there are more than one remaining locations,
    -- we take the one with the minimum score in the distance-from-enemy criterion
    local max_rating, best_hex = -9e99
    for _,pos in ipairs(best_overall) do
        if (pos[3] > max_rating) then
            max_rating, best_hex = pos[3], pos
        end
    end

    AH.movefull_stopunit(ai, coward, best_hex[1], best_hex[2])
    if (not coward) or (not coward.valid) then return end

    -- If 'attack_if_trapped' is set, the coward attacks the weakest unit it ends up next to
    if cfg.attack_if_trapped then
        local max_rating, best_target = -9e99
        for xa,ya in H.adjacent_tiles(coward.x, coward.y) do
            local target = wesnoth.get_unit(xa, ya)
            if target and wesnoth.is_enemy(coward.side, target.side) then
                local rating = - target.hitpoints

                if (rating > max_rating) then
                    max_rating, best_target = rating, target
                end
            end
        end

        if best_target then
            AH.checked_attack(ai, coward, best_target)
            if (not coward) or (not coward.valid) then return end
        end
    end

    AH.checked_stopunit_all(ai, coward)
end

return ca_coward
