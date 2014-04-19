local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "lua/location_set.lua"

local function get_escort(cfg)
    local escorts = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", cfg.filter_second }
    }
    return escorts[1]
end

local function get_messenger(cfg)
    local messenger = wesnoth.get_units { side = wesnoth.current.side, id = cfg.id }[1]
    return messenger
end

local ca_messenger_escort_move = {}

function ca_messenger_escort_move:evaluation(ai, cfg)
    -- Move escort units close to messenger and in between messenger and enemies.
    -- The messenger has moved at this time, so we don't need to exclude him,
    -- but we check that he exist
    if (not get_escort(cfg)) then return 0 end
    if (not get_messenger(cfg)) then return 0 end
    return cfg.ca_score
end

function ca_messenger_escort_move:execution(ai, cfg)
    local escort = get_escort(cfg)
    local messenger = get_messenger(cfg)

    local reach = LS.of_pairs(wesnoth.find_reach(escort))
    local dist_messenger = AH.distance_map({ messenger }, reach)

    local enemies = wesnoth.get_units {
        { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } }
    }

    -- Rating (in the end, we pick the _minimum_ rating):
    -- 1. Minimize distance from enemies
    local rating = AH.distance_map(enemies, reach)
    -- 2. This one favors hexes in between messenger and enemies
    rating:union_merge(dist_messenger, function(x, y, v1, v2)
        return v1 + v2 * #enemies
    end)
    -- 3. Strongly prefer hexes close to the messenger
    rating:union_merge(dist_messenger, function(x, y, v1, v2)
        return v1 + v2^2
    end)

    min_rating, best_hex = 9e99
    rating:iter(function(x, y, r)
        local unit_in_way = wesnoth.get_units { x = x, y = y, { "not", { id = escort.id } } }[1]
        if (not unit_in_way) and (r < min_rating) then
           min_rating, best_hex = r, { x, y }
        end
    end)

    AH.movefull_stopunit(ai, escort, best_hex)
end

return ca_messenger_escort_move
