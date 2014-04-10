local AH = wesnoth.require "ai/lua/ai_helper.lua"

local ca_protect_unit_finish = {}

function ca_protect_unit_finish:evaluation(ai, cfg, self)
    -- If a unit can make it to the goal, this is the first thing that happens
    for i,id in ipairs(cfg.id) do
        local unit = AH.get_units_with_moves { id = id }[1]
        if unit then
            local path, cost = wesnoth.find_path(unit, cfg.goal_x[i], cfg.goal_y[i])
            if (cost <= unit.moves) and ((unit.x ~= cfg.goal_x[i]) or (unit.y ~= cfg.goal_y[i])) then
                self.data.unit = unit
                self.data.goal = { cfg.goal_x[i], cfg.goal_y[i] }
                return 300000
            end
        end
    end
    return 0
end

function ca_protect_unit_finish:execution(ai, cfg, self)
    AH.movefull_stopunit(ai, self.data.unit, self.data.goal)
    self.data.unit = nil
    self.data.goal = nil
end

return ca_protect_unit_finish
