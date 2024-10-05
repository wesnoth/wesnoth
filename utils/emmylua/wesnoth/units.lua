---@meta

---Defines a weapon possessed by a unit
---@class unit_weapon
---@field read_only boolean
---@field description tstring
---@field name string
---@field type string
---@field icon string
---@field range integer
---@field alignment string
---@field number integer
---@field movement_used integer
---@field attacks_used integer
---@field attack_weight number
---@field defense_weight number
---@field accuracy integer
---@field parry integer
---@field max_range integer
---@field min_range integer
---@field specials WMLTable
---@field __cfg WMLTable

---Defines a unit type
---@class unit_type
---@field name tstring
---@field id string
---@field race string
---@field image string
---@field icon string
---@field profile string
---@field small_profile string
---@field max_hitpoints integer
---@field max_moves integer
---@field max_experience integer
---@field cost integer
---@field level integer
---@field recall_cost integer
---@field advances_to string[]
---@field advances_from string[]
---@field traits table<string, WMLTable>
---@field abilities string[]
---@field attacks unit_weapon[]
---@field variations table<string, unit_type>
---@field __cfg WMLTable

---Defines a unit race
---@class unit_race
---@field description tstring
---@field name tstring
---@field plural_name tstring
---@field num_traits integer
---@field ignore_global_traits boolean
---@field undead_variation string
---@field traits table<string, WMLTable>
---@field male_name_gen fun():string
---@field female_name_gen fun():string
---@field __cfg WMLTable

---Defines an individual unit
---@class unit : wesnoth.units, location
---@field valid "'map'"|"'recall'"|"'private'"|nil
---@field loc location
---@field id string
---@field side integer
---@field type string
---@field variation string
---@field gender "'male'"|"'female'"
---@field race string
---@field portrait string
---@field image_mods string
---@field ellipse string
---@field halo string
---@field hidden boolean
---@field name tstring
---@field description tstring
---@field facing direction
---@field overlays string[]
---@field hitpoints integer
---@field max_hitpoints integer
---@field experience integer
---@field max_experience integer
---@field moves integer
---@field max_moves integer
---@field attacks_left integer
---@field max_attacks integer
---@field level integer
---@field recall_cost integer
---@field cost integer
---@field canrecruit boolean
---@field zoc boolean
---@field alignment string
---@field upkeep integer|"'full'"|"'free'"|"'loyal'"
---@field usage string
---@field renamable boolean
---@field undead_variation string
---@field role string
---@field resting boolean
---@field recall_filter WML
---@field extra_recruit string[]
---@field advances_to string[]
---@field advancements WMLTable[]
---@field status table<string, boolean>
---@field variables WMLVariableProxy
---@field attacks unit_weapon[]
---@field traits string[]
---@field abilities string[]
---@field animations string[]
---@field __cfg WMLTable

---@class wesnoth.units
wesnoth.units = {}

---Advances the unit if has enough XP_attack
---@param unit unit
---@param animate? boolean
---@param fire_events? boolean
function wesnoth.units.advance(unit, animate, fire_events) end

---Creates a copy of the unit
---@param unit unit
---@return unit
function wesnoth.units.clone(unit) end

---Erases the unit from the map
---@param unit unit
---@overload fun(loc:location)
---@overload fun(x:integer, y:integer)
function wesnoth.units.erase(unit) end

---Extracts the unit from the map, making it a private unit
---@param unit unit
function wesnoth.units.extract(unit) end

---Tests if the unit matches a filter
---@param unit unit The unit to test
---@param filter WML The filter to match
---@param context? location|unit A second unit for the filter, or a reference location
function wesnoth.units.matches(unit, filter, context) end

---Place or move a unit on the map
---@param unit unit|WML
---@param loc? location
---@param fire_event? boolean
---@overload fun(unit:unit|WML, x:integer, y:integer)
---@overload fun(unit:unit|WML, x:integer, y:integer, fire_event:boolean)
---@overload fun(unit:unit|WML, fire_event:boolean)
function wesnoth.units.to_map(unit, loc, fire_event) end

---Place a unit on a recall lists
---@param unit unit|WML
---@param side? integer
function wesnoth.units.to_recall(unit, side) end

