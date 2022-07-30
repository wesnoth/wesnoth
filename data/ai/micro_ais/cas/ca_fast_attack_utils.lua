local AH = wesnoth.require "ai/lua/ai_helper.lua"
local T = wml.tag
local M = wesnoth.map

-- Functions to perform fast evaluation of attacks and attack combinations.
-- The emphasis with all of this is on speed, not elegance.
-- This might result in redundant information being produced/passed and similar
-- or the format of cache tables being somewhat tedious, etc.
-- Note to self: working with Lua tables is generally much faster than with unit proxy tables.
--   Also, creating tables takes time, indexing by number is faster than by string, etc.
-- Note 2: these utility functions are a subset of those used in the experimental grunt rush AIs
--   and therefore contain some bits that are not necessary for the Fast MAI. Some, albeit
--   likely not a significant, speed-up can therefore be obtained eventually by removing those
--   parts from the code.

local ca_fast_attack_utils = {}

local function attack_filter(which, filter, is_leader)
    if (is_leader == nil) then is_leader = false end
    if (which == 'leader') then
        which = 'own'
        is_leader = true
    end
    if (which == 'own') then
        return {
            side = wesnoth.current.side,
            canrecruit = is_leader,
            { "and", filter or {} }
        }
    elseif (which == 'enemy') then
        return {
            T.filter_side { T.enemy_of { side = wesnoth.current.side } },
            { "and", filter or {} }
        }
    else
        return filter
    end
end

ca_fast_attack_utils.build_attack_filter = attack_filter

function ca_fast_attack_utils.gamedata_setup()
    -- Keep game data in a table for faster access.
    -- This is currently re-done on every move. Could be optimized by only
    -- updating changes, if this is found to be critical (likely not needed).

    local gamedata = {}

    local village_map = {}
    for _,village in ipairs(wesnoth.map.find{gives_income = true}) do
        if (not village_map[village[1]]) then village_map[village[1]] = {} end
        village_map[village[1]][village[2]] = true
    end
    gamedata.village_map = village_map

    local healing_map = {}
    for _,loc in ipairs(AH.get_healing_locations {}) do
        if (not healing_map[loc[1]]) then healing_map[loc[1]] = {} end
        healing_map[loc[1]][loc[2]] = wesnoth.terrain_types[wesnoth.current.map[loc]].healing
    end
    gamedata.healing_map = healing_map

    -- Only uses one leader per side right now, but only used for finding direction
    -- of move -> sufficient for this.
    gamedata.leaders = {}
    for _,unit_proxy in ipairs(AH.get_live_units { canrecruit = 'yes' }) do
        gamedata.leaders[unit_proxy.side] = { unit_proxy.x, unit_proxy.y, id = unit_proxy.id }
    end

    -- Tables that will only be populated as needed:
    gamedata.unit_copies = {}
    gamedata.unit_infos = {}
    gamedata.defense_maps = {}

    return gamedata
end

function ca_fast_attack_utils.single_unit_info(unit_proxy)
    -- Collects unit information from proxy unit table @unit_proxy into a Lua table
    -- so that it is accessible faster.
    -- Note: Even accessing the directly readable fields of a unit proxy table
    -- is slower than reading from a smaller Lua table.
    --
    -- Important: this is slow, so it should only be called as needed,
    -- but it does need to be redone after each move, as it contains
    -- information like HP and XP (or the unit might have leveled up or been changed
    -- in an event).

    local single_unit_info = {
        id = unit_proxy.id,
        canrecruit = unit_proxy.canrecruit,
        side = unit_proxy.side,

        x = unit_proxy.x,
        y = unit_proxy.y,

        hitpoints = unit_proxy.hitpoints,
        max_hitpoints = unit_proxy.max_hitpoints,
        experience = unit_proxy.experience,
        max_experience = unit_proxy.max_experience,

        cost = unit_proxy.cost,
        level = unit_proxy.level
    }

    -- Include the ability type, such as: hides, heals, regenerate, skirmisher (set up as 'hides = true')
    local abilities = wml.get_child(unit_proxy.__cfg, "abilities")
    if abilities then
        for _,ability in ipairs(abilities) do
            if ability[1] == 'regenerate' then
                single_unit_info[ability[1]] = ability[2].value
            else
                single_unit_info[ability[1]] = true
            end
        end
    end

    return single_unit_info
