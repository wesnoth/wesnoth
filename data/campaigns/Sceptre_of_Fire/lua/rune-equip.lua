
function wesnoth.wml_actions.rune_choice(cfg)
   cfg = wml.literal(cfg)
   for i, tag in ipairs(cfg) do
        if tag[1] == "option" then
            tag[2].label = tag[2].label:vformat{cost = tag[2].cost}
        end
    end
    wesnoth.wml_actions.message(wml.tovconfig(cfg))
end
