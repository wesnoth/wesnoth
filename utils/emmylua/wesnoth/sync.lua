---@meta

---Evaluate an expression on one client and synchronize the results to all clients
---@param description? tstring
---@param fcn fun():WMLTable
---@param ai_fcn? fun():WMLTable
---@param for_side? integer[]
---@return WMLTable
---@overload fun(fcn: (fun():WMLTable), ai_fcn?: (fun():WMLTable), for_side?:integer[]):WMLTable
function wesnoth.sync.evaluate_single(description, fcn, ai_fcn, for_side) end

---Evaluate an expression on multiple clients and synchronize the results to all clients
---@param description? tstring
---@param fcn fun():WMLTable
---@param default_fcn? fun():WMLTable
---@param for_sides? integer[]
---@return table<integer, WMLTable>
function wesnoth.sync.evaluate_multiple(description, fcn, default_fcn, for_sides) end

---Run code in an unsynced context
---@param fcn fun()
function wesnoth.sync.run_unsynced(fcn) end

---Invoke a defined synced commands
---@param cmd string
---@param cfg WMLTable
function wesnoth.sync.invoke_command(cmd, cfg) end
