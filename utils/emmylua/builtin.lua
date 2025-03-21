---@meta

-- This file covers the built-in OS and debug modules, or rather, the parts of them
-- that exist in Wesnoth.

std_print = print
os = {}
debug = {}

--[========[OS library - truncated]========]
--(copy-pasted from the plugin's internal meta files)

---
---Returns an approximation of the amount in seconds of CPU time used by the program.
---
---[View documents](command:extension.lua.doc?["en-us/54/manual.html/pdf-os.clock"])
---
---@return number
function os.clock() end

---@class osdate
---
---four digits
---
---[View documents](command:extension.lua.doc?["en-us/54/manual.html/pdf-osdate.year"])
---
---@field year  integer
---
---1-12
---
---[View documents](command:extension.lua.doc?["en-us/54/manual.html/pdf-osdate.month"])
---
---@field month integer
---
---1-31
---
---[View documents](command:extension.lua.doc?["en-us/54/manual.html/pdf-osdate.day"])
---
---@field day   integer
---
---0-23
---
---[View documents](command:extension.lua.doc?["en-us/54/manual.html/pdf-osdate.hour"])
---
---@field hour  integer
---
---0-59
---
---[View documents](command:extension.lua.doc?["en-us/54/manual.html/pdf-osdate.min"])
---
---@field min   integer
---
---0-61
---
---[View documents](command:extension.lua.doc?["en-us/54/manual.html/pdf-osdate.sec"])
---
---@field sec   integer
---
---weekday, 1–7, Sunday is 1
---
---[View documents](command:extension.lua.doc?["en-us/54/manual.html/pdf-osdate.wday"])
---
---@field wday  integer
---
---day of the year, 1–366
---
---[View documents](command:extension.lua.doc?["en-us/54/manual.html/pdf-osdate.yday"])
---
---@field yday  integer
---
---daylight saving flag, a boolean
---
---[View documents](command:extension.lua.doc?["en-us/54/manual.html/pdf-osdate.isdst"])
---
---@field isdst boolean

---
---Returns a string or a table containing date and time, formatted according to the given string `format`.
---
---[View documents](command:extension.lua.doc?["en-us/54/manual.html/pdf-os.date"])
---
---@param format? string
---@param time?   integer
---@return string|osdate
function os.date(format, time) end

---
---Returns the difference, in seconds, from time `t1` to time `t2`.
---
---[View documents](command:extension.lua.doc?["en-us/54/manual.html/pdf-os.difftime"])
---
---@param t2 integer
---@param t1 integer
---@return integer
function os.difftime(t2, t1) end

---
---Returns the current time when called without arguments, or a time representing the local date and time specified by the given table.
---
---[View documents](command:extension.lua.doc?["en-us/54/manual.html/pdf-os.time"])
---
---@param date? osdate
---@return integer
function os.time(date) end

--[========[Debug library - truncated]========]
--(copy-pasted from the plugin's internal meta files)

---
---Returns a string with a traceback of the call stack. The optional message string is appended at the beginning of the traceback.
---
---[View documents](command:extension.lua.doc?["en-us/54/manual.html/pdf-debug.traceback"])
---
---@param thread   thread
---@param message? any
---@param level?   integer
---@return string  message
function debug.traceback(thread, message, level) end

