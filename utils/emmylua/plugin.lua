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