end

function ca_fast_attack_utils.get_unit_info(unit_proxy, gamedata)
    -- Get unit_info for a unit; read from gamedata if it has been claculated
    -- already, otherwise store in there

    if (not gamedata.unit_infos[unit_proxy.id]) then
        gamedata.unit_infos[unit_proxy.id] = ca_fast_attack_utils.single_unit_info(unit_proxy)
    end

    return gamedata.unit_infos[unit_proxy.id]
end

function ca_fast_attack_utils.get_unit_copy(id, gamedata)
    -- Get copy of a unit; read from gamedata if it has been claculated
    -- already, otherwise store in there

    if (not gamedata.unit_copies[id]) then
        local unit_proxy = wesnoth.units.find_on_map { id = id }[1]
        gamedata.unit_copies[id] = unit_proxy:clone()
    end

    return gamedata.unit_copies[id]
end

function ca_fast_attack_utils.get_unit_defense(unit_copy, x, y, defense_maps)
    -- Get the terrain defense of a unit as a factor (that is, e.g. 0.40 rather than 40)
    -- The result is stored (or accessed, if it exists) in @defense_maps
    --
    -- Inputs:
    -- @unit_copy: private copy of the unit (proxy table works too, but is slower)
    -- @x, @y: the location for which to calculate the unit's terrain defense
    -- @defense_maps: table in which to cache the results. Note: this is NOT an optional input
    --
    -- Sample structure of defense_maps:
    --   defense_maps['Vanak'][19][4] = 0.6

    if (not defense_maps[unit_copy.id]) then defense_maps[unit_copy.id] = {} end
    if (not defense_maps[unit_copy.id][x]) then defense_maps[unit_copy.id][x] = {} end

    if (not defense_maps[unit_copy.id][x][y]) then
        local defense = unit_copy:defense_on(wesnoth.current.map[{x, y}]) / 100.
        defense_maps[unit_copy.id][x][y] = { defense = defense }
    end

    return defense_maps[unit_copy.id][x][y].defense
end

function ca_fast_attack_utils.is_acceptable_attack(damage_taken, damage_done, aggression)
    -- Evaluate whether an attack is acceptable, based on the damage taken/done ratio
    --
    -- Inputs:
    -- @damage_taken, @damage_done: should be in gold units as returned by ca_fast_attack_utils.attack_rating
    --   This could be either the attacker (for taken) and defender (for done) rating of a single attack (combo)
    --   or the overall attack (for done) and counter attack rating (for taken)
    -- @aggression: value determining which ratio of damage done/taken that is acceptable

    -- Otherwise it depends on whether the numbers are positive or negative
    -- Negative damage means that one or several of the units are likely to level up
    if (damage_taken < 0) and (damage_done < 0) then
        return (damage_done / damage_taken) >= (1 - aggression)
    end

    if (damage_taken <= 0) then damage_taken = 0.001 end
    if (damage_done <= 0) then damage_done = 0.001 end

    return (damage_done / damage_taken) >= (1 - aggression)
end

