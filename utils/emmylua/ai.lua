---@meta

---@class ailib
---@field read_only boolean
---@field side integer
ai = {}

---@alias ai_aspects_recruitment_more string|integer

---@class ai.aspects
---@field aggression number
---@field caution number
---@field grouping string
---@field leader_aggression number
---@field leader_ignores_keep boolean|string
---@field leader_value number
---@field passive_leader boolean|string[]
---@field passive_leader_shares_keep boolean|string[]
---@field recruitment_diversity number
---@field recruitment_randomness number
---@field retreat_enemy_weight number
---@field retreat_factor number
---@field scout_village_targeting number
---@field simple_targeting boolean
---@field support_villages boolean
---@field village_value number
---@field villages_per_scout integer
---@field advancements table<integer, string[]>
---@field attacks {own:unit[], enemy:unit[]}
---@field avoid location[]
---@field leader_goal WMLTable
---@field recruitment_instructions WMLTable
---@field recruitment_more ai_aspects_recruitment_more[]
---@field recruitment_pattern string[]
---@field recruitment_save_gold WMLTable
ai.aspects = {}

---@class ai_attack_analysis
---@field rating fun():number
---@field movements {src:location, dst:location}[]
---@field target location
---@field target_value number
---@field avg_losses number
---@field chance_to_kill number
---@field avg_damage_inflicted number
---@field target_starting_damage integer
---@field avg_damage_taken number
---@field resources_used number
---@field terrain_quality number
---@field alternative_terrain_quality number
---@field vulnerability number
---@field support number
---@field leader_thread boolean
---@field uses_leader boolean
---@field is_surrounded boolean
---@return ai_attack_analysis[]
function ai.get_attacks() end

---@class ai_target
---@field loc location
---@field type ai_target_type
---@field value number
---@alias ai_target_type
---| "'village'"
---| "'leader'"
---| "'explicit'"
---| "'thread'"
---| "'battle aid'"
---| "'mass'"
---| "'support'"
---@return ai_target[]
function ai.get_targets() end

---Get the location of a suitable keep near the given unit
---@param unit unit
---@return integer x
---@return integer y
function ai.suitable_keep(unit) end

---The result of an AI action
---@class ai_result
---@field gamestate_changed boolean
---@field ok boolean
---@field status number
---@field result string

---Attack a unit
---@param attacker location
---@param defender location
---@param weapon? integer
---@param aggression? number
---@return ai_result
---@overload fun(attacker:location, defender_x:integer, defender_y:integer, weapon?:integer, aggression?:number):ai_result
---@overload fun(attacker_x:integer, attacker_y:integer, defender:location, weapon?:integer, aggression?:number):ai_result
---@overload fun(attacker_x:integer, attacker_y:integer, defender_x:integer, defender_y:integer, weapon?:integer, aggression?:number):ai_result
function ai.attack(attacker, defender, weapon, aggression) end

---Check if attacking a unit is possible
---@param attacker location
---@param defender location
---@param weapon? integer
---@param aggression? number
---@return ai_result
---@overload fun(attacker:location, defender_x:integer, defender_y:integer, weapon?:integer, aggression?:number):ai_result
---@overload fun(attacker_x:integer, attacker_y:integer, defender:location, weapon?:integer, aggression?:number):ai_result
---@overload fun(attacker_x:integer, attacker_y:integer, defender_x:integer, defender_y:integer, weapon?:integer, aggression?:number):ai_result
function ai.check_attack(attacker, defender, weapon, aggression) end

---Move a unit
---@param unit location
---@param to location
---@return ai_result
---@overload fun(unit:unit, to_x:integer, to_y:integer):ai_result
---@overload fun(unit_x:integer, unit_y:integer, to:location):ai_result
---@overload fun(unit_x:integer, unit_y:integer, to_x:integer, to_y:integer):ai_result
function ai.move(unit, to) end

---Move a unit and set its remaining moves to 0
---@param unit unit
---@param to location
---@return ai_result
---@overload fun(unit:unit, to_x:integer, to_y:integer):ai_result
---@overload fun(unit_x:integer, unit_y:integer, to:location):ai_result
---@overload fun(unit_x:integer, unit_y:integer, to_x:integer, to_y:integer):ai_result
function ai.move_full(unit, to) end

---Check if moving a unit is possible
---@param unit unit
---@param to location
---@return ai_result
---@overload fun(unit:unit, to_x:integer, to_y:integer):ai_result
---@overload fun(unit_x:integer, unit_y:integer, to:location):ai_result
---@overload fun(unit_x:integer, unit_y:integer, to_x:integer, to_y:integer):ai_result
function ai.check_move(unit, to) end

---Recall a unit
---@param unit_id string
---@param at? location
---@return ai_result
---@overload fun(unit_id:string, x:integer, y:integer):ai_result
function ai.recall(unit_id, at) end

---Check if recalling a unit is possible
---@param unit_id string
---@param at? location
---@return ai_result
---@overload fun(unit_id:string, x:integer, y:integer):ai_result
function ai.check_recall(unit_id, at) end

---Recruit a unit
---@param unit_type string
---@param at? location
---@return ai_result
---@overload fun(unit_type:string, x:integer, y:integer):ai_result
function ai.recruit(unit_type, at) end

---Check if recruiting a unit is possible
---@param unit_type string
---@param at? location
---@return ai_result
---@overload fun(unit_type:string, x:integer, y:integer):ai_result
function ai.check_recruit(unit_type, at) end

---Set a unit's attacks left to 0
---@param unit location
---@return ai_result
---@overload fun(unit_x:integer, unit_y:integer):ai_result
function ai.stopunit_attacks(unit) end

---Set a unit's remaining moves to 0
---@param unit location
---@return ai_result
---@overload fun(unit_x:integer, unit_y:integer):ai_result
function ai.stopunit_moves(unit) end

---Set a unit's remaining moves and attacks left to 0
---@param unit location
---@return ai_result
---@overload fun(unit_x:integer, unit_y:integer):ai_result
function ai.stopunit_all(unit) end

---Check if setting a units remaining moves and/or attacks left to 0 is possible
---@param unit location
---@return ai_result
---@overload fun(unit_x:integer, unit_y:integer):ai_result
function ai.check_stopunit(unit) end

---Give control to a human player for the rest of the turn
function ai.fallback_human() end

---@alias move_map table<integer, location[]>
---@return move_map
function ai.get_dst_src() end
---@return move_map
function ai.get_src_dst() end
---@return move_map
function ai.get_enemy_dst_src() end
---@return move_map
function ai.get_enemy_src_dst() end
