local H = wesnoth.require "lua/helper.lua"
local replace_map
replace_map = wesnoth.register_wml_action("replace_map", function(cfg)
							    if not cfg.x and not cfg.y then
							       replace_map(cfg)
							    else
							       local x1,x2 = string.match(cfg.x, "(%d+)-(%d+)")
							       local y1,y2 = string.match(cfg.y, "(%d+)-(%d+)")
							       local header,map = string.match(cfg.map_data, "(.-)\n\n(.*)")
							       local b = string.match(header, "border_size=(%d+)")
							       x2 = x2 + 2 * b
							       y2 = y2 + 2 * b
							       local t = {}
							       local y = 1
							       for row in string.gmatch(map, "[^\n]+") do
								  if y >= tonumber(y1) and y <= tonumber(y2) then
								     local r = {}
								     local x = 1
								     for tile in string.gmatch(row, "[^,]+") do
									if x >= tonumber(x1) and x <= tonumber(x2) then r[x - tonumber(x1) + 1] = tile end
									x = x + 1
								     end
								     t[y - tonumber(y1) + 1] = table.concat(r, ',')
								  end
								  y = y + 1
							       end
							       local s = table.concat(t, '\n')
							       local new_map = string.format("border_size=%d\nusage=map\n\n%s", b, s)
							       replace_map { map = new_map, expand = true, shrink = true }
							    end
							 end)