function ca_fast_attack_utils.damage_rating_unit(attacker_info, defender_info, att_stat, def_stat, is_village, healing, cfg)
    -- Calculate the rating for the damage received by a single attacker on a defender.
    -- The attack att_stat both for the attacker and the defender need to be precalculated for this.
    -- Unit information is passed in unit_infos format, rather than as unit proxy tables for speed reasons.
    -- Note: this is _only_ the damage rating for the attacker, not both units
    -- Note: damage is damage TO the attacker, not damage done BY the attacker
    --
    -- Input parameters:
    --  @attacker_info, @defender_info: unit_info tables produced by ca_fast_attack_utils.get_unit_info()
    --  @att_stat, @def_stat: attack statistics for the two units
    --  @is_village: whether the hex from which the attacker attacks is a village
    --    Set to nil or false if not, to anything if it is a village (does not have to be a boolean)
    --  @healing: the amount of healing given by the hex from which the attacker attacks
    --    Set to nil or false if there is no healing
    --
    -- Optional parameters:
    --  @cfg: the optional weights listed right below (currently only leader_weight)

    local leader_weight = (cfg and cfg.leader_weight) or 2.

    local damage = attacker_info.hitpoints - att_stat.average_hp

    -- Count poisoned as additional damage done by poison times probability of being poisoned
    if (att_stat.poisoned ~= 0) then
        damage = damage + wesnoth.game_config.poison_amount * (att_stat.poisoned - att_stat.hp_chance[0])
    end
    -- Count slowed as additional 4 HP damage times probability of being slowed
    if (att_stat.slowed ~= 0) then
        damage = damage + 4 * (att_stat.slowed - att_stat.hp_chance[0])
    end

    -- If attack is from a healing location, count its healing_value
    if healing then
        damage = damage - healing
    -- Otherwise only: if attacker can regenerate, add the regeneration bonus
    else
        damage = damage - (attacker_info.regenerate or 0)
    end

    if (damage < 0) then damage = 0 end

    -- Fractional damage (= fractional value of the attacker)
    local fractional_damage = damage / attacker_info.max_hitpoints

    -- Additionally, subtract the chance to die, in order to (de)emphasize units that might die
    fractional_damage = fractional_damage + att_stat.hp_chance[0]

    -- In addition, potentially leveling up in this attack is a huge bonus.
    -- we reduce the fractions damage by the chance of it happening multiplied
    -- by the chance of not dying itself.
    -- Note: this can make the fractional damage negative (as it should)
    local defender_level = defender_info.level

    local level_bonus = 0.
    if (attacker_info.max_experience - attacker_info.experience <= defender_level * wesnoth.game_config.combat_experience) then
        level_bonus = 1. - att_stat.hp_chance[0]
    elseif (attacker_info.max_experience - attacker_info.experience <= defender_level * wesnoth.game_config.kill_experience) then
        if (defender_level == 0) then defender_level = 0.5 end
        level_bonus = (1. - att_stat.hp_chance[0]) * def_stat.hp_chance[0]
    end

    fractional_damage = fractional_damage - level_bonus

    -- Now convert this into gold-equivalent value
    local value = attacker_info.cost

    -- If this is the side leader, make damage to it much more important
    if attacker_info.canrecruit then
        value = value * leader_weight
    end

    -- Being closer to leveling makes the attacker more valuable
    local xp_bonus = attacker_info.experience / attacker_info.max_experience
    value = value * (1. + xp_bonus)

    local rating = fractional_damage * value

    return rating
end

