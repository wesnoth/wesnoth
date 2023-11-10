--[========[Game Event Functions]========]

if wesnoth.kernel_type() == "Game Lua Kernel" then
    wesnoth.add_event_handler = wesnoth.deprecate_api('wesnoth.add_event_handler', 'wesnoth.game_events.add_wml', 1, nil, function(cfg) wesnoth.wml_actions.event(cfg) end)
    wesnoth.remove_event_handler = wesnoth.deprecate_api('wesnoth.remove_event_handler', 'wesnoth.game_events.remove', 1, nil, wesnoth.game_events.remove)

    local function old_fire_event(fcn)
        return function(...)
            local id = select(1, ...)
            local loc1, n1 = wesnoth.map.read_location(select(2, ...))
            local loc2, n2 = wesnoth.map.read_location(select(2 + n1, ...))
            local weap1, weap2 = select(2 + n1 + n2)
            local data = {}
            if weap1 ~= nil then
                table.insert(data, wml.tag.first(weap1))
            end
            if weap2 ~= nil then
                table.insert(data, wml.tag.second(weap2))
            end
            if n1 > 0 then
                if n2 > 0 then
                    fcn(id, loc1, loc2, data)
                else
                    fcn(id, loc1, data)
                end
            else
                fcn(id, data)
            end
        end
    end
    wesnoth.fire_event = wesnoth.deprecate_api('wesnoth.fire_event', 'wesnoth.game_events.fire', 1, nil, old_fire_event(wesnoth.game_events.fire))
    wesnoth.fire_event_by_id = wesnoth.deprecate_api('wesnoth.fire_event_by_id', 'wesnoth.game_events.fire_by_id', 1, nil, old_fire_event(wesnoth.game_events.fire_by_id))
    -- This will be deprecated once it's no longer considered experimental
    wesnoth.allow_undo = wesnoth.game_events.set_undoable

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
