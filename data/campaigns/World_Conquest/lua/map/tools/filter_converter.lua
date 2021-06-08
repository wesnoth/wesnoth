
----------------------------------------------------------
---- A helper tool, that works (mostly) indpendently  ----
---- of the rest to convert location filter wml to    ----
---- the filter syntax used in this addon             ----
---- (map:find)                                       ----
----------------------------------------------------------



local and_to_all = {
	["and"] = "all",
	["or"] = "any",
	["not"] = "none"
}

local comp_tags = {
	any = true,
	all = true,
	none = true,
	notall = true,
}

function compose_filter_1(tag, f1, f2)
	if tag == f1[1] then
		if tag == f2[1] then
			for i = 2, #f2 do
				table.insert(f1, f2[i])
			end
		else
			table.insert(f1, f2)
		end
		return f1
	else
		if tag == f2[1] then
			table.insert(f2, 2, f1)
			return f2
		else
			return {tag, f1, f2}
		end
	end
end

function compose_filter(tag, f1, f2)
	local res = compose_filter_1(tag, f1, f2)
	if #res == 2 then
		return res[2]
	end
	return res
end

function parse_wml_filter(cfg)
	local res = { "all" }
	if cfg.x then
		table.insert(res, { "x", cfg.x})
	end
	if cfg.y then
		table.insert(res, { "y", cfg.y})
	end
	if cfg.terrain then
		table.insert(res, { "terrain", cfg.terrain})
	end
	if cfg.find_in then
		table.insert(res, { "find_in_wml", cfg.find_in})
	end
	for ad in wml.child_range(cfg, "filter_adjacent_location") do
		table.insert(res, { "adjacent", parse_wml_filter(ad), adjacent = ad.adjacent, count = ad.count })
	end
	if #res == 1 then
		print("empty filter")
	end
	if #res == 2 then
		res = res[2]
	end
	for i, v in ipairs(cfg) do
		local tag = and_to_all[v[1]]
		local content = v[2]
		if tag ~= nil then
			local subfilter = parse_wml_filter(content)
			if tag == "none" then
				subfilter = { "none", subfilter }
				tag = "all"
			end
			res = compose_filter(tag, res, subfilter)
		end
	end

	if cfg.radius then
		local filter_radius = wml.get_child(cfg, "filter_radius")
		filter_radius = filter_radius and parse_wml_filter(filter_radius)
		res = { "radius", cfg.radius, res, filter_radius = filter_radius }
	end

	return res
end



local function value_to_str(val)
	if val == nil then
		return "nil"
	elseif type(val) == "number" or type(val) == "boolean" then
		return tostring(val)
	elseif type(val) == "string" then
		return string.format("%q", val)
	else
		error("unknown type:'" .. type(val) .. "'")
	end
end

function print_filter(f, indent)
	local res = {}
	indent = indent or 0

	local function write(str)
		res[#res + 1] = str
	end

	local function write_newline()
		res[#res + 1] = "\n" .. string.rep("\t", indent)
	end

	local function write_filter(filter)
		if filter[1] == "adjacent" then
			write("f.adjacent(")
			write_filter(filter[2])
			if (filter.adjacent or filter.count) then
				write(", " .. value_to_str(filter.adjacent))
				write(", " .. value_to_str(filter.count))
			end
			write(")")
		end
		if filter[1] == "x" or  filter[1] == "y" or  filter[1] == "terrain"  or  filter[1] == "find_in_wml" then
			write("f." .. filter[1] .. "(")
			write(value_to_str(filter[2]))
			write(")")
		end
		if filter[1] == "radius" then
			--f_radius(r, f, f_r)
			write("f.radius(")
			write(value_to_str(filter[2]))
			write(", ")
			write_filter(filter[3])
			if filter.filter_radius then
				write(", ")
				write_filter(filter.filter_radius)
			end
			write(")")
		end
		if comp_tags[filter[1]] then
			write("f." .. filter[1] .. "(")
			indent = indent + 1

			for i = 2, #filter do
				local is_last = (i == #filter)
				write_newline()
				write_filter(filter[i])
				if not is_last then
					write(",")
				end
			end
			indent = indent - 1
			write_newline()
			write(")")
		end
	end
	write_filter(f)
	return table.concat(res, "")
end


function print_set_terrain(filter, terrain, extra)
	std_print("set_terrain { " .. value_to_str(terrain) .. ",")
	std_print("\t" .. print_filter(filter, 1) .. ",")
	for k, v in pairs(extra) do
		std_print("\t" .. k .. " = " .. value_to_str(v) .. ",")
	end
	std_print("}")
end

function convert_filter()
	local cfg = wml.parse(filesystem.read_file("./filterdata.cfg"))
	std_print("")
	for i, v in ipairs(cfg) do
		local tag = v[1]
		local content = v[2]
		if tag == "terrain" then
			local terrain = content.terrain
			content.terrain = nil
			local f = parse_wml_filter(content)
			print_set_terrain(f, terrain, { layer = content.layer })
		end
		if tag == "wc2_terrain" then
			for change in wml.child_range(content, "change") do
				local terrain = change.terrain
				local f = parse_wml_filter(wml.get_child(change, "filter"))
				local extras = {}
				for k, v in pairs(change) do
					if type(k) == "string" and k ~= "terrain" then
						extras[k] = v
					end
				end

				print_set_terrain(f, terrain, extras)
			end
		end
		if tag == "store_locations" then
			local variable = content.variable
			local f = parse_wml_filter(content)

			std_print("local " .. variable .. " = map:find(" .. print_filter(f, 1) .. ")")
		end
	end
	--local filter = parse_wml_filter(cfg)
	--std_print(print_filter(filter))
end
convert_filter()
