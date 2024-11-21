
local hint_message = {valid = false}
local hint_text = ""
function wesnoth.wml_actions.hint_message(cfg)
    if hint_message.valid then
        hint_message:remove()
    end
    if cfg.message and not cfg.remove then
        hint_text = cfg.message
        if cfg.caption and cfg.caption ~= "" then
            hint_text = "<b><big>" .. cfg.caption .. "</big></b>\n" .. hint_text
        end
        hint_message = wesnoth.interface.add_overlay_text(hint_text, {
            size = 18,
            location = {5,5},
            color = {255, 255, 255},
            bgcolor = {0, 0, 0},
            bgalpha = 85,
            duration = "unlimited",
            max_width = "40%",
            valign = "top",
            halign = "left"
        })
    end
end

wesnoth.persistent_tags.hint_message.read = wesnoth.wml_actions.hint_message

function wesnoth.persistent_tags.hint_message.write(add)
    add{message = hint_text}
end

