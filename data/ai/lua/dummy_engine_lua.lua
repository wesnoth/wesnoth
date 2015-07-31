--! #textdomain wesnoth
-- This is the engine used by the Lua AI when no engine is
-- defined specifically in the [side] tag

return {
    get_ai = function(ai)
        local my_ai = {}

        local ai_stdlib = wesnoth.require('ai/lua/stdlib.lua')
        ai_stdlib.init(ai)

        -- Make the ai table available to the eval/exec functions
        function my_ai:get_ai()
            return ai
        end

        -- Make the persistent data table available to the eval/exec functions
        my_ai.data = {}

        return my_ai
    end
}
