---@meta

---@alias plugin_callback fun(data:WMLTable)
---@alias plugin_accessor fun(query?:WMLTable):string|integer|WMLTable

---Contains mutators for the current context.
---@class plugin_context
---@field [string] plugin_callback A mutator takes a WML table as its only argument.

---Contains accessors for the current context
---@class plugin_info
---@field name string The name of the current context.
---@field [string] plugin_accessor An accessor takes a WML table as its argument and returns a string, integer, or WML table.

---Execute a function within the current context's game state, if supported.
---Functions returning a value can request that the result be returned in an event in the next slice.
---If the function raises an error, that too will be returned as an event in the next slice.
---@param context plugin_context The current plugin context.
---@param fcn function An arbitrary function to execute. The function will be run in a different Lua kernel and thus cannot access the wesnoth.plugin module.
---@param event_name? string The name to use for the event that contains the function's result
---@return boolean #True if the function will be executed; false if unsupported
---@return string? #If the first value is false, this will hold an explanatory string
function wesnoth.plugin.execute(context, fcn, event_name) end
