
local empty_pkg = {}

function wesnoth.require(pkg_name)
	-- First, check if the package is already loaded
	if wesnoth.package[pkg_name] then
		return wesnoth.package[pkg_name]
	end

	-- Next, load the package with dofile
	local pkg = wesnoth.dofile(pkg_name)
	wesnoth.package[pkg_name] = pkg or empty_pkg
	return pkg
end
