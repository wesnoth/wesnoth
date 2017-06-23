local AH = wesnoth.require "ai/lua/ai_helper.lua"
local M = wesnoth.map

local function get_tusklets(cfg)
    local tusklets = AH.get_units_with_moves {
        side = wesnoth.current.side,
        type = cfg.tusklet_type
    }
    return tusklets
end

local function get_tuskers(cfg)
    local tuskers = wesnoth.get_units {
        side = wesnoth.current.side,
        type = cfg.tusker_type
    }
    return tuskers
end

local ca_forest_animals_tusklet_move = {}

function ca_forest_animals_tusklet_move:evaluation(cfg)
    -- Tusklets will simply move toward the closest tusker, without regard for anything else
    -- Except if no tuskers are left, in which case ca_forest_animals_move takes over and does a random move

    if (not cfg.tusker_type) or (not cfg.tusklet_type) then return 0 end
    if (not get_tusklets(cfg)[1]) then return 0 end
    if (not get_tuskers(cfg)[1]) then return 0 end
    return cfg.ca_score
end

function ca_forest_animals_tusklet_move:execution(cfg)
    local tusklet = get_tusklets(cfg)[1]
    local tuskers = get_tuskers(cfg)

    local goto_tusker, min_dist = {}, 9e99
    for _,tusker in ipairs(tuskers) do
        local dist = M.distance_between(tusker.x, tusker.y, tusklet.x, tusklet.y)
        if (dist < min_dist) then
            min_dist, goto_tusker = dist, tusker
        end
    end

    local best_hex = AH.find_best_move(tusklet, function(x, y)
        return - M.distance_between(x, y, goto_tusker.x, goto_tusker.y)
    end)

    AH.movefull_stopunit(ai, tusklet, best_hex)

    -- Also make sure tusklets never attack
    if tusklet and tusklet.valid then AH.checked_stopunit_all(ai, tusklet) end
end

return ca_forest_animals_tusklet_move
