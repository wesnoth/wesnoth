return {
    init = function(ai, existing_engine)

        local engine = existing_engine or {}

        local AH = wesnoth.require "ai/lua/ai_helper.lua"

        function engine:mai_return_guardian_eval(cfg)
            local unit = wesnoth.get_units { id = cfg.id }[1]

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

        function engine:mai_return_guardian_exec(cfg)
            local unit = wesnoth.get_units { id = cfg.id }[1]
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

        return engine
    end
}
