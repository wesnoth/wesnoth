local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local swarm_scatter = {}

function swarm_scatter:evaluation(ai, cfg)
    local scatter_distance = cfg.scatter_distance or 3

    -- Any enemy within "scatter_distance" hexes of a unit will cause swarm to scatter
    local enemies = wesnoth.get_units {
        { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} },
        { "filter_location",
            { radius = scatter_distance, { "filter", { side = wesnoth.current.side } } }
        }
    }

    if enemies[1] then  -- don't use 'formula=' for moves, it's slow
        local units = wesnoth.get_units { side = wesnoth.current.side }
        for i,u in ipairs(units) do
            if (u.moves > 0) then return cfg.ca_score end
        end
    end

    return 0
end

function swarm_scatter:execution(ai, cfg)
    local scatter_distance = cfg.scatter_distance or 3
    local vision_distance = cfg.vision_distance or 12

    -- Any enemy within "scatter_distance" hexes of a unit will cause swarm to scatter
    local units = wesnoth.get_units { side = wesnoth.current.side }
    for i = #units,1,-1 do
        if (units[i].moves == 0) then table.remove(units, i) end
    end

    local enemies = wesnoth.get_units {
        { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} },
        { "filter_location",
            { radius = scatter_distance, { "filter", { side = wesnoth.current.side } } }
        }
    }

    -- In this case we simply maximize the distance from all these close enemies
    -- but only for units that are within 'vision_distance' of one of those enemies
    for i,unit in ipairs(units) do
        local unit_enemies = {}
        for i,e in ipairs(enemies) do
            if (H.distance_between(unit.x, unit.y, e.x, e.y) <= vision_distance) then
                table.insert(unit_enemies, e)
            end
        end

        if unit_enemies[1] then
            local best_hex = AH.find_best_move(unit, function(x, y)
                local rating = 0
                for i,e in ipairs(unit_enemies) do
                    rating = rating + H.distance_between(x, y, e.x, e.y)
                end
                return rating
            end)

            AH.movefull_stopunit(ai, unit, best_hex)
        end
    end
end

return swarm_scatter