function ca_fast_attack_utils.attack_rating(attacker_infos, defender_info, dsts, att_stats, def_stat, gamedata, cfg)
    -- Returns a common (but configurable) rating for attacks of one or several attackers against one defender
    --
    -- Inputs:
    --  @attackers_infos: input array of attacker unit_info tables (must be an array even for single unit attacks)
    --  @defender_info: defender unit_info
    --  @dsts: array of attack locations in form { x, y } (must be an array even for single unit attacks)
    --  @att_stats: array of the attack stats of the attack combination(!) of the attackers
    --    (must be an array even for single unit attacks)
    --  @def_stat: the combat stats of the defender after facing the combination of the attackers
    --  @gamedata: table with the game state as produced by ca_fast_attack_utils.gamedata()
    --
    -- Optional inputs:
    --  @cfg: table with optional configuration parameters:
    --    - aggression: the default aggression aspect, determining how to balance own vs. enemy damage
    --    - leader_weight: to be passed on to damage_rating_unit()
    --
    -- Returns:
    --   - Overall rating for the attack or attack combo
    --   - Attacker rating: the sum of all the attacker damage ratings
    --   - Defender rating: the combined defender damage rating
    --   - Extra rating: additional ratings that do not directly describe damage
    --       This should be used to help decide which attack to pick,
    --       but not for, e.g., evaluating counter attacks (which should be entirely damage based)
    --   Note: rating = defender_rating - attacker_rating * (1 - aggression) + extra_rating

    -- Set up the config parameters for the rating
    local aggression = (cfg and cfg.aggression) or 0.4

    local attacker_rating = 0
    for i,attacker_info in ipairs(attacker_infos) do
        local attacker_on_village = gamedata.village_map[dsts[i][1]]
            and gamedata.village_map[dsts[i][1]][dsts[i][2]]
        local attacker_healing = gamedata.healing_map[dsts[i][1]]
            and gamedata.healing_map[dsts[i][1]][dsts[i][2]]
        attacker_rating = attacker_rating + ca_fast_attack_utils.damage_rating_unit(
            attacker_info, defender_info, att_stats[i], def_stat, attacker_on_village, attacker_healing, cfg
        )
    end

    -- attacker_info is passed only to figure out whether the attacker might level up
    -- TODO: This is only works for the first attacker in a combo at the moment
    local defender_x, defender_y = defender_info.x, defender_info.y
    local defender_on_village = gamedata.village_map[defender_x]
        and gamedata.village_map[defender_x][defender_y]
    local defender_healing = gamedata.healing_map[defender_x]
        and gamedata.healing_map[defender_x][defender_y]
    local defender_rating = ca_fast_attack_utils.damage_rating_unit(
        defender_info, attacker_infos[1], def_stat, att_stats[1], defender_on_village, defender_healing, cfg
    )

    -- Now we add some extra ratings. They are positive for attacks that should be preferred
    -- and expressed in fraction of the defender maximum hitpoints
    -- They should be used to help decide which attack to pick all else being equal,
    -- but not for, e.g., evaluating counter attacks (which should be entirely damage based)
    local extra_rating = 0.

    -- Prefer to attack already damaged enemies
    local defender_starting_damage_fraction = defender_info.max_hitpoints - defender_info.hitpoints
    extra_rating = extra_rating + defender_starting_damage_fraction * 0.33

    -- If defender is on a village, add a bonus rating (we want to get rid of those preferentially)
    -- This is in addition to the healing bonus already included above (but as an extra rating)
    if defender_on_village then
        extra_rating = extra_rating + 10.
    end

    -- Normalize so that it is in fraction of defender max_hitpoints
    extra_rating = extra_rating / defender_info.max_hitpoints

    -- We don't need a bonus for good terrain for the attacker, as that is covered in the damage calculation
    -- However, we add a small bonus for good terrain defense of the _defender_ on the _attack_ hexes
    -- This is in order to take good terrain away from defender on its next move
    local defense_rating = 0.
    for _,dst in ipairs(dsts) do
        defense_rating = defense_rating + ca_fast_attack_utils.get_unit_defense(
            ca_fast_attack_utils.get_unit_copy(defender_info.id, gamedata),
            dst[1], dst[2],
            gamedata.defense_maps
        )
    end
    defense_rating = defense_rating / #dsts * 0.1

    extra_rating = extra_rating + defense_rating

    -- Get a very small bonus for hexes in between defender and AI leader
    -- 'relative_distances' is larger for attack hexes closer to the side leader (possible values: -1 .. 1)
    if gamedata.leaders[attacker_infos[1].side] then
        local leader_x, leader_y = gamedata.leaders[attacker_infos[1].side][1], gamedata.leaders[attacker_infos[1].side][2]

        local rel_dist_rating = 0.
        for _,dst in ipairs(dsts) do
            local relative_distance =
                M.distance_between(defender_x, defender_y, leader_x, leader_y)
                - M.distance_between(dst[1], dst[2], leader_x, leader_y)
            rel_dist_rating = rel_dist_rating + relative_distance
        end
        rel_dist_rating = rel_dist_rating / #dsts * 0.002

        extra_rating = extra_rating + rel_dist_rating
    end

    -- Finally add up and apply factor of own unit weight to defender unit weight
    -- This is a number equivalent to 'aggression' in the default AI (but not numerically equal)
    local rating = defender_rating - attacker_rating * (1 - aggression) + extra_rating

    return rating, attacker_rating, defender_rating, extra_rating
end

