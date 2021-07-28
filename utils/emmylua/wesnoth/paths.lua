---@meta

wesnoth.paths = {}

---Find a good path between two hexes
---@alias path_options {max_cost:integer, ignore_units:boolean, ignore_teleport:boolean, viewing_side:integer, width:integer, height:integer, include_borders:boolean, calculate:fun(x:integer, y:integer, cost:integer):integer}
---@param start location
---@param finish location
---@param options path_options
---@return location[] path
---@return integer cost
---@overload fun(x1:integer, y1:integer, finish:location, options:path_options):location[],integer
---@overload fun(start:location, x2:integer, y2:integer, options:path_options):location[],integer
---@overload fun(x1:integer, y1:integer, x2:integer, y2:integer, options:path_options):location[],integer
function wesnoth.paths.find_path(start, finish, options) end

---Find a vacant hex as close as possible
---@param x integer
---@param y integer
---@param unit unit
---@return integer x, integer y
function wesnoth.paths.find_vacant_hex(x, y, unit) end

---Get all locations a unit can reach
---@param unit unit
---@param options {additional_turns:integer, ignore_units:boolean, ignore_teleport:boolean, viewing_side:integer}
---@return location[]
function wesnoth.paths.find_reach(unit, options) end

---Get all locations a unit can see
---@param unit unit
---@return location[]
function wesnoth.paths.find_vision_range(unit) end

---Build a cost map for the unit
---@param unit location|WML
---@param types? {[1]:integer, [2]:integer, [3]:integer, [4]:string} {x, y, side, unit_type}
---@param options? {ignore_units:boolean, ignore_teleport:boolean, viewing_side:integer, debug:boolean, use_max_moves:boolean}
---@param filter? WML
---@return {[1]: integer, [2]:integer, [3]:integer, [4]:integer} {x, y, cost, reach_count}
function wesnoth.paths.find_cost_map(unit, types, options, filter) end
