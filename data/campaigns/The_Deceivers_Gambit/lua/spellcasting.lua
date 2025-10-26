local _ = wesnoth.textdomain "wesnoth-tdg"
local utils = wesnoth.require "wml-utils"
local skill_data = wesnoth.dofile('campaigns/The_Deceivers_Gambit/lua/skill_set.lua')

-- to make code shorter
local wml_actions = wesnoth.wml_actions

-- metatable for GUI tags
local T = wml.tag

function deep_copy(original)
    local copy = {}
    for g, v in pairs(original) do
        if type(v) == "table" then
            copy[g] = deep_copy(v)
        else
            copy[g] = v
        end
    end
    return copy
end



--###########################################################################################################################################################
--                                                                  SKILL DIALOG
--###########################################################################################################################################################
function display_skills_dialog(selecting)
    local result_table = {} -- table used to return selected skills
    local delfador   = ( wesnoth.units.find_on_map({ id="Delfador"      }) )[1]
    local apprentice = ( wesnoth.units.find_on_map({ type="Delfador L1" }) )[1]

    --###############################
    -- CREATE DIALOG
    --###############################
    local dialog = {
        definition="menu",
        T.helptip{ id="tooltip_large" }, -- mandatory field
        T.tooltip{ id="tooltip_large" }, -- mandatory field
        T.grid{} }
    local grid = dialog[3]

    local skill_set_copy = deep_copy(skill_data.skill_set)

    -------------------------
    -- HEADER
    -------------------------
    table.insert( grid[2], T.row{ T.column{ border="bottom", border_size=15, T.image{  label="icons/banner1.png"  }}} )
    local                title_text = selecting and _"Select Delfador’s Spells"       or _"Cast Delfador’s Spells"
    if (apprentice) then title_text = selecting and _"Select the Apprentice’s Spells" or _"Cast the Apprentice’s Spells" end
    table.insert( grid[2], T.row{ T.column{ T.label{
        definition="title",
        horizontal_alignment="center",
        label = title_text,
    }}} )
    local                help_text = _"<span size='small'><i>Delfador knows many useful spells, and will learn more as he levels-up automatically throughout the campaign. Delfador does not use XP to level-up. Instead,\nDelfador uses XP to cast certain spells. If you select spells that cost XP, <b>double- or right-click on Delfador to cast them</b>. You can only cast 1 spell per turn.</i></span>"
    if (apprentice) then help_text = _"<span size='small'><i>The apprentice knows several useful spells, and will learn more as he levels-up automatically throughout the campaign. The apprentice does not use XP to level-up. Instead,\nhe uses XP to cast certain spells. If you select spells that cost XP,<b>double- or right-click on the apprentice to cast them</b>. You can only cast 1 spell per turn.</i></span>" end
    table.insert( grid[2], T.row{ T.column{ border="top", border_size=15, T.label{ use_markup=true, label=help_text }}} )
    table.insert( grid[2], T.row{ T.column{ border="top", border_size=15, T.image{  label="icons/banner2.png"  }}} )

    -------------------------
    -- SKILL GROUPS
    -------------------------
    -- each button/image/label id ends with the index of the skill group it corresponds to
    -- put all these in 1 big grid, so they can have their own table-layout
    local skill_grid = T.grid{}
    for i=0,#skill_set_copy,1 do if (i>delfador.level) then skill_set_copy[i]=nil end end -- don't show skill groups if underleveled
    for i=0,#skill_set_copy,1 do
        local button
        local subskill_row
        if (selecting) then
            -- menu button for selecting skills
            button = T.menu_button{  id="button"..i, use_markup=true  }
            for j=1,#skill_set_copy[i],1 do
                if not wml.variables["unlock_" .. string.sub(skill_set_copy[i][j].id, 7, -1)] then
                    skill_set_copy[i][j] = skill_data.locked
                else
                    skill_set_copy[i][j] = skill_set_copy[i][j]
                end
                table.insert( button[2], T.option{label=skill_set_copy[i][j].label} )
            end
        else -- button for casting spells, or label for displaying skills
            for j=1,#skill_set_copy[i],1 do
                local skill = skill_set_copy[i][j]
                if (wml.variables[skill.id]) then
                    if (not (skill.xp_cost or skill.gold_cost)) then button=T.label{  id="button"..i, use_markup=true, label=skill.label }
                    else                        button=T.button{ id="button"..i, use_markup=true, label=skill.label } end
                    -- handle one skill with multiple buttons
                    if (skill.subskills) then
                        subskill_row = T.row{}
                        for k=1,#skill.subskills,1 do
                            local subskill = skill.subskills[k]
                            table.insert( subskill_row[2], T.column{T.button{id=subskill.id,use_markup=true,label=subskill.label}} );
                        end
                    end
                end
            end
            if (not button) then button=T.label{id="button"..i} end -- dummy button
        end

        -- skill row
        table.insert( skill_grid[2], T.row{
            T.column{ border="left",  border_size=15, button},
            T.column{                                 T.label{label="  "}},  T.column{  horizontal_alignment="left", T.image{      id="image"..i          }  },
            T.column{ border="right", border_size=15, T.label{label="  "}},  T.column{  horizontal_alignment="left", T.rich_label{ id="label"..i, width=0 }  },
        } )

        -- subskill row
        if (subskill_row) then table.insert( skill_grid[2], T.row{
            T.column{T.label{}}, T.column{T.label{}},
            T.column{T.label{}}, T.column{T.label{}},
            T.column{T.grid{subskill_row}},
        } ) end

        -- spacer row
        table.insert( skill_grid[2], T.row{
            T.column{T.label{label="  "}},
            T.column{T.label{}}, T.column{T.label{}},
            T.column{T.label{}}, T.column{T.label{}}
        } )
    end
    table.insert( grid[2], T.row{T.column{ horizontal_alignment="left", skill_grid }} )

    -------------------------
    -- CONFIRM BUTTON
    -------------------------
    table.insert( grid[2], T.row{ T.column{T.image{  label="icons/banner2.png"  }}} )
    if (selecting) then
        table.insert( grid[2], T.row{ T.column{ T.grid{ T.row{ T.column{
            border="top,right", border_size=10,
            T.button{  id="confirm_button", use_markup=true, return_value=1, label=_"Confirm Spells <small><i>(can be changed every scenario)</i></small>"  }
        }, T.column{
            border="top,left",  border_size=10,
            T.button{  id="wait_button",    use_markup=true, return_value=2, label=_"Choose Later"  }
        }}}}})
    else
        table.insert( grid[2], T.row{ T.column{
            border="top", border_size=10,
            T.button{  id="confirm_button", use_markup=true, return_value=1, label="Cancel"  }
        }})
    end

    table.insert( grid[2], T.row{ T.column{ border="top", border_size=15,  T.image{  label="icons/banner4.png"  }}} )





    --###############################
    -- POPULATE DIALOG
    --###############################
    -------------------------
    -- PRESHOW
    -------------------------
    local function preshow(dialog2) -- can't name this variable "dialog" or we get a warning about shadowing the "dialog" defined on 345
        -- for the button corresponding to each skill group
        for i,group in pairs(skill_set_copy) do
            button = dialog2["button"..i]

            -- menu callbacks for selecting skills
            if (selecting) then
                -- default to whatever skill we had selected last time
                for j,skill in pairs(skill_set_copy[i]) do
                    if (wml.variables[skill.id]) then button.selected_index=j end
                end

                -- whenever we refresh the menu, update the image and label
                refresh = function(button)
                    if (not skill_set_copy[i][1]) then return end
                    dialog2["image"..i].label = skill_set_copy[i][button.selected_index].image
                    dialog2["label"..i].label = skill_set_copy[i][button.selected_index].description
                    dialog2["label"..i].on_link_click = function(dest)
                        gui.show_help(dest)
                    end

                    -- also update variables
                    for j,skill in pairs(skill_set_copy[i]) do
                        result_table[skill.id] = (j==button.selected_index) and "yes" or "no"
                        if (skill.id=="skill_locked") then result_table[skill.id]="no" end
                    end
                end

                -- refresh immediately, and after any change
                refresh(button)
                button.on_modified = refresh

            -- fixed labels for casting/displaying skills/spells
            else dialog2["button"..i].visible = false
                for j,skill2 in pairs(skill_set_copy[i]) do -- can't name this variable "skill" or we get a warning about being shadowed by the "skill" defined on 503
                    if (not wml.variables[skill2.id]) then goto continue end

                    -- if we know this skill, reveal and initialize the UI
                    dialog2["button"..i].visible = true
                    dialog2["image" ..i].label = skill2.image
                    dialog2["label" ..i].label = skill2.description
                    dialog2["label" ..i].on_link_click = function(dest)
                        gui.show_help(dest)
                    end

                    -- if the button is clickable (i.e. a castable spell), set on_button_click
                    local function initialize_button( buttonid, skill, small )
                        if (dialog2[buttonid].type=="button") then
                            -- cancel spell
                            local function delfador_has_object(object_id) return wesnoth.units.find_on_map{ id='Delfador', T.filter_wml{T.modifications{T.object{id=object_id}}} }[1] end
                            if (delfador_has_object(skill.id)) then
                                dialog2[buttonid].label = small and "<span size='small'>Cancel</span>" or label('Cancel')
                                dialog2[buttonid].on_button_click = function()
                                    wml.variables['skill_id'] = skill.id.."_cancel"
                                    gui.widget.close(dialog2)
                                end

                            -- errors (extra spaces are to center the text)
                            elseif (wml.variables['spellcasted_this_turn']) then
                                dialog2[buttonid].label = small and _"<span size='small'>1 spell/turn</span>" or _"<span> Can only cast\n1 spell per turn</span>"
                                dialog2[buttonid].enabled = false
                            elseif (delfador.race~='human') then
                                dialog2[buttonid].label = small and _"<span size='small'>Polymorphed</span>" or _"<span>  Blocked by\n  Polymorph</span>"
                                dialog2[buttonid].enabled = false
                            elseif (wesnoth.units.find_on_map{ id='Delfador', T.filter_location{radius=3, T.filter{id='delfador_mirror3'}} }[1]) then   -- mirror delfador counterspell
                                dialog2[buttonid].label = small and _"<span size='small'>Counterspelled</span>" or _"<span>  Blocked by\n Counterspell</span>"
                                dialog2[buttonid].enabled = false
                            elseif (wml.variables['counterspell_active']) then -- delfador counterspell
                                dialog2[buttonid].label = small and _"<span size='small'>Counterspelled</span>" or _"<span>  Blocked by\n Counterspell</span>"
                                dialog2[buttonid].enabled = false
                            elseif (skill.xp_cost and skill.xp_cost>delfador.experience) then
                                dialog2[buttonid].label = small and _"<span size='small'>No XP</span>" or label('Insufficient XP')
                                dialog2[buttonid].enabled = false
                            elseif (skill.gold_cost and skill.gold_cost>wesnoth.sides[delfador.side].gold) then
                                dialog2[buttonid].label = small and _"<span size='small'>No Gold</span>" or label('Insufficient Gold')
                                dialog2[buttonid].enabled = false
                            elseif (skill.atk_cost and skill.atk_cost>delfador.attacks_left) then
                                dialog2[buttonid].label = small and _"<span size='small'>No Attack</span>" or label('No Attack')
                                dialog2[buttonid].enabled = false

                            -- cast spell
                            else
                                dialog2[buttonid].on_button_click = function()
                                    if (skill.xp_cost)  then delfador.experience  =delfador.experience  -skill.xp_cost  end
                                    if (skill.gold_cost)  then wesnoth.sides[delfador.side].gold =wesnoth.sides[delfador.side].gold  -skill.gold_cost  end
                                    if (skill.atk_cost) then delfador.attacks_left=delfador.attacks_left-skill.atk_cost end
                                    wml.variables['skill_id'] = skill.id
                                    wml.variables['spellcasted_this_turn'] = skill.id
                                    gui.widget.close(dialog2)
                                end
                            end
                        end
                    end
                    initialize_button("button"..i, skill2);

                    -- if this skill has subskills, initialize each button
                    if (skill2.subskills) then
                        for k,subskill in pairs(skill2.subskills) do
                            initialize_button(subskill.id, subskill, true);
                        end
                    end
                    ::continue::
                end
            end
        end
    end

    -------------------------
    -- SHOW DIALOG
    -------------------------
    wml.variables['skill_id'] = nil

    -- select spell, synced
    if (selecting) then
        dialog_result = wesnoth.sync.evaluate_single(function()
            retval = gui.show_dialog( dialog, preshow )
            result_table.wait_to_select_spells = retval==2 and 'yes' or 'no' --not nil, or else the key appears blank
            return result_table;
        end)
        for skill_id,skill_value in pairs(dialog_result) do wml.variables[skill_id] = result_table.wait_to_select_spells=='yes' and 'no' or skill_value end
        wml.variables['wait_to_select_spells'] = result_table.wait_to_select_spells; --set wait_to_select_spells manually, since it often gets overwritten to 'no' above
        wesnoth.game_events.fire('refresh_delfador_skills')

    -- cast spells, synced
    else
        dialog_result = wesnoth.sync.evaluate_single(function()
            gui.show_dialog( dialog, preshow )
            if (wml.variables['skill_id']) then wesnoth.game_events.fire('cast_skill_synced', delfador.x, delfador.y) end --used by spellcasting.cfg
            wml.variables['skill_id'] = nil
        end)
    end
