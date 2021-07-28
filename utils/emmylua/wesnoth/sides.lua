---@meta

---Defines a single side in a scenario
---@class side : wesnoth.sides
---@param side integer
---@param side_name tstring
---@param saveid string
---@param flag string
---@param flag_icon string
---@param color string
---@param user_team_name tstring
---@param team_name string
---@param controller string
---@param is_local boolean
---@param gold integer
---@param village_gold integer
---@param village_support integer
---@param recall_cost integer
---@param base_income integer
---@param num_villages integer
---@param num_units integer
---@param total_upkeep integer
---@param expenses integer
---@param net_income integer
---@param total_income integer
---@param carryover_bonus integer
---@param carryover_percent number
---@param carryover_add boolean
---@param fog boolean
---@param shroud boolean
---@param share_vision "'all'"|"'shroud'"|"'none'"
---@param hidden boolean
---@param scroll_to_leader boolean
---@param suppress_end_turn_confirmation boolean
---@param persistent boolean
---@param defeat_condition string
---@param starting_location location
---@param recruit string[]
---@param faction string
---@param faction_name tstring
---@param chose_random boolean
---@param lost boolean
---@param variables table<string, WML>
---@param __cfg WMLTable

---@class wesnoth.sides
wesnoth.sides = {}

---Check if two sides are enemies
---@param side1 integer|side
---@param side2 integer|side
---@return boolean
function wesnoth.sides.is_enemy(side1, side2) end

---Check if a side matches a filter
---@param side integer|string
---@param filter WML
---@return boolean
function wesnoth.sides.matches(side, filter) end

---Set the flag and team color of the side
---@param side integer|side
---@param flag string
---@param color string
function wesnoth.sides.set_id(side, flag, color) end

---Place fog for one or more sides
---@param sides? integer|side|integer[]
---@param locations location[]
---@param permanent? boolean
function wesnoth.sides.place_fog(sides, locations, permanent) end

---Remove fog for one or more sides
---@param sides? integer|side|integer[]
---@param locations location[]
---@param permanent? boolean
function wesnoth.sides.remove_fog(sides, locations, permanent) end

---Check if a hex is fogged for the given side
---@param side integer|side
---@param location location
---@return boolean
---@overload fun(side:integer|side, x:integer, y:integer):boolean
function wesnoth.sides.is_fogged(side, location) end

---Place shroud for the given side
---@param side integer|side
---@param locations location[]
function wesnoth.sides.place_shroud(side, locations) end

---Replace the shroud for the given side
---@param side integer|side
---@param locations location[]
function wesnoth.sides.override_shroud(side, locations) end

---Remove shroud for the given side
---@param side integer|side
---@param locations location[]
function wesnoth.sides.remove_shroud(side, locations) end

---Check if a hex is shrouded for a given side
---@param side integer|side
---@param location location
---@return boolean
---@overload fun(side:integer|side, x:integer, y:integer):boolean
function wesnoth.sides.is_shrouded(side, location) end

---Replace the AI for the given side
---@param side integer|side
---@param file string
function wesnoth.sides.switch_ai(side, file) end

---Add AI parameters for the given side
---@param side integer|side
---@param params WML
function wesnoth.sides.append_ai(side, params) end

---Add a new component to the given side's AI
---@param side integer|side
---@param path string
---@param component WML
function wesnoth.sides.add_ai_component(side, path, component) end

---Change a component of the given side's AI
---@param side integer|side
---@param path string
---@param component WML
function wesnoth.sides.change_ai_component(side, path, component) end

---Remove a component from the given side's AI
---@param side integer|side
---@param path string
---@param component WML
function wesnoth.sides.delete_ai_component(side, path, component) end

---Get a specific side
---@param number integer
---@return side
function wesnoth.sides.get(number) end

---Get all sides matching a filter
---@param filter WML
---@return side[]
function wesnoth.sides.find(filter) end

---Add a new side to the end of the list
---@return integer #The number of the new side
function wesnoth.sides.create() end

---Get debug information for the given side
---@param side integer
---@return debug_ai_info
function wesnoth.sides.debug_ai(side) end

---@class debug_ai_info
---@field ai ailib
---@field components ai_component[]
---@field engine WMLTable
---@field params WMLTable
---@field data WMLTable
---@field update_self fun(params:WMLTable, data:WMLTable)
---@class ai_component
---@field engine string
---@field id string
---@field name string
---@class ai_engine : ai_component
---@field stage ai_component[]
---@class ai_stage : ai_component
---@field candidate_action ai_component[]
---@field stg_ptr lightuserdata
---@field exec fun()
---@class ai_candidate_action : ai_component
---@field ca_ptr lightuserdata
---@field exec fun()
