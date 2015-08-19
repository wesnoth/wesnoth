--! #textdomain wesnoth-low

local labels = {}
local wml_label = wesnoth.wml_actions.label
local replace_map = wesnoth.wml_actions.replace_map

function wesnoth.wml_actions.shift_labels(cfg)
	for k, v in ipairs(labels) do
		wml_label { x = v.x, y = v.y }
	end

	for k, v in ipairs(labels) do
		v.x = v.x + cfg.x
		v.y = v.y + cfg.y
		wml_label(v)
	end
end

--
-- Overrides of core tags
--

function wesnoth.wml_actions.label(cfg)
	table.insert(labels, cfg.__parsed)
	wml_label(cfg)
end

function wesnoth.wml_actions.replace_map(cfg)
	if not cfg.x and not cfg.y then
		return replace_map(cfg)
	end

	local x1,x2 = string.match(cfg.x, "(%d+)-(%d+)")
	local y1,y2 = string.match(cfg.y, "(%d+)-(%d+)")
	local map = cfg.map_data

	x1 = tonumber(x1)
	y1 = tonumber(y1)
	x2 = x2 + 2
	y2 = y2 + 2

	local t = {}
	local y = 1

	for row in string.gmatch(map, "[^\n]+") do
		if y >= y1 and y <= y2 then
			local r = {}
			local x = 1

			for tile in string.gmatch(row, "[^,]+") do
				if x >= x1 and x <= x2 then r[x - x1 + 1] = tile end
				x = x + 1
			end

			t[y - y1 + 1] = table.concat(r, ',')
		end

		y = y + 1
	end

	local new_map = table.concat(t, '\n')
	replace_map { map = new_map, expand = true, shrink = true }
end
