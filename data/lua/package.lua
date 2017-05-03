
local empty_pkg = {}

local function resolve_package(pkg_name)
	if wesnoth.have_file(pkg_name) then return pkg_name end
	if pkg_name:sub(-4) ~= ".lua" then
		if wesnoth.have_file(pkg_name .. ".lua") then
			return pkg_name .. ".lua"
		end
		if pkg_name:sub(1, 4) ~= "lua/" then
			if wesnoth.have_file("lua/" .. pkg_name .. ".lua") then
				return "lua/" .. pkg_name .. ".lua"
			end
		end
	end
	if pkg_name:sub(1, 4) ~= "lua/" then
		if wesnoth.have_file("lua/" .. pkg_name) then
			return "lua/" .. pkg_name
		end
	end
	return nil
end

function wesnoth.require(pkg_name)
	-- First, check if the package is already loaded
	local loaded_name = resolve_package(pkg_name)
	if loaded_name and wesnoth.package[loaded_name] then
		return wesnoth.package[loaded_name]
	end

	-- Next, load the package with dofile
	local pkg = wesnoth.dofile(loaded_name)
	wesnoth.package[loaded_name] = pkg or empty_pkg
	return pkg
end
