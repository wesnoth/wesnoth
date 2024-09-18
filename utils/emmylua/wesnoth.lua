---@meta

wesnoth = {}

---@return "'Game Lua Kernel'" | "'Mapgen Lua Kernel'" | "'Application Lua Kernel'"
function wesnoth.kernel_type() end

---@param path string
---@return ...
function wesnoth.dofile(path, ...) end

---@class tstring : string
---@operator concat(string):tstring
---@operator concat(tstring):tstring
tstring = {}

tstring.format = string.format
tstring.vformat = stringx.vformat

---Constructs a textdomain, which can be called to create translatable strings.
---@param domain string The textdomain name
---@return fun(str:string, str_plural?:string, n?:number):tstring
function wesnoth.textdomain(domain) end

---Logs a message to the console
---@param logger "'info'"|"'debug'"|"'dbg'"|"'warning'"|"'warn'"|"'wrn'"|"'error'"|"'err'"|"'wml'"
---@param message string
---@param in_chat? boolean
---@overload fun(message:string, in_chat?:boolean)
function wesnoth.log(logger, message, in_chat) end

---Simulate combat between two units
---@param attacker unit
---@param attacker_weapon integer
---@param defender unit
---@param defender_weapon integer
---@return stats_evaluation attacker_stats
---@return stats_evaluation defender_stats
---@return weapon_evaluation attacker_weapon
---@return weapon_evaluation defender_weapon
---@overload fun(attacker:unit, defender:unit):stats_evaluation,stats_evaluation,weapon_evaluation,weapon_evaluation
---@overload fun(attacker:unit, attacker_weapon:integer, defender:unit):stats_evaluation,stats_evaluation,weapon_evaluation,weapon_evaluation
---@overload fun(attacker:unit, defender:unit, defender_weapon:integer):stats_evaluation,stats_evaluation,weapon_evaluation,weapon_evaluation
function wesnoth.simulate_combat(attacker, attacker_weapon, defender, defender_weapon) end

---@class stats_evaluation
---@field poisoned number The chance to be poisoned as a number in [0,1].
---@field slowed number The chance to be slowed as a number in [0,1].
---@field untouched number The chance to receive no damage as a number in [0,1].
---@field average_hp number
---@field hp_chance table<integer, number> The chance to end the attack with a specific number of hit points. The key is the number of hit points, from 0 to max hitpoints. The value is the chance as a number in [0,1].

---@class weapon_evaluation
---@field num_blows integer The number of times the attack hits.
---@field damage integer The damage the attack inflicts per hit.
---@field chance_to_hit number The chance for the attack to hit.
---@field poisons boolean Indicates whether the attack inflicts poison.
---@field slows boolean Indicates whether the attack inflicts slow.
---@field petrifies boolean Indicates whether the attack inflicts petrification.
---@field plagues boolean Indicates whether the attack inflicts plague.
---@field plague_type string If the attack inflicts plague, this specifies the unit type it is converted to.
---@field backstabs boolean
---@field rounds integer The number of rounds of combat.
---@field firststrike boolean Indicates whether the attack always strikes first.
---@field drains boolean Indicates whether the attack drains hit points.
---@field drain_constant integer
---@field drain_percent number
---@field number integer
---@field name string

---Construct a function that generates random names using a context-free grammar
---@param type '"cfg"' The type of generator to create
---@param definition table<string, string|string[]> Definition rules for the generator
---@return fun():string
function wesnoth.name_generator(type, definition) end

---Construct a function that generates random names using Markov chains
---@param type '"markov"' The type of generator to create
---@param definition string[] Sample names to be input to the generator
---@param chain_size? integer The size of the Markov chain unit
---@param max_length? integer Maximum length of the generated names
---@return fun():string
function wesnoth.name_generator(type, definition, chain_size, max_length) end

---@alias formula fun(variables:WML):any

---Compile a WFL formula into a Lua function
---@param formula string A WFL formula
---@return formula
function wesnoth.compile_formula(formula) end

---Evaluate a WFL formula
---@param formula string|formula A WFL formula
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

---Parse a version
---@param str string A version string to parse
---@return version
---@overload fun(major:integer, minor?:integer, revision?:integer, suffix?:string):version
function wesnoth.version(str) end

---Parse a version
---@param str string A version string to parse
---@return version
function wesnoth.version(str) end
---Construct a version
---@param major integer The major revision number
---@param minor? integer The minor revision number
---@param revision? integer The patch revision number
---@param suffix string The suffix, including the separator character if any
---@return version
function wesnoth.version(major, minor, revision, suffix) end

---@return version #The current version of Wesnoth
function wesnoth.current_version() end

---@return integer #The number of milliseconds since Wesnoth launched
function wesnoth.ms_since_init() end

---Output a deprecated message
---@param element_name string The name of the element being deprecated
---@param level 1|2|3|4 The deprecation level
---@param version string|nil The earliest version the element may be removed in
---@param detail_message string An additional message describing the deprecation and usually indicating a replacement
function wesnoth.deprecated_message(element_name, level, version, detail_message) end

---@type table<string, fun(cfg:WML)>
wesnoth.wml_actions = {}

---@type table<string, fun(cfg:WML):boolean>
wesnoth.wml_conditionals = {}

---@type table<string, fun(unit:unit, cfg:WML)>
wesnoth.effects = {}

---@type table<string, fun(cfg:WML)>
wesnoth.custom_synced_commands = {}

---@class color
---@field r integer
---@field g integer
---@field b integer
---@field a integer
---@class color_list
---@field [1] integer
---@field [2] integer
---@field [3] integer
---@field [4] integer?
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
---@field damage_inflicted integer
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

---@class color_palette : color[]
---@field name string

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
---@field red_green_scale color_palette
---@field red_green_scale_text color_palette
---@field blue_white_scale color_palette
---@field blue_white_scale_text color_palette
---@field palettes table<string, color_palette>
wesnoth.game_config = {}

---@type table<string, unit_race>
wesnoth.races = {}

---Holds information about the current scenario
---@class scenario
---@field turns integer
---@field next string|nil
---@field id string
---@field name tstring
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
