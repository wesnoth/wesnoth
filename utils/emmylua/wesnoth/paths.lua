---@meta

wesnoth.paths = {}

---@alias path_function fun(x:integer, y:integer, cost:integer):integer

---@class path_options
---@field max_cost? integer
---@field ignore_units? boolean
---@field ignore_teleport? boolean
---@field viewing_side? integer
---@field width? integer
---@field height? integer
---@field include_borders? boolean
---@field calculate? path_function

---Find a good path between two hexes
---@param start location
---@param finish location
---@param options path_options
---@return location[] path
---@return integer cost
---@overload fun(x1:integer, y1:integer, finish:location, options:path_options):location[],integer
---@overload fun(start:location, x2:integer, y2:integer, options:path_options):location[],integer
---@overload fun(x1:integer, y1:integer, x2:integer, y2:integer, options:path_options):location[],integer
---@overload fun(start:location, finish:location, calc:path_function):location[],integer
---@overload fun(x1:integer, y1:integer, finish:location, calc:path_function):location[],integer
---@overload fun(start:location, x2:integer, y2:integer, calc:path_function):location[],integer
---@overload fun(x1:integer, y1:integer, x2:integer, y2:integer, calc:path_function):location[],integer
function wesnoth.paths.find_path(start, finish, options) end

---Find a vacant hex as close as possible
---@param loc location
---@param unit? unit
---@return integer x, integer y
---@overload fun(x:integer, y:integer, unit:unit):integer,integer
function wesnoth.paths.find_vacant_hex(loc, unit) end

---@class reach_options
---@field additional_turns? integer
---@field ignore_units? boolean
---@field ignore_teleport? boolean
---@field viewing_side? integer

---@class reachable_location : location
---@field moves_left integer

---Get all locations a unit can reach
---@param unit unit
---@param options? reach_options
---@return reachable_location[]
function wesnoth.paths.find_reach(unit, options) end

---Get all locations a unit can see
---@param unit unit
---@return location[]
function wesnoth.paths.find_vision_range(unit) end

---@class cost_map_types
---@field [1] integer x coordinate
---@field [2] integer y coordinate
---@field [3] integer side number
---@field [4] string unit type name
---@class cost_map_options
---@field ignore_units boolean
---@field ignore_teleport boolean
---@field viewing_side integer
---@field debug boolean
---@field use_max_moves boolean
---@class cost_map
---@field [1] integer x coordinate
---@field [2] integer y coordinate
---@field [3] integer cost to reach tile
---@field [4] integer number of units who can reach tile

---Build a cost map for the unit
---@param unit location|WML
---@param types? cost_map_types
---@param options? cost_map_options
---@param filter? WML
---@return cost_map
function wesnoth.paths.find_cost_map(unit, types, options, filter) end
