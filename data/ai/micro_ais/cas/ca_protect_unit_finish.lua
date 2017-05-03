local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local ca_protect_unit_finish, PU_unit, PU_goal = {}

function ca_protect_unit_finish:evaluation(cfg)
    -- If a unit can make it to the goal, this is the first thing that happens
    for u in H.child_range(cfg, "unit") do
        local unit = AH.get_units_with_moves { id = u.id }[1]
        if unit then
            local path, cost = AH.find_path_with_shroud(unit, u.goal_x, u.goal_y)
            if (cost <= unit.moves) and ((unit.x ~= u.goal_x) or (unit.y ~= u.goal_y)) then
                PU_unit = unit
                PU_goal = { u.goal_x, u.goal_y }
                return 300000
            end
        end
    end
    return 0
end

function ca_protect_unit_finish:execution(cfg)
    AH.movefull_stopunit(ai, PU_unit, PU_goal)
    PU_unit, PU_goal = nil, nil
end

return ca_protect_unit_finish
