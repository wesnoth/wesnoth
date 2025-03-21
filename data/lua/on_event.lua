-- registers an event handler. note that, like all lua variables this is not persitent in savefiles,
-- so you have to call this function from a toplevel lua tag or from a preload event.
-- It is also not possible to use this for first_time_only=yes events.

-- This api used to be the default way to add events from lua (with its own implementation in lua
-- based on game_events.on_event). Now it just calls game_evets.add, because otherwise the
-- priority parameter wouldn't work across the different implementations.
-- Still kept for compatibility and because it has an easier to
-- use interface. Meaning you can easily write in a lua file:
--
-- on_event("moveto", 10, function(ec)
--   ...
-- end)
--
-- which is imo more convenient than the interface wesnoth.game_events.add or wesnoth.game_events.add_repeating offers
-- even though its at this point technically equivalent to the latter.

---Register an event handler
---@param eventname string The event to handle; can be a comma-separated list
---@param priority? number Events execute in order of decreasing priority, and secondarily in order of adding
---@param fcn fun(ctx:event_context)
---@overload fun(eventname:string, fcn:fun(ctx:event_context))
return function(eventname, priority, fcn)
	if type(priority) == "function" then
		fcn = priority
		priority = 0.5
	end

	wesnoth.game_events.add{
		name = eventname,
		priority = priority,
		first_time_only = false,
		action = function()
			local context = wesnoth.current.event_context
			wesnoth.experimental.game_events.set_undoable(true)
			fcn(context)
		end
	}
end
