local on_event = wesnoth.require("on_event")

local wc2_utils = {}

function wc2_utils.split_to_array(s, res)
	res = res or {}
	for part in tostring(s or ""):gmatch("[^%s,][^,]*") do
		table.insert(res, part)
	end
	return res
end

function wc2_utils.split_to_set(s, res)
	res = res or {}
	for part in tostring(s or ""):gmatch("[^%s,][^,]*") do
		res[part] = true
	end
	return res
end

function wc2_utils.set_to_array(s, res)
	res = res or {}
	for k,v in pairs(s) do
		table.insert(res, k)
	end
	table.sort( res )
	return res
end


function wc2_utils.remove_dublicates(t)
	local found = {}
	for i = #t, 1, -1 do
		local v = t[i]
		if found[v] then
			table.remove(t, i)
		else
			found[v] = true
		end
	end
end

--comma seperated list
function wc2_utils.pick_random(str, generator)
	local s2 = wml.variables[str]
	if s2 ~= nil or generator then
		local array = s2 and wc2_utils.split_to_array(s2) or {}
		if #array == 0 and generator then
			array = generator()
		end
		local index  = wesnoth.random(#array)
		local res = array[index]
		table.remove(array, index)
		wml.variables[str] = table.concat(array, ",")
		return res
	end
end

local function filtered_from_array(array, filter)
	local possible_indicies = {}
	for i, v in ipairs(array) do
		if filter(v) then
			table.insert(possible_indicies, i)
		end
	end
	if #possible_indicies == 0 then
		return nil
	end
	local index  = possible_indicies[wesnoth.random(#possible_indicies)]
	return index
end

function wc2_utils.pick_random_filtered(str, generator, filter)
	local s2 = wml.variables[str]
	if s2 == nil and generator == nil then
		return
	end

	local array = s2 and wc2_utils.split_to_array(s2) or {}
	if #array == 0 and generator then
		array = generator()
	end
	local index  = filtered_from_array(array, filter)
	if index == nil then
		array = generator()
		index = filtered_from_array(array, filter)
	end
	local res = array[index]
	table.remove(array, index)
	wml.variables[str] = table.concat(array, ",")
	return res
end

--wml array
function wc2_utils.pick_random_t(str)
	local size = wml.variables[str .. ".length"]
	if size ~= 0 then
		local index = wesnoth.random(size) - 1
		local res = wml.variables[str .. "[" ..  index .. "]"]
		wml.variables[str .. "[" ..  index .. "]"] = nil
		return res
	end
end

--like table concat but for tstrings.
function wc2_utils.concat(t, sep)
	local res = t[1]
	if not res then
		return ""
	end
	for i = 2, #t do
		-- uses .. so we dont hae to call tostring. so this function can still return a tstring.
		res = res .. sep .. t[i]
	end
	return res
end

function wc2_utils.range(a1,a2)
	if a2 == nil then
		a2 = a1
		a1 = 1
	end
	local res = {}
	for i = a1, a2 do
		res[i] = i
	end
	return res
end

function wc2_utils.facing_each_other(u1,u2)
	u1.facing = wesnoth.map.get_relative_dir(u1.x, u1.y, u2.x, u2.y)
	u2.facing = wesnoth.map.get_relative_dir(u2.x, u2.y, u1.x, u1.y)
	wesnoth.wml_actions.redraw {}
end

function wc2_utils.has_no_advances(u)
	return #u.advances_to == 0
end

local global_vars = setmetatable({}, {
	__index = function(self, namespace)
		return setmetatable({}, {
			__index = function(self, name)
				wml.variables.lua_global_variable = nil
				wesnoth.unsynced(function()
					wesnoth.wml_actions.get_global_variable {
						namespace = namespace,
						to_local = "lua_global_variable",
						from_global = name,
						immediate = true,
					}
				end)
				local res = wml.variables.lua_global_variable
				wml.variables.lua_global_variable = nil
				if res == "" then
					return nil
				end
				return res
			end,
			__newindex = function(self, name, val)
				wml.variables.lua_global_variable = val
				wesnoth.unsynced(function()
					wesnoth.wml_actions.set_global_variable {
						namespace = namespace,
						from_local = "lua_global_variable",
						to_global = name,
						immediate = true,
					}
				end)
				wml.variables.lua_global_variable = nil
			end,
		})
	end
})

wc2_utils.global_vars = global_vars.wc2

if rawget(_G, "wc2_menu_filters") == nil then
	wc2_menu_filters = {}
end

function wc2_utils.menu_item(t)
	local id_nospace = string.gsub(t.id, " ", "_")
	local cfg = {}
	on_event("start", function()
		wesnoth.wml_actions.set_menu_item {
			id = t.id,
			description = t.description,
			image = t.image,
			synced = t.synced,
			wml.tag.filter_location {
				lua_function="wc2_menu_filters." .. id_nospace,
			},
		}
	end)
	if t.handler then
		on_event("menu_item_" .. t.id, t.handler)
	end
	wc2_menu_filters[id_nospace] = t.filter
end


function wc2_utils.get_fstring(t, key)
	local args = wml.get_child(t, key .. "_data")
	if args then
		args = wc2_utils.get_fstring_all(args)
	else
		args = {}
	end
	return wesnoth.format(t[key], args)
end

function wc2_utils.get_fstring_all(t)
	local res = {}
	for k,v in pairs(t) do
		res[k] = wc2_utils.get_fstring(t, k)
	end
	return res
end

-- populates wc2_utils.world_conquest_data, reads [world_conquest_data] from all [resource]s and [era]
function wc2_utils.load_wc2_data()
	if wc2_utils.world_conquest_data == nil then
		local data_dict = {}
		local ignore_list = {}
		for i,res_id in ipairs(wesnoth.game_config.active_resources) do
			local ressource = wesnoth.get_resource(res_id)
			local world_conquest_data = wml.get_child(ressource, "world_conquest_data")
			if world_conquest_data then
				for ignore in wml.child_range(world_conquest_data, "ignore") do
					ignore_list[ignore.id] = true
				end
				table.insert(data_dict, {id=res_id, data = world_conquest_data})
			end
		end

		table.insert(data_dict, {id="era", data = wesnoth.game_config.era})
		

		-- make sure the result does not depend on the order in which these addons are loaded.
		table.sort(data_dict, function(a,b) return a.id<b.id end)


		wc2_utils.world_conquest_data = {}
		for i, v in ipairs(data_dict) do
			if not ignore_list[v.id] then
				for i2, tag in ipairs(v.data) do
					local tagname = tag[1]
					wc2_utils.world_conquest_data[tagname] = wc2_utils.world_conquest_data[tagname] or {}
					table.insert( wc2_utils.world_conquest_data[tagname], tag )
				end
			end
		end
	end
end

-- reads the tag @a tagnaem from [world_conquest_data] provided by any of the ressoucrs or eras used in the game.
-- returns a wml table that contains only tagname subtags.
function wc2_utils.get_wc2_data(tagname)
	wc2_utils.load_wc2_data()
	--todo: maybe we shoudl clear wc2_utils.world_conquest_data[tagname] afterwards ?
	return wc2_utils.world_conquest_data[tagname]
end


return wc2_utils
