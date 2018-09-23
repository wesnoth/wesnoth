local utils = wesnoth.require "wml-utils"
-- registers an event handler. note that, like all lua variables this is not persitent in savefiles,
-- so you have to call this function from a toplevel lua tag or from a preload event.
-- It is also not possible to use this for first_time_only=yes events.

if rawget(_G, "core_on_event") then
	return rawget(_G, "core_on_event")  -- prevent double execution
end

local event_handlers = {}

local old_on_event = wesnoth.game_events.on_event or function(eventname) end
wesnoth.game_events.on_event = function(eventname)
	old_on_event(eventname)
	local context = nil
	for _, entry in pairs(event_handlers[eventname] or {}) do
		if context == nil then
			context = wesnoth.current.event_context
		end
		entry.h(context)
	end
end


local function on_event(eventname, arg1, arg2)
	if string.match(eventname, ",") then
		for elem in utils.split(eventname or "") do
			on_event(elem, arg1, arg2)
		end
		return
	end
	local priority = 0
	local handler
	if type(arg1) == "function" then
		handler = arg1
	else
		priority = arg1
		handler = arg2
	end
	eventname = string.gsub(eventname, " ", "_")
	event_handlers[eventname] = event_handlers[eventname] or {}
	local eh = event_handlers[eventname]
	table.insert(eh, { h = handler, p = priority})
	-- prioritize last entry
	for i = #eh - 1, 1, -1 do
		if eh[i].p < eh[i + 1].p then
			eh[i], eh[i + 1] = eh[i + 1], eh[i]
		end
	end
end

core_on_event = on_event
return on_event
