---@meta

---@class stringx : stringlib
stringx = {}

string = stringx

---Split a string into an array
---@param str string
---@param sep? string
---@param options? {remove_empty:boolean, strip_spaces:boolean, escape:string, quote:string, quote_left:string, quote_right:string, expand_anim:boolean}
---@return string[]
function stringx.split(str, sep, options) end

---Split a string into a maps
---@param str string
---@param item_sep? string
---@param kv_sep? string
---@param options? {remove_empty:boolean, strip_spaces:boolean, default: string}
---@return table<string, string>
function stringx.map_split(str, item_sep, kv_sep, options) end

---Join a list of strings into a single string
---@param strings string[]
---@param sep string
---@return string
---@overload fun(sep:string, strings:string[]):string
function stringx.join(strings, sep) end

---Join a string map into a ssingle string
---@param string_map table<string,string>
---@param item_sep string
---@param kv_sep string
---@return string
---@overload fun(item_sep:string, string_map:table<string,string>, kv_sep:string):string
function stringx.join_map(string_map, item_sep, kv_sep) end

---Substitute variables into a format string
---@param format string|tstring
---@param values table<string, string|tstring>
---@return string|tstring
function stringx.vformat(format, values) end

---Formats a list using natural language in the form "a, b, and c"
---@param empty tstring
---@param strings tstring[]
function stringx.format_conjunct_list(empty, strings) end

---Formats a list using natural language in the form "a, b, or c"
---@param empty tstring
---@param strings tstring[]
function stringx.format_disjunct_list(empty, strings) end

---Trims leading and trailing whitespace from a string
---@param str string
---@return string
function stringx.trim(str) end

---Parses a range of the form "1-5" into endpoints
---@param range string
---@return integer start
---@return integer end
function stringx.parse_range(range) return 0,0 end
