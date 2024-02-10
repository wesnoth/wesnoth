---@meta

---@class stringx : stringlib
stringx = {}

---Split a string into an array
---@param str string
---@param sep? string
---@param options? {remove_empty:boolean, strip_spaces:boolean, escape:string, quote:string, quote_left:string, quote_right:string, expand_anim:boolean}
---@return string[]
function stringx.split(str, sep, options) end
string.split = stringx.split

---Split a string into a maps
---@param str string
---@param item_sep? string
---@param kv_sep? string
---@param options? {remove_empty:boolean, strip_spaces:boolean, default: string}
---@return table<string, string>
function stringx.map_split(str, item_sep, kv_sep, options) end
string.map_split = stringx.map_split

---Join a list of strings into a single string
---@param strings string[]
---@param sep string
---@return string
---@overload fun(sep:string, strings:string[]):string
function stringx.join(strings, sep) end
string.join = stringx.join

---Join a string map into a ssingle string
---@param string_map table<string,string>
---@param item_sep string
---@param kv_sep string
---@return string
---@overload fun(item_sep:string, string_map:table<string,string>, kv_sep:string):string
function stringx.join_map(string_map, item_sep, kv_sep) end
string.join_map = stringx.join_map

---Substitute variables into a format string
---@param format string|tstring
---@param values table<string, string|tstring|number>
---@return string|tstring
function stringx.vformat(format, values) end
string.vformat = stringx.vformat

---Formats a list using natural language in the form "a, b, and c"
---@param empty tstring
---@param strings tstring[]
function stringx.format_conjunct_list(empty, strings) end
string.format_conjunct_list = stringx.format_conjunct_list

---Formats a list using natural language in the form "a, b, or c"
---@param empty tstring
---@param strings tstring[]
function stringx.format_disjunct_list(empty, strings) end
string.format_disjunct_list = stringx.format_disjunct_list

---Trims leading and trailing whitespace from a string
---@param str string
---@return string
function stringx.trim(str) end
string.trim = stringx.trim

---Parses a range of the form "1-5" into endpoints
---@param range string
---@return integer start
---@return integer end
function stringx.parse_range(range) return 0,0 end
string.parse_range = stringx.parse_range

-- These are defined in lua/core/stringx.lua
string.escaped_split = stringx.escaped_split
string.quoted_split = stringx.quoted_split
string.anim_split = stringx.anim_split
string.iter_range = stringx.iter_range
string.iter_ranges = stringx.iter_ranges
