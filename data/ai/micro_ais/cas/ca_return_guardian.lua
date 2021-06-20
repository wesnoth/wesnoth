local AH = wesnoth.require "ai/lua/ai_helper.lua"

local function get_guardian(cfg)
    local filter = wml.get_child(cfg, "filter") or { id = cfg.id }
    local guardian = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", filter }
    }[1]

    return guardian
end

local ca_return_guardian = {}

function ca_return_guardian:evaluation(cfg)
    local guardian = get_guardian(cfg)
    if guardian then
        local return_loc = AH.get_named_loc_xy('return', cfg)
        if (guardian.x == return_loc[1]) and (guardian.y == return_loc[2]) then
            return cfg.ca_score - 200
        else
            return cfg.ca_score
        end
    end

    return 0
end

function ca_return_guardian:execution(cfg)
    local guardian = get_guardian(cfg)
    local return_loc = AH.get_named_loc_xy('return', cfg)

    -- In case the return hex is occupied:
    local x, y = return_loc[1], return_loc[2]
    if (guardian.x ~= x) or (guardian.y ~= y) then
        x, y = wesnoth.paths.find_vacant_hex(x, y, guardian)
    end

    local nh = AH.next_hop(guardian, x, y)
    if (not nh) then nh = { guardian.x, guardian.y } end

    AH.movefull_stopunit(ai, guardian, nh)
end

return ca_return_guardian
