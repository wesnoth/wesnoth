local _ = wesnoth.textdomain "wesnoth-tdg"


-- to make code shorter
local wml_actions = wesnoth.wml_actions

-- metatable for GUI tags
local T = wml.tag

























--###########################################################################################################################################################
--                                                                  DEFINE SKILLS
--###########################################################################################################################################################
function label(text)     return "<span size='1000'> \n</span><span size='large'>"..text.."</span><span size='8000'>\n </span>"  end
local skills = {
    --###############################
    -- GROUP 0 SKILLS
    --###############################
    [0] = {
        -------------------------
        -- MAGIC MISSILE
        -------------------------
        [1] = {
            id          = "skill_magic_missile",
            label       = label(_"Magic Missile"),
            image       = "attacks/magic-missile.png",
            description = _"<span color='#ad6a61'><i><b>Attack:</b></i></span> Ranged 7x3 fire, <i>magical</i>.",
        },
        -------------------------
        -- SHIELD
        -------------------------
        [2] = {
            id          = "skill_shield",
            label       = label(_"Shield"),
            image       = "icons/shield.png",
            description = _"<span color='#6ca364'><i><b>Spell:</b></i></span> Spend <span color='#00bbe6'><i>8xp</i></span> to gain <i>+20% dodge chance</i> until the start of your next turn or until cancelled.",
            xp_cost=8, --XP is also used in S04
        },
--         ------------------------- removed; too powerful
--         -- STASIS
--         -------------------------
--         [3] = {
--             id          = "skill_stasis",
--             label       = label("Stasis"),
--             image       = "icons/stasis.png",
--             description = _"<span color='#6ca364'><i><b>Spell:</b></i></span> Spend <span color='#00bbe6'><i>8xp</i></span> and <span color='#c06a61'><i>your attack</i></span> to <i>petrify</i> yourself and adjacent units until the start of your next turn.",
--             xp_cost=8,
--         },
        -------------------------
        -- PANACEA
        -------------------------
        [3] = {
            id          = "skill_panacea",
            label       = label(_"Panacea"),
            image       = "icons/potion_green_small.png",
            description = _"<span color='#6ca364'><i><b>Spell:</b></i></span> Spend <span color='#00bbe6'><i>8xp</i></span> to fully heal the lowest-health adjacent living ally, and increase its\n           attacks, strikes, and damage by its level. <span color='#dd0000'><b>Next turn, it dies.</b></span>",
            xp_cost=8, --XP is also used in spellcasting.cfg
        },
        -------------------------
        -- ANIMATE MUD
        -------------------------
        [4] = {
            id          = "skill_animate_mud",
            label       = label(_"Animate Mud"),
            image       = "icons/animate-mud.png",
            description = _"<span color='#a9a150'><i><b>Passive:</b></i></span> Learn to recruit <i>Mudcrawlers</i>. Mudcrawlers gain +100% damage and XP\n               while adjacent to you, but dissolve at the end of each scenario.",
        },
    },
    --###############################
    -- GROUP 1 SKILLS
    --###############################
    [1] = {
        -------------------------
        -- CHILL TOUCH
        -------------------------
        [1] = {
            id          = "skill_chill_touch",
            label       = label(_"Chill Touch"),
            image       = "icons/chill-touch.png",
            description = _"<span color='#ad6a61'><i><b>Attack:</b></i></span> Melee 6x3 cold, <i>slows</i>. Replaces your default melee attack.",
        },
        -------------------------
        -- LEVITATE
        -------------------------
        [2] = {
            id          = "skill_levitate",
            label       = label(_"Levitate"),
            image       = "icons/levitate.png",
            description = _"<span color='#6ca364'><i><b>Spell:</b></i></span> Spend <span color='#00bbe6'><i>8xp</i></span> to gain <i>flight</i> and the <i>skirmisher</i> ability until the start of your next turn or until cancelled.",
            xp_cost=8, --XP=8 is also used in S04
        },
        -------------------------
        -- FIND FAMILIAR
        -------------------------
        [3] = {
            id          = "skill_find_familiar",
            label       = label(_"Find Familiar"),
            image       = "icons/find-familiar.png",
            description = _"<span color='#a9a150'><i><b>Passive:</b></i></span> Begin each scenario with your trusty pet raven.\n               Your familiar’s level and xp persist across scenarios, but reset if it dies.",
        },
        -------------------------
        -- MNEMONIC
        -------------------------
        [4] = {
            id          = "skill_mnemonic",
            label       = label(_"Mnemonic"),
            image       = "icons/mnemonic.png",
            description = _"<span color='#a9a150'><i><b>Passive:</b></i></span> Whenever an adjacent ally gains xp, you gain the same amount of xp.",
        },
    },
    --###############################
    -- GROUP 2 SKILLS
    --###############################
    [2] = {
        -------------------------
        -- FIREBALL2
        -------------------------
        [1] = {
            id          = "skill_fireball2",
            label       = label(_"Fireball"),
            image       = "attacks/fireball.png",
            description = _"<span color='#ad6a61'><i><b>Attack:</b></i></span> Ranged 8x4 fire, <i>magical</i>.",
        },
        -------------------------
        -- ENERVATE
        -------------------------
        [2] = {
            id          = "skill_enervate",
            label       = label(_"Siphon"),
            image       = "icons/enervate.png", -- better than fireball2 vs orcs or undead, but sarians resist arcane and are vulnerable to fire. You also get this a few scenarios later than fireball2.
            description = _"<span color='#ad6a61'><i><b>Attack:</b></i></span> Ranged 8x4 arcane, <i>magical</i>, <i>drains</i>.",
        },
        -------------------------
        -- BLIZZARD
        -------------------------
        [3] = {
            id          = "skill_blizzard",
            label       = label(_"Blizzard"),
            image       = "icons/blizzard.png",
            description = _"<span color='#6ca364'><i><b>Spell:</b></i></span> Spend <span color='#00bbe6'><i>16xp</i></span> and <span color='#c06a61'><i>your attack</i></span> to slow enemy units and freeze terrain in a 3-hex radius.",
            xp_cost=16, atk_cost=1,
        },
        -------------------------
        -- COUNTERSPELL
        -------------------------
        [4] = {
            id          = "skill_counterspell",
            label       = label(_"Counterspell"),
            image       = "icons/counterspell.png",
            description = _"<span color='#6ca364'><i><b>Spell:</b></i></span> Spend <span color='#00bbe6'><i>16xp</i></span> to <i>disallow magical attacks</i> in a 3-hex radius, until cancelled.\n           Prevents spellcasting, but not passive skills.",
            xp_cost=16, --XP=16 is also used in S04
        },
        -------------------------
        -- POLYMORPH
        -------------------------
        [5] = {
            id          = "skill_polymorph",
            label       = label(_"Polymorph"),
            image       = "icons/polymorph.png",
            description = _"<span color='#6ca364'><i><b>Spell:</b></i></span> Transform into a stoat (<span color='#00bbe6'><i>1xp</i></span>), bear (<span color='#00bbe6'><i>8xp</i></span>), crab (<span color='#00bbe6'><i>16xp</i></span>), or roc (<span color='#00bbe6'><i>32xp</i></span>). Lasts until cancelled.\n            Replaces Delfador’s attacks, spells, and passives, but does not affect hitpoints.",
            subskills   = {
                [1]={ id="skill_polymorph_stoat",  xp_cost=1,  label="   <span>Stoat (<span color='#00bbe6'><i >1xp</i></span>)</span>   " },
                [2]={ id="skill_polymorph_bear",   xp_cost=8,  label="   <span>Bear (<span  color='#00bbe6'><i >8xp</i></span>)</span>   " },
                [3]={ id="skill_polymorph_crab",   xp_cost=16, label="   <span>Crab (<span  color='#00bbe6'><i>16xp</i></span>)</span>   " },
                [4]={ id="skill_polymorph_roc",    xp_cost=32, label="   <span>Roc (<span   color='#00bbe6'><i>32xp</i></span>)</span>   " }, },
        },
        -------------------------
        -- GLAMOUR
        -------------------------
        [6] = {
            id          = "skill_glamour",
            label       = label(_"Glamour"),
            image       = "icons/glamour.png",
            description = _"<span color='#a9a150'><i><b>Passive:</b></i></span> Gain the <i>leadership</i> ability.",
        },
    },
    --###############################
    -- GROUP 3 SKILLS
    --###############################
    [3] = {
        -------------------------
        -- FIREBALL3
        -------------------------
        [1] = {
            id          = "skill_fireball3",
            label       = label(_"Fireball"),
            image       = "attacks/fireball.png",
            description = _"<span color='#ad6a61'><i><b>Attack:</b></i></span> Ranged 12x4 fire, <i>magical</i>.",
        },
        -------------------------
        -- DANCING DAGGERS
        -------------------------
        [2] = {
            id          = "skill_dancing_daggers",
            label       = label(_"Dancing Daggers"),
            image       = "icons/dancing-daggers.png",
            description = _"<span color='#ad6a61'><i><b>Attack:</b></i></span> Ranged 5x8 blade, <i>backstab</i>.",
        },
        -------------------------
        -- ILLUSION
        -------------------------
        [3] = {
            id          = "skill_illusion",
            label       = label(_"Enthrall"),
            image       = "icons/illusion.png",
            description = _"<span color='#6ca364'><i><b>Spell:</b></i></span> Spend <span color='#00bbe6'><i>48xp</i></span> and <span color='#c06a61'><i>your attack</i></span> to magically disguise yourself as an awe-inspiring drake,\n           reducing accuracy and dodge by 10% for enemies in a 2 hex radius. Lasts until cancelled.",
            xp_cost=48, atk_cost=1,
        },
        -------------------------
        -- ANIMATE FIRE
        -------------------------
        [4] = {
            id          = "skill_animate_fire",
            label       = label(_"Animate Fire"),
            image       = "icons/animate-fire.png",
            description = _"<span color='#a9a150'><i><b>Passive:</b></i></span> Learn to recruit <i>Fire Guardians</i>. Fire Guardians gain +100% damage and XP\n               while adjacent to you, but dissipate at the end of each scenario.",
        },
        -------------------------
        -- CONTINGENCY
        -------------------------
        [5] = {
            id          = "skill_contingency",
            label       = label(_"Contingency"),
            image       = "icons/contingency.png",
            description = _"<span color='#a9a150'><i><b>Passive:</b></i></span> Whenever one of your human soldiers dies, they are instead returned safely to your recall list.",
        },
    },
    --###############################
    -- GROUP 4 SKILLS
    --###############################
    [4] = {
        -------------------------
        -- FIREBALL4
        -------------------------
        [1] = {
            id          = "skill_fireball4",
            label       = label(_"Fireball"),
            image       = "attacks/fireball.png",
            description = _"<span color='#ad6a61'><i><b>Attack:</b></i></span> Ranged 18x4 fire, <i>magical</i>.",
        },
        -------------------------
        -- LIGHTNING
        -------------------------
        [2] = {
            id          = "skill_lightning",
            label       = label(_"Chain Lightning"),
            image       = "attacks/lightning.png",
            description = _"<span color='#ad6a61'><i><b>Attack:</b></i></span> Ranged 14x4 fire, <i>magical</i>. If this attack kills an enemy, you may attack again.",
        },
        -------------------------
        -- TIME DILATION
        -------------------------
        [3] = {
            id          = "skill_time_dilation",
            label       = label(_"Time Dilation"),
            image       = "icons/time-dilation.png",
            description = _"<span color='#6ca364'><i><b>Spell:</b></i></span> Spend <span color='#00bbe6'><i>48xp</i></span> to grant yourself and all allies double movement and a second attack this turn.\n           When this turn ends, affected units become slowed.",
            xp_cost=48, --XP=48 is also used in S04
        },
        -------------------------
        -- CATACLYSM
        -------------------------
        [4] = {
            id          = "skill_cataclysm",
            label       = label(_"Cataclysm"),
            image       = "icons/cataclysm.png",
            description = _"<span color='#6ca364'><i><b>Spell:</b></i></span> Spend <span color='#00bbe6'><i>99xp</i></span> and <span color='#c06a61'><i>your attack</i></span> to injure everyone in a 5-hex radius for ~75% of their\n           current HP. Dries water, melts snow, burns forest, and levels castles/villages.",
            xp_cost=99, atk_cost=1,
        },
    },
}
--###############################
-- LOCKED INDICATOR
--###############################
local locked = {
    id          = "skill_locked",
    label       = label("<span color='grey'>Locked</span>"),
    image       = "icons/locked.png",
    description = "<span color='grey'>This option is not available yet.</span>",
}






























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

    -------------------------
    -- HEADER
    -------------------------
    table.insert( grid[2], T.row{ T.column{ border="bottom", border_size=15, T.image{  label="icons/banner1.png"  }}} )
    local spacer = "                                                                  "
    local                title_text = selecting and _"Select Delfador’s Spells"       or _"Cast Delfador’s Spells"
    if (apprentice) then title_text = selecting and _"Select the Apprentice’s Spells" or _"Cast the Apprentice’s Spells" end
    table.insert( grid[2], T.row{ T.column{ T.label{
        definition="title",
        horizontal_alignment="center",
        label = spacer..title_text..spacer,
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
    for i=0,#skills,1 do if (i>delfador.level) then skills[i]=nil end end -- don't show skill groups if underleveled
    for i=0,#skills,1 do
        -- lock skills
        for j=1,#skills[i],1 do
            if (not wml.variables[ "unlock_"..string.sub(skills[i][j].id,7,-1) ]) then skills[i][j]=locked end
        end

        local button
        local subskill_row
        if (selecting) then
            -- menu button for selecting skills
            button = T.menu_button{  id="button"..i, use_markup=true  }
            for j=1,#skills[i],1 do
                table.insert( button[2], T.option{label=skills[i][j].label} )
            end
        else -- button for casting spells, or label for displaying skills
            for j=1,#skills[i],1 do
                local skill = skills[i][j]
                if (wml.variables[skill.id]) then
                    if (not skill.xp_cost) then button=T.label{  id="button"..i, use_markup=true, label=skill.label }
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
            T.column{                                 T.label{label="  "}},  T.column{  horizontal_alignment="left", T.image{id="image"..i                }  },
            T.column{ border="right", border_size=15, T.label{label="  "}},  T.column{  horizontal_alignment="left", T.label{id="label"..i,use_markup=true}  },
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
        for i,group in pairs(skills) do
            button = dialog2["button"..i]

            -- menu callbacks for selecting skills
            if (selecting) then
                -- default to whatever skill we had selected last time
                for j,skill in pairs(skills[i]) do
                    if (wml.variables[skill.id]) then button.selected_index=j end
                end

                -- whenever we refresh the menu, update the image and label
                refresh = function(button)
                    if (not skills[i][1]) then return end
                    dialog2["image"..i].label = skills[i][button.selected_index].image
                    dialog2["label"..i].label = skills[i][button.selected_index].description

                    -- also update variables
                    for j,skill in pairs(skills[i]) do
                        result_table[skill.id] = (j==button.selected_index) and "yes" or "no"
                        if (skill.id=="skill_locked") then result_table[skill.id]="no" end
                    end
                end

                -- refresh immediately, and after any change
                refresh(button)
                button.on_modified = refresh

            -- fixed labels for casting/displaying skills/spells
            else dialog2["button"..i].visible = false
                for j,skill2 in pairs(skills[i]) do -- can't name this variable "skill" or we get a warning about being shadowed by the "skill" defined on 503
                    if (not wml.variables[skill2.id]) then goto continue end

                    -- if we know this skill, reveal and initialize the UI
                    dialog2["button"..i].visible = true
                    dialog2["image" ..i].label = skill2.image
                    dialog2["label" ..i].label = skill2.description

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
                            elseif (skill.atk_cost and skill.atk_cost>delfador.attacks_left) then
                                dialog2[buttonid].label = small and _"<span size='small'>No Attack</span>" or label('No Attack')
                                dialog2[buttonid].enabled = false

                            -- cast spell
                            else
                                dialog2[buttonid].on_button_click = function()
                                    if (skill.xp_cost)  then delfador.experience  =delfador.experience  -skill.xp_cost  end
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