end




























--###########################################################################################################################################################
--                                                                      "MAIN"
--###########################################################################################################################################################
-------------------------
-- DEFINE WML TAGS
-------------------------
function wml_actions.select_delfador_skills(cfg)
    display_skills_dialog(true)
end
function wml_actions.display_skills_dialog(cfg)
    if (wml.variables['is_during_attack']) then return end
    if (wml.variables['is_during_move']  ) then return end
    if (wml.variables['not_player_turn'] ) then return end

    wesnoth.audio.play("miss-2.ogg")
    if (wml.variables['no_spellcasting_event']) then
        wesnoth.game_events.fire(wml.variables['no_spellcasting_event'], cfg.x, cfg.y)
    else
        display_skills_dialog()
    end
end


-------------------------
-- DETECT DOUBLECLICKS
-------------------------
local last_click = os.clock()
wesnoth.game_events.on_mouse_action = function(x,y)
    local selected_unit = wesnoth.units.find_on_map{ x=x, y=y }
    if (not selected_unit[1] or selected_unit[1].id~='Delfador') then return end

    if (os.clock()-last_click<0.25) then
        if (wml.variables['wait_to_select_spells']) then
            wml_actions.select_delfador_skills()
        else
            wml_actions.display_skills_dialog{ x=x, y=y }
        end
        last_click = 0 --prevent accidentally immediately re-opening the dialog
    else
        last_click = os.clock()
    end
end

-------------------------
-- DETECT MOUSEMOVES
-------------------------
function wml_actions.listen_for_mousemove(cfg)
    wesnoth.game_events.on_mouse_move = function(x,y)
         wesnoth.game_events.fire('mousemove_synced', x, y)
         wesnoth.game_events.on_mouse_move = nil --only trigger once
    end
end
