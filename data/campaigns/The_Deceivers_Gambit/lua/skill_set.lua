local _ = wesnoth.textdomain "wesnoth-tdg"

--###########################################################################################################################################################
--                                                                  DEFINE SKILLS
--###########################################################################################################################################################
function label(text)     return "<span size='1000'> \n</span><span size='large'>"..text.."</span><span size='8000'>\n </span>"  end
local skill_set = {
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
            description = _"<span color='#6ca364'><i><b>Spell:</b></i></span> Spend <span color='#00bbe6'><i>16xp</i></span> to <i>disallow magical attacks</i> in a 3-hex radius, until cancelled.\n           Disables Delfador’s spells, but not his passive skills.",
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

return {
    --###############################
    -- LOCKED INDICATOR
    --###############################
    locked = {
    id          = "skill_locked",
    label       = label("<span color='grey'>Locked</span>"),
    image       = "icons/locked.png",
    description = "<span color='grey'>This option is not available yet.</span>",
    },
    skill_set = skill_set,
}
