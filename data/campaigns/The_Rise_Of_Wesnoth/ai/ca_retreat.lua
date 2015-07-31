local R = wesnoth.require "ai/lua/retreat.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local retreat = {}

function retreat:evaluation(ai, cfg, self)

    local units = wesnoth.get_units {
        side = wesnoth.current.side,
        formula = 'movement_left > 0'
    }
    --print('#units', #units)
    if (not units[1]) then return 0 end

    local unit, dst, enemy_threat = R.retreat_injured_units(units)

    if unit then
        self.data.retreat_unit = unit
        self.data.retreat_dst = dst
        return 101000
    end

    return 0
end

function retreat:execution(ai, cfg, self)
    AH.movefull_outofway_stopunit(ai, self.data.retreat_unit, self.data.retreat_dst[1], self.data.retreat_dst[2])
    self.data.retreat_unit, self.data.retreat_dst = nil, nil
end

return retreat
