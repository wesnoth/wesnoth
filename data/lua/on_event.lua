-- registers an event handler. note that, like all lua variables this is not persitent in savefiles,
-- so you have to call this function from a toplevel lua tag or from a preload event.
-- It is also not possible to use this for first_time_only=yes events.

-- This api used to be the default way to add events from lua (with its own implementation in lua
-- based on game_events.on_event). Now it just calls game_evets.add, because otherwise the
-- priority parammter wouldn't work across the differnt implmentaions.
-- Still kept for compatibility and because it has an easier to
-- use interface. Meaning you can easily write in a lua file:
--
-- on_event("moveto", 10, function(ec)
--   ...
-- end)
--
-- which is imo more convenient than the interace wesnoth.game_events.add or wesnoth.game_events.add_repeating offers
-- even though its at this point technicially equivalent to the later.


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