---Transform a unit into a different type
---@param unit unit
---@param to_type string
---@param to_variation? string
function wesnoth.units.transform(unit, to_type, to_variation) end

---Select the unit, as if it had been clicked with the mouse
---@param unit unit
---@param highlight? boolean
---@param fire_event? boolean
---@overload fun(unit_x:integer, unit_y:integer, highlight?:boolean, fire_event?:boolean)
function wesnoth.units.select(unit, highlight, fire_event) end

---Test if the unit is affected by an ability
---@param unit unit
---@param tag_name string
---@return boolean
function wesnoth.units.ability(unit, tag_name) end

---Get the defense of a unit on a particular terrain
---@param unit unit
---@param terrain string
---@return number
function wesnoth.units.defense_on(unit, terrain) end

---Get the resistance of a unit againt an attack type
---@param unit unit The unit to check
---@param damage_type string The damage type
---@param as_attacker? boolean Whether to consider the unit as the attacker or the defender
---@param location? location Calculate as if the unit is on this location
---@return number
---@overload fun(unit:unit, damage_type:string, as_attacker?:boolean, x:integer, y:integer)
function wesnoth.units.resistance_against(unit, damage_type, as_attacker, location) end

---Get the movement cost of a unit on a particular terrain
---@param unit unit
---@param terrain string
---@return number
function wesnoth.units.movement_on(unit, terrain) end

---Get the vision cost of a unit on a particular terrain
---@param unit unit
---@param terrain string
---@return number
function wesnoth.units.vision_on(unit, terrain) end

---Get the jamming cost of a unit on a particular terrain
---@param unit unit
---@param terrain string
---@return number
function wesnoth.units.jamming_on(unit, terrain) end

---@alias modification_type
---| "'trait'"
---| "'object'"
---| "'advancement'"

---Add a modification to a unit using EffectWML
---@param unit unit
---@param type modification_type
---@param effects WML
---@param write_to_mods? boolean
function wesnoth.units.add_modification(unit, type, effects, write_to_mods) end

---Remove modifications matching a WML filter
---@param unit unit
---@param filter WML
---@param type? modification_type
function wesnoth.units.remove_modifications(unit, filter, type) end

---@class unit_animator
local animator = {}

---Run the animation
function animator:run() end

---Clear all units added to the animation, resetting it to a blank slate
function animator:clear() end

---@class unit_animator_params
---@field facing? location
---@field value? number|number[]
---@field with_bars? boolean
---@field text? tstring|number
---@field color? color|color_list
---@field primary? unit_weapon
---@field secondary? unit_weapon

---Add a unit to the animation
---@param unit unit
---@param flag string
---@param hits "'hit'"|"'miss'"|"'kill'"
---@param params unit_animator_params
function animator:add(unit, flag, hits, params) end

---@return unit_animator
function wesnoth.units.create_animator() end

---Create a new private unit
---@param cfg WML
---@return unit
function wesnoth.units.create(cfg) end

---Find all matching units on the map
---@param filter WML The filter to match
---@param context? location|unit A second unit for the filter, or a reference location
---@return unit[]
function wesnoth.units.find_on_map(filter, context) end


---Find all matching units on recall lists
---@param filter WML The filter to match
---@return unit[]
function wesnoth.units.find_on_recall(filter) end

---Get a specific unit by ID or location
---@param id string
---@return unit
---@overload fun(loc:location):unit
---@overload fun(x:integer, y:integer):unit
function wesnoth.units.get(id) end

---Create a weapon not attached to any unit
---@param cfg WMLTable
function wesnoth.units.create_weapon(cfg) end

---Teleports a unit to a specified location, potentially with animation.
---If the target location is occupied, a nearby location is selected.
---If no valid location can be found, the unit is not teleported.
---@param unit unit The unit to teleport
---@param target location The target location
---@param ignore_passability boolean
---@param clear_shroud boolean
---@param animate boolean
---@overload fun(unit:unit, x:integer, y:integer, ignore_passability:boolean, clear_shroud:boolean, animate:boolean)
function wesnoth.units.teleport(unit, target, ignore_passability, clear_shroud, animate) end

wesnoth.units.get_hovered = wesnoth.interface.get_displayed_unit
