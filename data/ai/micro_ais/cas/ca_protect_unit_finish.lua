local AH = wesnoth.require "ai/lua/ai_helper.lua"

local ca_protect_unit_finish, PU_unit, PU_goal = {}, nil, nil

function ca_protect_unit_finish:evaluation(cfg)
    -- If a unit can make it to the goal, this is the first thing that happens
    for u in wml.child_range(cfg, "unit") do
        local unit = AH.get_units_with_moves { id = u.id, side = wesnoth.current.side }[1]
        if unit then
            local goal = AH.get_named_loc_xy('goal', u)
            local path, cost = AH.find_path_with_shroud(unit, goal[1], goal[2])
            if (cost <= unit.moves) and ((unit.x ~= goal[1]) or (unit.y ~= goal[2])) then
                PU_unit = unit
                PU_goal = { goal[1], goal[2] }
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
