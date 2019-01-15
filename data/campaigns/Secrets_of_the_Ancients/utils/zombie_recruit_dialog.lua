-- After preshow, this variable will hold the unit_types being shown in the listbox.
local listedZombies = {}
-- These two variables will be passed back to the WML engine.
local recruitedType
local recruitCost

local zombies = V.zombies
local sides = wesnoth.get_sides()

local unit_row = T.row {
    T.column { grow_factor=0, border="right", border_size=5, horizontal_alignment = "left", T.image { id = "unit_sprite" } },
    T.column { grow_factor=0, border="right", border_size=10, T.grid {
        T.row { T.column { T.grid { T.row { T.column { T.label { definition = "default", use_markup = "true", id = "unit_type" } } } } } },
        T.row { T.column { border="top", border_size=7, horizontal_alignment = "left", T.grid { T.row {
            T.column {border="right", border_size=6, T.image { label = "themes/gold.png" } },
            T.column { T.label { definition = "default", use_markup = "true", id = "unit_cost"} }
        } } } }
    } },
    T.column { grow_factor=1, T.spacer {} }
}

-- There is a panel definition that does this, but it's more complicated than
-- this grid; several of these fields can have constant values.
local info_grid = T.grid {
    T.row { T.column { border = "top,bottom", border_size = 10, horizontal_alignment="left", horizontal_grow=true, T.image { id = "large_unit_sprite" } } },
    T.row {  T.column { border = "bottom", border_size = 5, horizontal_alignment="left", horizontal_grow=true, T.label { use_markup = "true", id = "large_unit_type" } } },
    T.row { T.column { T.grid { T.row {
        T.column { border = "right", border_size = 10, T.label { definition = "default_bold", label= _ "Lvl 0" } },
        T.column { border = "right", border_size = 5, T.image { label = "../../images/icons/alignments/alignment_chaotic_30.png" } },
        T.column { border = "right", border_size = 5, T.image { label = "../../images/icons/unit-groups/race_undead_30.png" } },
        T.column { border = "right", border_size = 35, T.label { definition="default_small", label= _ "Undead" } },
        T.column { border = "right", border_size = 17, T.button { definition="action_about", id="unit_help_button" } }
    } } } },
    T.row { T.column { border = "bottom", border_size = 6, T.spacer { } } },
    T.row { T.column { border = "bottom", border_size = 12, horizontal_grow = true, T.label { definition = "default_tiny", use_markup = "true", id="unit_points" } } },
    T.row { T.column { border = "bottom", border_size = 2, horizontal_alignment = "left", T.label { definition ="default_small", use_markup = "true", label = "<b>" .. _ "Traits" .. "</b>" } } },
    T.row { T.column { border = "bottom", border_size = 2, horizontal_alignment = "left", T.label { definition = "default_small", label = "   undead" } } },
    T.row { T.column { border = "bottom", border_size = 12, horizontal_alignment = "left", T.label { definition = "default_small", label = "   fearless" } } },
    T.row { T.column { border = "bottom", border_size = 2, horizontal_alignment = "left", T.label { definition = "default_small", use_markup = "true", label = "<b>" .. _ "Attacks" .. "</b>" } } },
    T.row { T.column { border = "bottom", border_size = 2, horizontal_alignment = "left", horizontal_grow = true, T.label { definition = "default_small", use_markup = "true", id = "unit_attack" } } },
    T.row { T.column { border = "bottom", border_size = 2, horizontal_alignment = "left", horizontal_grow = true, T.label { definition = "default_small", use_markup = "true", id = "damage_type" } } },
    T.row { T.column { horizontal_alignment = "left", T.label { definition="default_small", use_markup = "true", label = "<span color='#a69275'>     plague</span>" } } },
    T.row { T.column { border = "bottom", border_size = 100, T.spacer { } } }
}

local zombie_recruit_dialog = { maximum_height=676, minimum_height=608, 
    T.tooltip { id = "tooltip_large" },
    T.helptip { id = "tooltip_large" },
    T.grid { 
        T.row {
            T.column { border = "left,top", border_size = 5, horizontal_alignment = "left", T.label { definition = "title", label = _ "Choose a Corpse" } },
            T.column { T.spacer {} },
        },
        T.row {
            T.column { border = "left", border_size = 5, vertical_alignment="top", info_grid },
            T.column { border = "right,top", border_size = 5, vertical_alignment = "top", T.grid {
                T.row { T.column { horizontal_grow = true, T.listbox { scrollbar="manditory", vertical_scrollbar_mode="initial_auto", id = "unit_list",
                    T.list_definition { T.row { T.column { horizontal_grow=true, T.toggle_panel { return_value = -1, T.grid { unit_row } }
                    } } }
                } } }
            } } 
        },
        T.row { 
            T.column { border = "left, top, bottom", border_size = 7, horizontal_alignment="left", T.button { id="help_button", definition="help" } },
            T.column { border = "left, top, bottom", border_size = 7, T.grid { T.row {
                T.column { border_size=10, border="right", T.button { return_value = 1, label = _"Recruit" } },
                T.column { T.button { id = "cancel", label = _"Cancel" } }
            } } } 
        } 
    }
}

