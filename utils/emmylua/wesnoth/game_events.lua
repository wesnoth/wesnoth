---@meta

---@class wesnoth.game_events
wesnoth.game_events = {}

---Hook to handle an event
---@param name string
function wesnoth.game_events.on_event(name) end

---Hook to interpret custom save data
---@param cfg WMLTable
function wesnoth.game_events.on_load(cfg) end

---Hook to emit custom save data
---@return WMLTable
function wesnoth.game_events.on_save() end

---Hook to handle a mouse click on the map
---@param x integer
---@param y integer
function wesnoth.game_events.on_mouse_action(x, y) end

---Hook to handle mouse movement across the map
---@param x integer
---@param y integer
function wesnoth.game_events.on_mouse_move(x, y) end
