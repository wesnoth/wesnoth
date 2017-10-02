return {
    init = function(ai)
        local AH = wesnoth.require "ai/lua/ai_helper.lua"

        local move_to_any_target = {}

        function move_to_any_target:move_to_enemy_eval()
            local units = wesnoth.get_units {
                side = wesnoth.current.side,
                canrecruit = 'no',
                formula = 'movement_left > 0'
            }

            if (not units[1]) then
                -- No units with moves left
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
                return 0
            end

            self.data.destination = destination
            self.data.unit = unit

            return 1
        end

        function move_to_any_target:move_to_enemy_exec()
            AH.checked_move(ai, self.data.unit, self.data.destination[1], self.data.destination[2])
        end

        return move_to_any_target
    end
}
