------- Move To Any Enemy CA --------------
-- Move AI units toward any enemy on the map. This has a very low CA score and
-- only kicks in when the AI would do nothing else. It prevents the AI from
-- being inactive on maps without enemy leaders and villages.

local AH = wesnoth.require "ai/lua/ai_helper.lua"

local MTAE_unit, MTAE_destination

local ca_move_to_any_enemy = {}

function ca_move_to_any_enemy:evaluation(cfg, data)
    local start_time, ca_name = wesnoth.get_time_stamp() / 1000., 'move_to_any_enemy'
    if AH.print_eval() then AH.print_ts('     - Evaluating move_to_any_enemy CA:') end

    local units = AH.get_units_with_moves {
        side = wesnoth.current.side,
        canrecruit = 'no'
    }

    if (not units[1]) then
        -- No units with moves left
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    local unit, destination
    -- Find a unit that has a path to an space close to an enemy
    for i,u in ipairs(units) do
        local distance, target = AH.get_closest_enemy({u.x, u.y})
        if target then
            unit = u

            local x, y = wesnoth.find_vacant_tile(target.x, target.y)
            destination = AH.next_hop(unit, x, y)

            if destination then
                break
            end
        end
    end

    if (not destination) then
        -- No path was found
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    MTAE_destination = destination
    MTAE_unit = unit

    if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
    return 1000
end

function ca_move_to_any_enemy:execution(cfg, data)
    if AH.print_exec() then AH.print_ts('   Executing move_to_any_enemy CA') end
    AH.checked_move(ai, MTAE_unit, MTAE_destination[1], MTAE_destination[2])
    MTAE_unit, MTAE_destination = nil,nil
end

return ca_move_to_any_enemy
