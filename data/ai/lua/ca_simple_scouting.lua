-- An example CA that tries to uncover shrouded areas of the map
-- This is a very simple and naive scouting algorithm.
-- It often does stupid things like sending one unit to a village on one turn,
-- but then changing its mind and sending a different unit to that village on the next turn.
-- Or sending a faraway unit instead of a close unit to a given village.

local LS = wesnoth.require "location_set"
local F = wesnoth.require "functional"
local AH = wesnoth.require "ai/lua/ai_helper"
local simple_scouting = {}

local possible_scouts, shroud

function simple_scouting:evaluation(cfg, data, filter_own)
    shroud = LS.of_pairs(wesnoth.map.find{
        include_borders = false,
        wml.tag.filter_vision{
            visible = false,
            respect_fog = false,
            side = ai.side,
        }
    })
    if shroud:size() == 0 then return 0 end
    possible_scouts = wesnoth.units.find{
        side = ai.side,
        canrecruit = false,
        formula = "max_moves > 5 and moves > 0",
        wml.tag["and"](filter_own)
    }
    if #possible_scouts == 0 then return 0 end
    return 30000
end

function simple_scouting:execution(cfg, data, filter_own)
    local villages = LS.of_pairs(wesnoth.map.find{owner_side = 0, gives_income = true})
    while villages:size() > 0 and #possible_scouts > 0 do
        local current_scout = table.remove(possible_scouts)
        local best = F.choose(villages:to_pairs(), function(loc)
            return -wesnoth.map.distance_between(loc, current_scout)
        end)
        if not best then
            table.insert(possible_scouts, current_scout)
            break
        end
        local hop = AH.next_hop(current_scout, best.x, best.y)
        ai.move(current_scout, hop)
        villages:remove(best)
    end
    while shroud:size() > 0 and #possible_scouts > 0 do
        local current_scout = table.remove(possible_scouts)
        local best = F.choose(shroud:to_pairs(), function(loc)
            return -wesnoth.map.distance_between(loc, current_scout)
        end)
        if not best then break end
        local hop = AH.next_hop(current_scout, best.x, best.y)
        ai.move(current_scout, hop)
        shroud:remove(best)
    end
end

return simple_scouting
