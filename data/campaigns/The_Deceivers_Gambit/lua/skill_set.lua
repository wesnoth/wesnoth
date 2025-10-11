local _ = wesnoth.textdomain "wesnoth-tdg"

--###########################################################################################################################################################
--                                                                  DEFINE SKILLS
--###########################################################################################################################################################
function label(text)      return "<span size='1000'> \n</span><span size='large'>"..text.."</span><span size='8000'>\n </span>"  end
function header_attack()  return "<span color='#ad6a61' style='italic' weight='bold'>".._"Attack:" .." </span>"  end
function header_spell()   return "<span color='#6ca364' style='italic' weight='bold'>".._"Spell:"  .." </span>"  end
function header_passive() return "<span color='#a9a150' style='italic' weight='bold'>".._"Passive:".." </span>"  end
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
            description = header_attack().._"Ranged 7x3 fire, <i><ref dst='weaponspecial_magical'>magical</ref></i>.",
        },
        -------------------------
        -- SHIELD
        -------------------------
        [2] = {
            id          = "skill_shield",
            label       = label(_"Shield"),
            image       = "icons/shield.png",
            description = header_spell().._"Spend <span color='#00bbe6' style='italic'>8xp</span> to gain <i>+20% dodge chance</i> until the start of your next turn or until cancelled.",
            xp_cost=8, --XP is also used in S04
        },
        -------------------------
        -- PANACEA
        -------------------------
        [3] = {
            id          = "skill_panacea",
            label       = label(_"Panacea"),
            image       = "icons/potion_green_small.png",
            description = header_spell().._"Spend <span color='#00bbe6' style='italic'>8xp</span> to fully heal the lowest-health adjacent human ally, and increase its\n          attacks, strikes, and damage by its level. <span color='#dd0000' weight='bold'>Next turn, it dies.</span>",
            xp_cost=8, --XP is also used in spellcasting.cfg
        },
        -------------------------
        -- ANIMATE MUD
        -------------------------
        [4] = {
            id          = "skill_animate_mud",
            label       = label(_"Animate Mud"),
            image       = "icons/animate-mud.png",
            description = header_passive().._"Learn to recruit <i>Mudcrawlers</i>. Mudcrawlers gain +100% damage and xp\n               while adjacent to you, but dissolve at the end of each scenario.",
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
            description = header_attack().._"Melee 6x3 cold, <i><ref dst='weaponspecial_slows'>slows</ref></i>. Replaces your default melee attack.",
        },
        -------------------------
        -- LEVITATE
        -------------------------
        [2] = {
            id          = "skill_levitate",
            label       = label(_"Levitate"),
            image       = "icons/levitate.png",
            description = header_spell().._"Spend <span color='#00bbe6' style='italic'>8xp</span> to gain <i>flight</i> and the <i><ref dst='ability_skirmisherskirmisher'>skirmisher</ref></i> ability until the start of your next turn or until cancelled.",
            xp_cost=8, --XP=8 is also used in S04
        },
        -------------------------
        -- FIND FAMILIAR
        -------------------------
        [3] = {
            id          = "skill_find_familiar",
            label       = label(_"Find Familiar"),
            image       = "icons/find-familiar.png",
            description = header_passive().._"Begin each scenario with your trusty pet raven.\n               Your familiar’s level and xp persist across scenarios, but reset if it dies.",
        },
        -------------------------
        -- MNEMONIC
        -------------------------
        [4] = {
            id          = "skill_mnemonic",
            label       = label(_"Mnemonic"),
            image       = "icons/mnemonic.png",
            description = header_passive().._"Whenever an adjacent ally gains xp, you gain the same amount of xp.",
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
            description = header_attack().._"Ranged 8x4 fire, <i><ref dst='weaponspecial_magical'>magical</ref></i>.",
        },
        -------------------------
        -- ENERVATE
        -------------------------
        [2] = {
            id          = "skill_enervate",
            label       = label(_"Siphon"),
            image       = "icons/enervate.png", -- better than fireball2 vs orcs or undead, but sarians resist arcane and are vulnerable to fire. You also get this a few scenarios later than fireball2.
            description = header_attack().._"Ranged 8x4 arcane, <i><ref dst='weaponspecial_magical'>magical</ref></i> and <i><ref dst='weaponspecial_drains'>drains</ref></i>.",
        },
        -------------------------
        -- BLIZZARD
        -------------------------
        [3] = {
            id          = "skill_blizzard",
            label       = label(_"Blizzard"),
            image       = "icons/blizzard.png",
            description = header_spell().._"Spend <span color='#00bbe6' style='italic'>16xp</span> and <span color='#c06a61' style='italic'>your attack</span> to <i><ref dst='weaponspecial_slows'>slow</ref></i> enemy units and freeze terrain in a 3-hex radius.",
            xp_cost=16, atk_cost=1,
        },
        -------------------------
        -- COUNTERSPELL
        -------------------------
        [4] = {
            id          = "skill_counterspell",
            label       = label(_"Counterspell"),
            image       = "icons/counterspell.png",
            description = header_spell().._"Spend <span color='#00bbe6' style='italic'>16xp</span> to <i>disallow <ref dst='weaponspecial_magical'>magical</ref> attacks</i> in a 3-hex radius, until cancelled.\n          Disables Delfador’s spells, but not his passive skills.",
            xp_cost=16, --XP=16 is also used in S04
        },
        -------------------------
        -- POLYMORPH
        -------------------------
        [5] = {
            id          = "skill_polymorph",
            label       = label(_"Polymorph"),
            image       = "icons/polymorph.png",
            description = header_spell().._"Transform into a stoat (<span color='#00bbe6' style='italic'>1xp</span>), bear (<span color='#00bbe6' style='italic'>8xp</span>), crab (<span color='#00bbe6' style='italic'>16xp</span>), or roc (<span color='#00bbe6' style='italic'>32xp</span>). Lasts until cancelled.\n           Replaces Delfador’s attacks, spells, and passives, but does not affect hitpoints.",
            subskills   = {
                [1]={ id="skill_polymorph_stoat",  xp_cost=1,  label="   <span>".._"Stoat".." (<span color='#00bbe6' style='italic'>".._"1xp" .."</span>)</span>   " },
                [2]={ id="skill_polymorph_bear",   xp_cost=8,  label="   <span>".._"Bear" .." (<span color='#00bbe6' style='italic'>".._"8xp" .."</span>)</span>   " },
                [3]={ id="skill_polymorph_crab",   xp_cost=16, label="   <span>".._"Crab" .." (<span color='#00bbe6' style='italic'>".._"16xp".."</span>)</span>   " },
                [4]={ id="skill_polymorph_roc",    xp_cost=32, label="   <span>".._"Roc"  .." (<span color='#00bbe6' style='italic'>".._"32xp".."</span>)</span>   " }, },
        },
        -------------------------
        -- GLAMOUR
        -------------------------
        [6] = {
            id          = "skill_glamour",
            label       = label(_"Glamour"),
            image       = "icons/glamour.png",
            description = header_passive().._"Gain the <i><ref dst='ability_leadershipleadership'>leadership</ref></i> ability.",
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
            description = header_attack().._"Ranged 12x4 fire, <i><ref dst='weaponspecial_magical'>magical</ref></i>.",
        },
        -------------------------
        -- DANCING DAGGERS
        -------------------------
        [2] = {
            id          = "skill_dancing_daggers",
            label       = label(_"Dancing Daggers"),
            image       = "icons/dancing-daggers.png",
            description = header_attack().._"Ranged 5x8 blade, <i><ref dst='weaponspecial_backstab'>backstab</ref></i>",
        },
        -------------------------
        -- ILLUSION
        -------------------------
        [3] = {
            id          = "skill_illusion",
            label       = label(_"Enthrall"),
            image       = "icons/illusion.png",
            description = header_spell().._"Spend <span color='#00bbe6' style='italic'>48xp</span> and <span color='#c06a61' style='italic'>your attack</span> to magically disguise yourself as an awe-inspiring drake,\n          reducing accuracy and dodge by 10% for enemies in a 2 hex radius. Lasts until cancelled.",
            xp_cost=48, atk_cost=1,
        },
        -------------------------
        -- ANIMATE FIRE
        -------------------------
        [4] = {
            id          = "skill_animate_fire",
            label       = label(_"Animate Fire"),
            image       = "icons/animate-fire.png",
            description = header_passive().._"Learn to recruit <i>Fire Guardians</i>. Fire Guardians gain +100% damage and xp\n               while adjacent to you, but dissipate at the end of each scenario.",
        },
        -------------------------
        -- CONTINGENCY
        -------------------------
        [5] = {
            id          = "skill_contingency",
            label       = label(_"Contingency"),
            image       = "icons/contingency.png",
            description = header_passive().._"Whenever one of your human soldiers dies, they are instead returned safely to your recall list.",
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
            description = header_attack().._"Ranged 18x4 fire, <i><ref dst='weaponspecial_magical'>magical</ref></i>.",
        },
        -------------------------
        -- LIGHTNING
        -------------------------
        [2] = {
            id          = "skill_lightning",
            label       = label(_"Chain Lightning"),
            image       = "attacks/lightning.png",
            description = header_attack().._"Ranged 14x4 fire, <i><ref dst='weaponspecial_magical'>magical</ref></i>. If this attack kills an enemy, you may attack again.",
        },
        -------------------------
        -- TIME DILATION
        -------------------------
        [3] = {
            id          = "skill_time_dilation",
            label       = label(_"Time Dilation"),
            image       = "icons/time-dilation.png",
            description = header_spell().._"Spend <span color='#00bbe6' style='italic'>48xp</span> to grant yourself and all allies double movement and a second attack this turn.\n          When this turn ends, affected units become <ref dst='weaponspecial_slows'>slowed</ref>.",
            xp_cost=48, --XP=48 is also used in S04
        },
        -------------------------
        -- CATACLYSM
        -------------------------
        [4] = {
            id          = "skill_cataclysm",
            label       = label(_"Cataclysm"),
            image       = "icons/cataclysm.png",
            description = header_spell().._"Spend <span color='#00bbe6' style='italic'>99xp</span> and <span color='#c06a61' style='italic'>your attack</span> to injure everyone in a 5-hex radius for ~75% of their\n          current hp. Dries water, melts snow, burns forest, and levels castles/villages.",
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
