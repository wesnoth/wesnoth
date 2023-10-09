--[========[Game Event Functions]========]

if wesnoth.kernel_type() == "Game Lua Kernel" then
    -- The undo API is still experimental, so move those functions

    -- Make sure wesnoth.experimental.game_events actually exists
    -- It's done this way so it doesn't break if we later need to add things here from C++
    wesnoth.experimental = wesnoth.experimental or {}
    wesnoth.experimental.game_events = wesnoth.experimental.game_events or {}

    wesnoth.experimental.game_events.set_undoable = wesnoth.game_events.set_undoable
    wesnoth.experimental.game_events.add_undo_actions = wesnoth.game_events.add_undo_actions
    wesnoth.game_events.set_undoable = nil
    wesnoth.game_events.add_undo_actions = nil
end
