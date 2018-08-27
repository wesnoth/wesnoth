local R = wesnoth.require "ai/lua/retreat.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local retreat = {}

function retreat:evaluation(cfg, data)

    local units = wesnoth.get_units {
        side = wesnoth.current.side,
        formula = 'movement_left > 0'
    }
    if (not units[1]) then return 0 end

    local unit, dst, enemy_threat = R.retreat_injured_units(units)

    if unit then
        data.retreat_unit = unit
        data.retreat_dst = dst
        return 101000
    end

    return 0
end

function retreat:execution(cfg, data)
    AH.movefull_outofway_stopunit(ai, data.retreat_unit, data.retreat_dst[1], data.retreat_dst[2])
    data.retreat_unit, data.retreat_dst = nil, nil
end

return retreat
