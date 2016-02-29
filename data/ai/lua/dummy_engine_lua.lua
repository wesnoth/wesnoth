--! #textdomain wesnoth
-- This is the engine used by the Lua AI when no engine is
-- defined specifically in the [side] tag

-- This provides a cache level for the move map functions,
-- making them a bit easier to use
local ai_stdlib = wesnoth.require('ai/lua/stdlib.lua')
ai_stdlib.init(ai)

-- No special state is returned by the default engine
