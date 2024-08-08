---@meta

---@class wesnoth.audio
---@field volume integer
wesnoth.audio = {}

---Play a sound
---@param sound string
---@param repeats? integer
function wesnoth.audio.play(sound, repeats) end

---@class music_track
---@field once boolean
---@field ms_before integer
---@field ms_after integer
---@field name string
---@field title tstring
---@field __cfg WMLTable

---@class wesnoth.audio.music_list : music_track[]
---@field current music_track
---@field current_i integer|nil
---@field previous music_track
---@field volume number
---@field all WMLTable[]
wesnoth.audio.music_list = {}

---Add a new track to the player_list
---@param track string
---@param immediate? boolean
---@param ms_before? integer
---@param ms_after? integer
function wesnoth.audio.music_list.add(track, immediate, ms_before, ms_after) end

---Remove tracks from the playlist
---@vararg integer
function wesnoth.audio.music_list.remove(...) end

---Clear the playlist
function wesnoth.audio.music_list.clear() end

---Play a track without adding it to the playlist
---@param track string
function wesnoth.audio.music_list.play(track) end

---@class sound_source
---@field id string
---@field sounds string[]
---@field delay integer
---@field chance integer
---@field loop integer
---@field range integer
---@field fade_range integer
---@field check_shrouded boolean
---@field check_fogged boolean
---@field locations location[]
---@field __cfg WMLTable

---@type table<string, sound_source>
wesnoth.audio.sources = {}
