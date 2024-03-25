---@meta

---@class schedule_base
---@field time_of_day string
---Represents the global time of day schedule
---@class schedule : schedule_base
---@field liminal_bonus number
---Represents a local area schedule
---@class time_area : schedule_base
---@field id string
---@field hexes location[]
---@class time_info
---@field id string
---@field lawful_bonus integer
---@field bonus_modified integer
---@field red integer
---@field green integer
---@field blue integer
---@field image string
---@field mask string
---@field sound string
---@field name tstring

---Get the time of day on the given hex, ignoring illumination
---@param ref location|string|nil
---@param turn? integer
---@return time_info
---@overload fun(x:integer, y:integer)
---@overload fun(x:integer, y:integer, turn:integer)
function wesnoth.schedule.get_time_of_day(ref, turn) end

---Get the time of day on the given hex, accounting for illumination
---@param ref location|string|nil
---@param turn? integer
---@return time_info
function wesnoth.schedule.get_illumination(ref, turn) end

---Replace the current schedule
---@param new time_info[]|WML|schedule_base
function wesnoth.schedule.replace(new) end