local function preshow()
    local function select()
        local unit_type = wesnoth.unit_types[ listedZombies[wesnoth.get_dialog_value "unit_list"] ]
        wesnoth.set_dialog_value(unit_type.image .. "~RC(magenta>red)~XBRZ(2)", "large_unit_sprite")
        wesnoth.set_dialog_value( "<span size='large'>" .. unit_type.name .. "</span>", "large_unit_type")
        wesnoth.set_dialog_value( "<span color='#20dc00'><b>".. _"HP: ".. "</b>" .. unit_type.max_hitpoints .. "</span> | <span color='#00a0e1'><b>".. _"XP: ".. "</b>" .. unit_type.max_experience .. "</span> | <b>".. _"MP: ".. "</b>" .. unit_type.max_moves, "unit_points")
        wesnoth.set_dialog_value( "<span color='#f5e6c1'>   6×2 " .. unit_type.attacks[1].description .. "</span>", "unit_attack")
        wesnoth.set_dialog_value( "<span color='#a69275'>     melee–" .. unit_type.attacks[1].type .. "</span>", "damage_type")
    end
    
    local function general_help()
        W.open_help { topic="recruit_and_recall" }
    end

    local function unit_help()
        W.open_help { topic="unit_" ..  listedZombies[wesnoth.get_dialog_value "unit_list"] }
    end
    
    wesnoth.set_dialog_callback( select, "unit_list" )
    wesnoth.set_dialog_callback( general_help, "help_button" )
    wesnoth.set_dialog_callback( unit_help, "unit_help_button" )

    local zArrayIndex = 0  -- Index of the original, zero-indexed array from WML.
    local zListIndex = 1  -- Index of the list of recrutable zombies in this dialog box.
    while zombies[zArrayIndex] do
        local z=zombies[zArrayIndex]
        if z.allow_recruit then
            local unit_type = wesnoth.unit_types[z.type]
            local afford_color_span_start = ""
            local afford_color_span_end = ""
            if sides[1].gold < unit_type.cost then
                afford_color_span_start = "<span color='red'>"
                afford_color_span_end = "</span>"
            end
            wesnoth.set_dialog_value(afford_color_span_start .. unit_type.name .. " " .. z.sota_variation .. afford_color_span_end, "unit_list", zListIndex, "unit_type")
            wesnoth.set_dialog_value(unit_type.image .. "~RC(magenta>red)", "unit_list", zListIndex, "unit_sprite")
            wesnoth.set_dialog_value(afford_color_span_start .. unit_type.cost .. afford_color_span_end, "unit_list", zListIndex, "unit_cost")
            listedZombies[ zListIndex ] = z.type
            zListIndex = zListIndex + 1
        end
        zArrayIndex = zArrayIndex + 1
    end
    wesnoth.set_dialog_value(1, "unit_list")
    select()
end

local function postshow()
    recruitedType = listedZombies[wesnoth.get_dialog_value "unit_list"]
    recruitCost = wesnoth.unit_types[ recruitedType ].cost
end

-- Start execution --

-- Find out if there is at least one zombie in the list box.
-- This will only be necessary if the WML changes, but it could, so we'll check.
local zArrayIndex = 0  -- Index of the original, zero-indexed array from WML.
local zExists = false
while zombies[zArrayIndex] and zExists == false do
    local z=zombies[zArrayIndex]
    if z.allow_recruit then
        zExists=true
    end
    zArrayIndex = zArrayIndex + 1
end

wesnoth.set_variable( "recruitedZombieType", "cancel" ) -- default value

if zExists==false then
    wesnoth.show_message_box("", _ "There are no corpses available.", "")
else
    local returned = wesnoth.show_dialog(zombie_recruit_dialog, preshow, postshow)
    if  returned ~= -2 and sides[1].gold  < recruitCost then
        wesnoth.show_message_box("", _ "You do not have enough gold to recruit that unit", "")
    elseif returned ~= -2 and (sides[1].gold ) >= recruitCost then
        wesnoth.set_variable( "recruitedZombieType", recruitedType )
        wesnoth.set_variable( "recruitedZombieCost", recruitCost )
    end
end
