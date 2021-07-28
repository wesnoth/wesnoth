---@meta

wesnoth = {}

---@return "'Game Lua Kernel'" | "'Mapgen Lua Kernel'" | "'Application Lua Kernel'"
function wesnoth.kernel_type() end

---@param path string
---@return ...
function wesnoth.dofile(path, ...) end

---@class tstring : string

---Constructs a textdomain, which can be called to create translatable strings.
---@param domain string The textdomain name
---@return fun(str:string, str_plural?:string, n?:number):tstring
function wesnoth.textdomain(domain) end

---Logs a message to the console
---@param logger "'info'"|"'debug'"|"'warning'"|"'error'"|"'wml'"
---@param message string
---@param in_chat boolean
---@overload fun(message:string, in_chat:boolean)
function wesnoth.log(logger, message, in_chat) end

---Simulate combat between two units_MP
---@param attacker unit
---@param attacker_weapon integer
---@param defender unit
---@param defender_weapon integer
---@return stats_evaluation attacker_stats
---@return stats_evaluation defender_stats
---@return weapon_evaluation attacker_weapon
---@return weapon_evaluation defender_weapon
function wesnoth.simulate_combat(attacker, attacker_weapon, defender, defender_weapon) end

---@class stats_evaluation
---@field poisoned number
---@field slowed number
---@field untouched number
---@field average_hp number
---@field hp_chance number[]

---@class weapon_evaluation
---@field num_blows integer
---@field damage integer
---@field chance_to_hit number
---@field poisons boolean
---@field slows boolean
---@field petrifies boolean
---@field plagues boolean
---@field plague_type string
---@field backstabs boolean
---@field rounds integer
---@field firststrike boolean
---@field drains boolean
---@field drain_constant integer
---@field drain_percent number
---@field attack_num integer
---@field name string

---Construct a function that generates random namespace
---@param type '"cfg"' The type of generator to create
---@param definition table<string, string|string[]> Definition rules for the generator
---@param chain_size integer The size of the Markov chain unit
---@param max_length integer Maximum length of the generated names
---@return fun():string
---@overload fun(type:'"markov"', definition: string[], chain_size?: integer, max_length?: integer) : fun():string
function wesnoth.name_generator(type, definition) end

---Compile a WFL formula into a Lua function
---@param formula string A WFL formula
---@return fun(variables:WML):any
function wesnoth.compile_formula(formula, variables) end

---Evaluate a WFL formula
---@param formula string A WFL formula
---@param variables WML Table defining WFL variables.
---@return any
function wesnoth.eval_formula(formula, variables) end

---Represents an arbitrary version number.
---@class version
---@field major integer The major revision number (index 1)
---@field minor integer The minor revision number (index 2)
---@field revision integer The patch revision number (index 3)
---@field is_canonical boolean Whether the version has at most three components
---@field sep string|nil The character separating the suffix from the main version
---@field special string The version suffix

---Construct a version
---@param str string A version string to parse
---@param major integer The major revision number
---@param minor? integer The minor revision number
---@param revision? integer The patch revision number
---@param suffix string The suffix, including the separator character if any
---@return version
---@overload fun(major:integer, minor?:integer, revision?:integer, suffix?:string):version
function wesnoth.version(str) end

---@return version #The current version of Wesnoth
function wesnoth.current_version() end

---@return integer #The number of milliseconds since Wesnoth launched
function wesnoth.ms_since_init() end

---Output a deprecated message
---@param element_name string The name of the element being deprecated
---@param level '1'|'2'|'3'|'4' The deprecation level
---@param version string|nil The earliest version the element may be removed in
---@param detail_message An additional message describing the deprecation and usually indicating a replacement
function wesnoth.deprecated_message(element_name, level, version, detail_message) end

---@type table<string, fun(cfg:WML)>
wesnoth.wml_actions = {}

---@type table<string, fun(cfg:WML):boolean
wesnoth.wml_conditionals = {}

---@type table<string, fun(unit:unit, cfg:WML)
wesnoth.effects = {}

---@type table<string, fun(cfg:WML)
wesnoth.custom_synced_commands = {}

---@class color
---@field r
---@field g
---@field b
---@field a
---@class color_range
---@field mid color
---@field min color
---@field max color
---@field minimap color
---@field pango_color string

---@type table<string, color_range>
wesnoth.colors = {}

---@class event_context
---@field name string
---@field id string
---@field weapon WMLTable
---@field second_weapon WMLTable
---@field damage_inflicted interger
---@field x1 integer
---@field y1 integer
---@field x2 integer
---@field y2 integer
---@field unit_x integer
---@field unit_y integer

---@alias synced_state
---| "'synced'"
---| "'unsynced'"
---| "'local_choice'"
---| "'preload'"

---Holds information about the current game state
---@class current
---@field side integer Number of the side that currently has control
---@field turn integer Number of the current turn
---@field synced_state synced_state
---@field user_can_invoke_commands boolean
---@field map terrain_map
---@field schedule schedule
---@field event_context event_context
wesnoth.current = {}

---Holds global game configuration options
---@class game_config
---@field base_income integer
---@field village_income integer
---@field village_support integer
---@field poison_amount integer
---@field rest_heal_amount integer
---@field recall_cost integer
---@field combat_experience integer
---@field kill_experience integer
---@field global_traits table<string, WMLTable>
---@field do_healing boolean
---@field theme string
---@field debug boolean
---@field debug_lua boolean
---@field mp_debug boolean
---@field strict_lua boolean
wesnoth.game_config = {}

---@type table<string, unit_race>
wesnoth.races = {}

---Holds information about the current scenario
---@class scenario
---@field turns integer
---@field next string|nil
---@field id string
---@field defeat_music string[]
---@field victory_music string[]
---@field show_credits boolean
---@field end_text tstring
---@field end_text_duration number
---@field difficulty string
---@field type "'scenario'"|"'test'"|"'multiplayer'"
---@field era WMLTable|nil
---@field campaign WMLTable|nil
---@field resources WMLTable[]
---@field modifications WMLTable[]
---@field mp_settings mp_settings
wesnoth.scenario = {}

---@class mp_settings
---@field game_name string
---@field hash string
---@field experience_modifier integer
---@field mp_countdown boolean
---@field mp_countdown_init_time integer
---@field mp_countdown_turn_bonus integer
---@field mp_countdown_reservoir_bonus integer
---@field mp_countdown_action_bonus integer
---@field mp_village_gold integer
---@field mp_village_support integer
---@field mp_fog boolean
---@field mp_shroud boolean
---@field mp_use_map_settings boolean
---@field mp_random_start_time boolean
---@field allow_observers boolean
---@field private_replay boolean
---@field shuffle_sides boolean
---@field random_faction_mode "'Independent'"|"'No Mirror'"|"'No Ally Mirror'"
---@field options WMLTable
---@field savegame 'false'|"'midgame'"|"'scenario_start'"
---@field side_players table<string, string>
---@field addons mp_addon[]
---@class mp_addon
---@field id string
---@field name string
---@field required boolean
---@field min_version? string
---@field version? string
---@field content {id:string, name:string, type:string}

---Information on a specific terrain type
---@class terrain_info
---@field id string
---@field name tstring
---@field editor_name tstring
---@field description tstring
---@field icon string
---@field editor_image string
---@field light integer
---@field village boolean
---@field castle boolean
---@field keep boolean
---@field healing boolean

---@type table<string, terrain_info>
wesnoth.terrain_types = {}

---@type table<string, unit_type>
wesnoth.unit_types = {}

---@generic T
---@param values T[]
---@param names string[]
---@return table<string, T>
function wesnoth.named_tuple(values, names) end
