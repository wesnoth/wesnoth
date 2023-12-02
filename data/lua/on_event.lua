-- registers an event handler. note that, like all lua variables this is not persitent in savefiles,
-- so you have to call this function from a toplevel lua tag or from a preload event.
-- It is also not possible to use this for first_time_only=yes events.

return function(eventname, priority, fcn)
	if type(priority) == "function" then
		fcn = priority
		priority = 0
	end
	
	wesnoth.game_events.add{
		name = eventname,
		priority = priority,
		action = function()
			context = wesnoth.current.event_context
			wesnoth.experimental.game_events.set_undoable(true)
			fcn(context)
		end
	}
end
