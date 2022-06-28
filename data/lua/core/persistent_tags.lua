--[========[Save/Load Hooks]========]

if wesnoth.kernel_type() == "Game Lua Kernel" then
	print("Creating persistent_tags table...")

	---Defines a tag for custom saved game data.
	---@class persistent_tag
	---@field read fun(cfg:WMLTable)
	---@field write fun(add:fun(WMLTable))

	---@type table<string, persistent_tag>
	wesnoth.persistent_tags = setmetatable({}, {
		-- This just makes assignment of the read/write funtions more convenient
		__index = function(t,k)
			rawset(t,k,{})
			return t[k]
		end
	})

	-- Note: We don't save the old on_load and on_save here.
	-- It's not necessary because we know this will be the first one registered.
	function wesnoth.game_events.on_load(cfg)
		local warned_tags = {}
		for i = 1, #cfg do
			local name = cfg[i][1]
			-- Use rawget so as not to trigger the auto-adding mechanism
			local tag = rawget(wesnoth.persistent_tags, name)
			if type(tag) == 'table' and type(tag.read) == 'function' then
				tag.read(cfg[i][2])
			elseif tag ~= nil and not warned_tags[name] then
				local msg = string.format("Invalid persistent tag [%s], should be a table containing read and write functions.", name)
				wesnoth.log("err", msg, true)
				warned_tags[name] = true
			else
				local msg = string.format("[%s] not supported at scenario toplevel", name)
				wesnoth.log("err", msg, true)
				warned_tags[name] = true
			end
		end
	end

	function wesnoth.game_events.on_save()
		local data_to_save = {}
		for name, tag in pairs(wesnoth.persistent_tags) do
			if type(tag) == 'table' and type(tag.write) == 'function' then
				local function add(data)
					table.insert(data_to_save, wml.tag[name](data))
				end
				tag.write(add)
			end
		end
		return data_to_save
	end
end