function ca_fast_attack_utils.battle_outcome(attacker_copy, defender_proxy, dst, attacker_info, defender_info, gamedata, move_cache)
    -- Calculate the stats of a combat by @attacker_copy vs. @defender_proxy at location @dst
    -- We use wesnoth.simulate_combat for this, but cache results when possible
    -- Inputs:
    -- @attacker_copy: private unit copy of the attacker (must be a copy, does not work with the proxy table)
    -- @defender_proxy: defender proxy table (must be a unit proxy table on the map, does not work with a copy)
    -- @dst: location from which the attacker will attack in form { x, y }
    -- @attacker_info, @defender_info: unit info for the two units (needed in addition to the units
    --   themselves in order to speed things up)
    --  @gamedata: table with the game state as produced by ca_fast_attack_utils.gamedata()
    --  @move_cache: for caching data *for this move only*, needs to be cleared after a gamestate change

    local defender_defense = ca_fast_attack_utils.get_unit_defense(defender_proxy, defender_proxy.x, defender_proxy.y, gamedata.defense_maps)
    local attacker_defense = ca_fast_attack_utils.get_unit_defense(attacker_copy, dst[1], dst[2], gamedata.defense_maps)

    if move_cache[attacker_info.id]
        and move_cache[attacker_info.id][defender_info.id]
        and move_cache[attacker_info.id][defender_info.id][attacker_defense]
        and move_cache[attacker_info.id][defender_info.id][attacker_defense][defender_defense]
        and move_cache[attacker_info.id][defender_info.id][attacker_defense][defender_defense][attacker_info.hitpoints]
        and move_cache[attacker_info.id][defender_info.id][attacker_defense][defender_defense][attacker_info.hitpoints][defender_info.hitpoints]
    then
        return move_cache[attacker_info.id][defender_info.id][attacker_defense][defender_defense][attacker_info.hitpoints][defender_info.hitpoints].att_stat,
            move_cache[attacker_info.id][defender_info.id][attacker_defense][defender_defense][attacker_info.hitpoints][defender_info.hitpoints].def_stat
    end

    local old_x, old_y = attacker_copy.x, attacker_copy.y
    attacker_copy.x, attacker_copy.y = dst[1], dst[2]
    local tmp_att_stat, tmp_def_stat = wesnoth.simulate_combat(attacker_copy, defender_proxy)
    attacker_copy.x, attacker_copy.y = old_x, old_y

    -- Extract only those hp_chances that are non-zero (except for hp_chance[0]
    -- which is always needed). This slows down this step a little, but significantly speeds
    -- up attack combination calculations
    local att_stat = {
        hp_chance = {},
        average_hp = tmp_att_stat.average_hp,
        poisoned = tmp_att_stat.poisoned,
        slowed = tmp_att_stat.slowed
    }

    att_stat.hp_chance[0] = tmp_att_stat.hp_chance[0]
    for i = 1,#tmp_att_stat.hp_chance do
        if (tmp_att_stat.hp_chance[i] ~= 0) then
            att_stat.hp_chance[i] = tmp_att_stat.hp_chance[i]
        end
    end

    local def_stat = {
        hp_chance = {},
        average_hp = tmp_def_stat.average_hp,
        poisoned = tmp_def_stat.poisoned,
        slowed = tmp_def_stat.slowed
    }

    def_stat.hp_chance[0] = tmp_def_stat.hp_chance[0]
    for i = 1,#tmp_def_stat.hp_chance do
        if (tmp_def_stat.hp_chance[i] ~= 0) then
            def_stat.hp_chance[i] = tmp_def_stat.hp_chance[i]
        end
    end

    if (not move_cache[attacker_info.id]) then
        move_cache[attacker_info.id] = {}
    end
    if (not move_cache[attacker_info.id][defender_info.id]) then
        move_cache[attacker_info.id][defender_info.id] = {}
    end
    if (not move_cache[attacker_info.id][defender_info.id][attacker_defense]) then
        move_cache[attacker_info.id][defender_info.id][attacker_defense] = {}
    end
    if (not move_cache[attacker_info.id][defender_info.id][attacker_defense][defender_defense]) then
        move_cache[attacker_info.id][defender_info.id][attacker_defense][defender_defense] = {}
    end
    if (not move_cache[attacker_info.id][defender_info.id][attacker_defense][defender_defense][attacker_info.hitpoints]) then
        move_cache[attacker_info.id][defender_info.id][attacker_defense][defender_defense][attacker_info.hitpoints] = {}
    end

    move_cache[attacker_info.id][defender_info.id][attacker_defense][defender_defense][attacker_info.hitpoints][defender_info.hitpoints]
        = { att_stat = att_stat, def_stat = def_stat }

    return att_stat, def_stat
end

return ca_fast_attack_utils
