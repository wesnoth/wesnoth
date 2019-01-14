local R = wesnoth.require "ai/lua/retreat.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local retreat_unit, retreat_dst

local retreat = {}

function retreat:evaluation(cfg, data)

    local units = AH.get_units_with_moves { side = wesnoth.current.side }
    if (not units[1]) then return 0 end

    local unit, dst, enemy_threat = R.retreat_injured_units(units)

    if unit then
        retreat_unit = unit
        retreat_dst = dst
        return 101000
    end

    return 0
end

function retreat:execution(cfg, data)
    AH.movefull_outofway_stopunit(ai, retreat_unit, retreat_dst[1], retreat_dst[2])
    retreat_unit, retreat_dst = nil, nil
end

return retreat
