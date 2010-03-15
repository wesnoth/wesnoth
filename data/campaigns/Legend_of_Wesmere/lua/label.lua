--! #textdomain wesnoth-low

local labels = {}
local wml_label

wml_label = wesnoth.register_wml_action("label",
  function(cfg)
    table.insert(labels, cfg.__parsed)
    wml_label(cfg)
  end)

wesnoth.register_wml_action("shift_labels",
  function(cfg)
    for k, v in ipairs(labels) do
      wml_label { x = v.x, y = v.y }
    end
    for k, v in ipairs(labels) do
      v.x = v.x + cfg.x
      v.y = v.y + cfg.y
      wml_label(v)
    end
  end)
