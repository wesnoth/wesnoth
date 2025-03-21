local T = wml.tag
local W = wml.fire
local _ = wesnoth.textdomain 'wesnoth-sota'

-- After preshow, this variable will hold the unit_types being shown in the listbox.
local listedZombies = {}
-- These two variables will be passed back to the WML engine.
local recruitedType
local recruitCost

local zombies = wml.array_access.get("zombies")
local sides = wesnoth.sides.find{}

local zombie_recruit_dialog = wml.load "campaigns/Secrets_of_the_Ancients/gui/zombie_recruit_dialog.cfg"

local function preshow(dialog)

    local function select()
        -- TODO: why not use unit_preview_pane widget ?
        local unit_type = wesnoth.unit_types[ listedZombies[dialog.unit_list.selected_index] ]
        dialog.large_unit_sprite.label = unit_type.image .. "~RC(magenta>red)~XBRZ(2)"
        dialog.large_unit_type.label = "<span size='large'>" .. unit_type.name .. "</span>"
        dialog.unit_points.label     = "<span color='#20dc00'><b>".. _"HP: ".. "</b>" .. unit_type.max_hitpoints .. "</span> | <span color='#00a0e1'><b>".. _"XP: ".. "</b>" .. unit_type.max_experience .. "</span> | <b>".. _"MP: ".. "</b>" .. unit_type.max_moves
        dialog.unit_attack.label     = "<span color='#f5e6c1'>   6×2 " .. unit_type.attacks[1].description .. "</span>"
        dialog.damage_type.label     = "<span color='#a69275'>     melee–" .. unit_type.attacks[1].type .. "</span>"
    end

    dialog.unit_list.on_modified = select

    function dialog.help_button.on_button_click()
        W.open_help { topic="recruit_and_recall" }
    end

    function dialog.unit_help_button.on_button_click()
        W.open_help { topic="unit_" .. listedZombies[dialog.unit_list.selected_index] }
    end

    for i,z in ipairs(zombies) do
        if z.allow_recruit then
            local unit_type = wesnoth.unit_types[z.type]
            local afford_color_span_start = ""
            local afford_color_span_end = ""
            if sides[1].gold < unit_type.cost then
                afford_color_span_start = "<span color='red'>"
                afford_color_span_end = "</span>"
            end

            local list_item = dialog.unit_list:add_item()
            if z.sota_name_in_recruit_dialog then
                list_item.unit_type.label   = afford_color_span_start .. z.sota_name_in_recruit_dialog .. afford_color_span_end
            else
                -- the player started the campaign with 1.15.6 or earlier
                list_item.unit_type.label   = afford_color_span_start .. unit_type.name .. " " .. z.sota_variation .. afford_color_span_end
            end
            list_item.unit_sprite.label = unit_type.image .. "~RC(magenta>red)"
            list_item.unit_cost.label   = afford_color_span_start .. unit_type.cost .. afford_color_span_end

            print("dialog.unit_list.size", dialog.unit_list.item_count)
            listedZombies[ dialog.unit_list.item_count ] = z.type
        end
    end
    dialog.unit_list.selected_index = 1
    select()
end

local function postshow(dialog)
    recruitedType = listedZombies[dialog.unit_list.selected_index]
    recruitCost = wesnoth.unit_types[ recruitedType ].cost
end

-- Start execution --

-- Find out if there is at least one zombie in the list box.
-- This will only be necessary if the WML changes, but it could, so we'll check.
local zArrayIndex = 1  -- Start index of the WML array in lua.
local zExists = false
while zombies[zArrayIndex] and zExists == false do
    local z=zombies[zArrayIndex]
    if z.allow_recruit then
        zExists=true
    end
    zArrayIndex = zArrayIndex + 1
end

wml.variables["recruitedZombieType"] = "cancel" -- default value

if zExists==false then
    gui.show_prompt("", _ "There are no corpses available.", "")
else
    local result = wesnoth.sync.evaluate_single(function()
        local res = gui.show_dialog(wml.get_child(zombie_recruit_dialog, 'resolution'), preshow, postshow)
        if res == -2 then
            return {recruitCost = 0, recruitedType = "cancel"}
        end
        if sides[1].gold  < recruitCost then
            gui.show_prompt("", _ "You do not have enough gold to recruit that unit", "")
            return {recruitCost = 0, recruitedType = "cancel"}
        end
        return {recruitCost = recruitCost, recruitedType = recruitedType}
    end)
    wml.variables["recruitedZombieType"] = result.recruitedType
    wml.variables["recruitedZombieCost"] = result.recruitCost
end
