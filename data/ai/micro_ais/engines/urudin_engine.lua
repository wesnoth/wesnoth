return {
    init = function()
        local AH = wesnoth.require "ai/lua/ai_helper.lua"

        local urudin = {}

        -- This is taken almost literally from 'Ka'lian under Attack' in 'Legend of Wesmere'
        function urudin:retreat()
            local urudin_units = wesnoth.units.find_on_map({ side = 3, id = "Urudin" })[1]
            if urudin_units and urudin_units.valid then
                local max_hp, hp = urudin_units.max_hitpoints, urudin_units.hitpoints
                local turn = wesnoth.current.turn
                if (turn >= 5) or (hp < max_hp / 2) then
                    AH.movefull_stopunit(ai, urudin_units, 33, 8)
                end
            end
        end

        return urudin
    end
}
