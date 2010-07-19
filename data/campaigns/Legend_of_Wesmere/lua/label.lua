--! #textdomain wesnoth-low

local labels = {}
local wml_label = wesnoth.wml_actions.label

function wesnoth.wml_actions.label(cfg)
    table.insert(labels, cfg.__parsed)
    wml_label(cfg)
end

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
