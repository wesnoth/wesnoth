
local hint_message = {valid = false}
function wesnoth.wml_actions.hint_message(cfg)
    if hint_message.valid then
        hint_message:remove()
    end
    if cfg.message and not cfg.remove then
        local text = cfg.message
        if cfg.caption and cfg.caption ~= "" then
            text = "<b><big>" .. cfg.caption .. "</big></b>\n" .. cfg.message
        else
            text = cfg.message
        end
        hint_message = wesnoth.interface.add_overlay_text(text, {
            size = 18,
            location = {5,5},
            color = {255, 255, 255},
            duration = "unlimited",
            max_width = "40%",
            valign = "top",
            halign = "left"
        })
    end
end
