local event_handlers = {}
local old_on_event = wesnoth.game_events.on_event or function(eventname) end
wesnoth.game_events.on_event = function(eventname)
	old_on_event(eventname)
	for k,v in pairs(event_handlers[eventname] or {}) do
		v()
	end
end

local function on_event(eventname, handler)
	eventname = string.gsub(eventname, " ", "_")
	event_handlers[eventname] = event_handlers[eventname] or {}
	table.insert(event_handlers[eventname], handler)
end

return on_event
