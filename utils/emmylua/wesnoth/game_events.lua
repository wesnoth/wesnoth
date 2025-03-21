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

---@class event_filter
---@field formula string
---@field condition WML|string
---@field side WML|string
---@field unit WML|string
---@field attack WML|string
---@field second_unit WML|string
---@field second_attack WML|string

---@class game_event_options
---@field name? string|string[] Event to handle, as a string or list of strings
---@field id? string Event ID
---@field menu_item? boolean True if this is a menu item (an ID is required); this means removing the menu item will automatically remove this event. Default false.
---@field first_time_only? boolean Whether this event should fire again after the first time; default true.
---@field priority? number Events execute in order of decreasing priority, and secondarily in order of addition
---@field filter? WML|event_filter|fun(cfg:WML):boolean Event filters as a config with filter tags, a table of the form {filter_type = filter_contents}, or a function
---@field filter_args? WML Arbitrary data that will be passed to the filter, if it is a function. Ignored if the filter is specified as WML or a table.
---@field content? WML The content of the event. This is a WML table passed verbatim into the event when it fires. If no function is specified, it will be interpreted as ActionWML.
---@field action? fun(cfg:WML) The function to call when the event triggers. Defaults to wesnoth.wml_actions.command.

---Add a game event handler
---@param opts game_event_options
function wesnoth.game_events.add(opts) end

---Add a repeating game event handler bound directly to a Lua function
---@param name string|string[] The event or events to handle
---@param action fun(WML) The function called when the event triggers
---@param priority? number Events execute in order of decreasing priority, and secondarily in order of addition
---@param undo_action? fun(WML) The function called if undoing after the event triggers.
function wesnoth.game_events.add_repeating(name, action, priority, undo_action) end

---Add a game event handler triggered from a menu item, bound directly to a Lua function
---@param id string
---@param action fun(WML)
function wesnoth.game_events.add_menu(id, action) end

---Add a game event that runs ActionWML when triggered
---@param event WML Event definition as WML, containing name, ID, filters, and ActionWML to run
function wesnoth.game_events.add_wml(event) end

---Fire an event by name
---@param name string The event to fire
---@param first? location The primary location or unit of the event
---@param second? location The secondary location or unit of the event
---@param data? WMLTable Additional data to pass to the event
---@return boolean #Indicates whether the event was handled or not
---@overload fun(name:string, x1:integer, y1:integer, data?:WMLTable):boolean
---@overload fun(name:string, x1:integer, y1:integer, x2:integer, y2:integer, data?:WMLTable):boolean
---@overload fun(name:string, first:location, x2:integer, y2:integer, data?:WMLTable):boolean
---@overload fun(name:string, x1:integer, y1:integer, second:location, data?:WMLTable):boolean
function wesnoth.game_events.fire(name, first, second, data) end

---Fire an event by ID
---@param id string The event to fire
---@param first? location The primary location or unit of the event
---@param second? location The secondary location or unit of the event
---@param data? WMLTable Additional data to pass to the event
---@return boolean #Indicates whether the event was handled or not
---@overload fun(id:string, x1:integer, y1:integer, data?:WMLTable):boolean
---@overload fun(id:string, x1:integer, y1:integer, x2:integer, y2:integer, data?:WMLTable):boolean
---@overload fun(id:string, first:location, x2:integer, y2:integer, data?:WMLTable):boolean
---@overload fun(id:string, x1:integer, y1:integer, second:location, data?:WMLTable):boolean
function wesnoth.game_events.fire_by_id(id, first, second, data) end

---Remove an event handler by ID
---@param id string The event to remove
function wesnoth.game_events.remove(id) end

---Set whether the current event is undoable.
---@param can_undo boolean Whether the event is undoable.
function wesnoth.game_events.set_undoable(can_undo) end

---Add undo actions for the current event
---@param actions WML|fun(ctx) The undo actions, either as ActionWML or a Lua function.
function wesnoth.game_events.add_undo_actions(actions) end
