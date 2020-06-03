
----------------------------------------------------------
---- defines debug_wml, a helper function to print a  ----
---- lua table that might or might not describe a wml ----
---- table, the function tries to print wml.tag       ----
---- whenever possible                                ----
----------------------------------------------------------

local function is_wml_tag(t)
	for i, v in pairs(t) do
		if i ~= 1 and i ~= 2 then
			return false
		end
	end
	return type(t[1]) == "string" and type(t[2]) == "table"
end

local function valid_lua_id(str)
	if type(str) ~= "string" then
		return false
	end
	local pattern = "^[_a-zA-Z][_0-9a-zA-Z]*$"
	if string.find(str, pattern) then
		return true
	else
		return false
	end
end

function debug_wml(cfg, use_newlines)
	use_newlines = use_newlines ~= false
	local indend = 0
	local res = {}

	local function write(str)
		res[#res + 1] = str
	end

	local function write_newline()
		if use_newlines then
			res[#res + 1] = "\n" .. string.rep("\t", indend)
		else
			res[#res + 1] = " "
		end
	end

	local print_table = nil
	local print_value = nil

	print_table = function(t)
		if is_wml_tag(t) then
			write("wml.tag." .. t[1])
			write(" ")
			print_table(t[2])
		else
			write("{")
			indend = indend + 1
			local length = 0
			for i, v in ipairs(t) do
				write_newline()
				length = i
				print_value(v)
				write(",")
			end
			for k, v in pairs(t) do
				if not (type(k) == "number" and k > 0 and k <= length) then
					write_newline()
					if valid_lua_id(k) then
						write(k)
					else
						write("[")
						print_value(k)
						write("]")
					end
					write(" = ")
					print_value(v)
					write(",")
				end
			end
			indend = indend - 1
			write_newline()
			write("}")
		end
	end

	print_value = function(val)
		if val == nil then
			write("nil")
		elseif type(val) == "number" or type(val) == "boolean" then
			write(tostring(val))
		elseif type(val) == "string" then
			write(string.format("%q", val))
		elseif type(val) == "userdata" and getmetatable(val) == "translatable string" then
			write(string.format("%q", tostring(val)))
		elseif type(val) == "table" then
			print_table(val)
		else
			error("unknonw type")
		end
	end
	print_value(cfg)
	return table.concat(res, "")
end
