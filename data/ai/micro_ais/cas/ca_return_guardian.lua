local AH = wesnoth.require "ai/lua/ai_helper.lua"

local ca_return_guardian = {}

function ca_return_guardian:evaluation(ai, cfg)
    local unit
    if cfg.filter then
        unit = wesnoth.get_units({
            side = wesnoth.current.side,
            { "and", cfg.filter },
            formula = '$this_unit.moves > 0' }
        )[1]
    else
        unit = wesnoth.get_units({ id = cfg.id, formula = '$this_unit.moves > 0' })[1]
    end

    -- Check if unit exists as sticky BCAs are not always removed successfully
    if unit then
        if ((unit.x ~= cfg.return_x) or (unit.y ~= cfg.return_y)) then
            return cfg.ca_score
        else
            return cfg.ca_score - 20
        end
    end
    return 0
end

function ca_return_guardian:execution(ai, cfg)
    local unit
    if cfg.filter then
        unit = wesnoth.get_units({
            side = wesnoth.current.side,
            { "and", cfg.filter },
            formula = '$this_unit.moves > 0' }
        )[1]
    else
        unit = wesnoth.get_units({ id = cfg.id, formula = '$this_unit.moves > 0' })[1]
    end
    --print("Exec guardian move",unit.id)

    -- In case the return hex is occupied:
    local x, y = cfg.return_x, cfg.return_y
    if (unit.x ~= x) or (unit.y ~= y) then
        x, y = wesnoth.find_vacant_tile(x, y, unit)
    end

    local nh = AH.next_hop(unit, x, y)
    if unit.moves~=0 then
        AH.movefull_stopunit(ai, unit, nh)
    end
end

return ca_return_guardian
