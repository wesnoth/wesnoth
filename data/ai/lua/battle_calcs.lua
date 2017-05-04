local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "location_set"

-- This is a collection of Lua functions used for custom AI development.
-- Note that this is still work in progress with significant changes occurring
-- frequently. Backward compatibility cannot be guaranteed at this time in
-- development releases, but it is of course easily possible to copy a function
-- from a previous release directly into an add-on if it is needed there.

local battle_calcs = {}

function battle_calcs.unit_attack_info(unit, cache)
    -- Return a table containing information about attack-related properties of @unit
    -- The result can be cached if variable @cache is given
    -- This is done in order to avoid duplication of slow processes, such as access to unit.__cfg

    -- Return table has fields:
    --  - attacks: the attack tables from unit.__cfg
    --  - resist_mod: resistance modifiers (multiplicative factors) index by attack type
    --  - alignment: just that

    -- Set up a cache index. We use id+max_hitpoints+side, since the
    -- unit can level up. Side is added to avoid the problem of MP leaders sometimes having
    -- the same id when the game is started from the command-line
    local cind = 'UI-' .. unit.id .. unit.max_hitpoints .. unit.side

    -- If cache for this unit exists, return it
    if cache and cache[cind] then
        return cache[cind]
    end

    -- Otherwise collect the information
    local unit_cfg = unit.__cfg
    local unit_info = {
        attacks = {},
        resist_mod = {},
        alignment = unit_cfg.alignment
    }
    for attack in H.child_range(unit_cfg, 'attack') do
        -- Extract information for specials; we do this first because some
        -- custom special might have the same name as one of the default scalar fields
        local a = {}
        for special in H.child_range(attack, 'specials') do
            for _,sp in ipairs(special) do
                if (sp[1] == 'damage') then  -- this is 'backstab'
                    if (sp[2].id == 'backstab') then
                        a.backstab = true
                    else
                        if (sp[2].id == 'charge') then a.charge = true end
                    end
                else
                    -- magical, marksman
                    if (sp[1] == 'chance_to_hit') then
                        a[sp[2].id] = true
                    else
                        a[sp[1]] = true
                    end
                end
            end
        end

        -- Now extract the scalar (string and number) values from attack
        for k,v in pairs(attack) do
            if (type(v) == 'number') or (type(v) == 'string') then
                a[k] = v
            end
        end

        -- [attack]number= defaults to zero; must be defined for battle_calcs.best_weapons()
        a.number = a.number or 0

        table.insert(unit_info.attacks, a)
    end

    local attack_types = { "arcane", "blade", "cold", "fire", "impact", "pierce" }
    for _,attack_type in ipairs(attack_types) do
        unit_info.resist_mod[attack_type] = wesnoth.unit_resistance(unit, attack_type) / 100.
    end

    if cache then cache[cind] = unit_info end

    return unit_info
end

function battle_calcs.strike_damage(attacker, defender, att_weapon, def_weapon, dst, cache)
    -- Return the single strike damage of an attack by @attacker on @defender
    -- Also returns the other information about the attack (since we're accessing the information already anyway)
    -- Here, @att_weapon and @def_weapon are the weapon numbers in Lua counts, i.e., counts start at 1
    -- If @def_weapon = 0, return 0 for defender damage
    -- This can be used for defenders that do not have the right kind of weapon, or if
    -- only the attacker damage is of interest
    -- @dst: attack location, to take terrain time of day, illumination etc. into account
    -- For the defender, the current location is assumed
    --
    -- 'cache' can be given to cache strike damage and to pass through to battle_calcs.unit_attack_info()

    -- Set up a cache index. We use id+max_hitpoints+side for each unit, since the
    -- unit can level up.
    -- Also need to add the weapons and lawful_bonus values for each unit
    local att_lawful_bonus = wesnoth.get_time_of_day({ dst[1], dst[2], true }).lawful_bonus
    local def_lawful_bonus = wesnoth.get_time_of_day({ defender.x, defender.y, true }).lawful_bonus

    local cind = 'SD-' .. attacker.id .. attacker.max_hitpoints .. attacker.side
    cind = cind .. 'x' .. defender.id .. defender.max_hitpoints .. defender.side
    cind = cind .. '-' .. att_weapon .. 'x' .. def_weapon
    cind = cind .. '-' .. att_lawful_bonus .. 'x' .. def_lawful_bonus

    -- If cache for this unit exists, return it
    if cache and cache[cind] then
        return cache[cind].att_damage, cache[cind].def_damage, cache[cind].att_attack, cache[cind].def_attack
    end

    local attacker_info = battle_calcs.unit_attack_info(attacker, cache)
    local defender_info = battle_calcs.unit_attack_info(defender, cache)

    -- Attacker base damage
    local att_damage = attacker_info.attacks[att_weapon].damage

    -- Opponent resistance modifier
    local att_multiplier = defender_info.resist_mod[attacker_info.attacks[att_weapon].type] or 1

    -- TOD modifier
    att_multiplier = att_multiplier * AH.get_unit_time_of_day_bonus(attacker_info.alignment, att_lawful_bonus)

    -- Now do all this for the defender, if def_weapon ~= 0
    local def_damage, def_multiplier = 0, 1.
    if (def_weapon ~= 0) then
        -- Defender base damage
        def_damage = defender_info.attacks[def_weapon].damage

        -- Opponent resistance modifier
        def_multiplier = attacker_info.resist_mod[defender_info.attacks[def_weapon].type] or 1

        -- TOD modifier
        def_multiplier = def_multiplier * AH.get_unit_time_of_day_bonus(defender_info.alignment, def_lawful_bonus)
    end

    -- Take 'charge' into account
    if attacker_info.attacks[att_weapon].charge then
        att_damage = att_damage * 2
        def_damage = def_damage * 2
    end

    -- Rounding of .5 values is done differently depending on whether the
    -- multiplier is greater or smaller than 1
    if (att_multiplier > 1) then
        att_damage = H.round(att_damage * att_multiplier - 0.001)
    else
        att_damage = H.round(att_damage * att_multiplier + 0.001)
    end

    if (def_weapon ~= 0) then
        if (def_multiplier > 1) then
            def_damage = H.round(def_damage * def_multiplier - 0.001)
        else
            def_damage = H.round(def_damage * def_multiplier + 0.001)
        end
    end

    if cache then
        cache[cind] = {
            att_damage = att_damage,
            def_damage = def_damage,
            att_attack = attacker_info.attacks[att_weapon],
            def_attack = defender_info.attacks[def_weapon]
        }
    end

    return att_damage, def_damage, attacker_info.attacks[att_weapon], defender_info.attacks[def_weapon]
end

function battle_calcs.best_weapons(attacker, defender, dst, cache)
    -- Return the number (index) of the best weapons for @attacker and @defender
    -- @dst: attack location, to take terrain time of day, illumination etc. into account
    -- For the defender, the current location is assumed
    -- Ideally, we would do a full attack_rating here for all combinations,
    -- but that would take too long. So we simply define the best weapons
    -- as those that has the biggest difference between
    -- damage done and damage received (the latter divided by 2)
    -- Returns 0 if defender does not have a weapon for this range
    --
    -- 'cache' can be given to cache best weapons

    -- Set up a cache index. We use id+max_hitpoints+side for each unit, since the
    -- unit can level up.
    -- Also need to add the weapons and lawful_bonus values for each unit
    local att_lawful_bonus = wesnoth.get_time_of_day({ dst[1], dst[2], true }).lawful_bonus
    local def_lawful_bonus = wesnoth.get_time_of_day({ defender.x, defender.y, true }).lawful_bonus

    local cind = 'BW-' .. attacker.id .. attacker.max_hitpoints .. attacker.side
    cind = cind .. 'x' .. defender.id .. defender.max_hitpoints .. defender.side
    cind = cind .. '-' .. att_lawful_bonus .. 'x' .. def_lawful_bonus

    -- If cache for this unit exists, return it
    if cache and cache[cind] then
        return cache[cind].best_att_weapon, cache[cind].best_def_weapon
    end

    local attacker_info = battle_calcs.unit_attack_info(attacker, cache)
    local defender_info = battle_calcs.unit_attack_info(defender, cache)

    -- Best attacker weapon
    local max_rating, best_att_weapon, best_def_weapon = -9e99, 0, 0
    for att_weapon_number,att_weapon in ipairs(attacker_info.attacks) do
        local att_damage = battle_calcs.strike_damage(attacker, defender, att_weapon_number, 0, { dst[1], dst[2] }, cache)
        local max_def_rating, tmp_best_def_weapon = -9e99, 0
        for def_weapon_number,def_weapon in ipairs(defender_info.attacks) do
            if (def_weapon.range == att_weapon.range) then
                local def_damage = battle_calcs.strike_damage(defender, attacker, def_weapon_number, 0, { defender.x, defender.y }, cache)
                local def_rating = def_damage * def_weapon.number
                if (def_rating > max_def_rating) then
                    max_def_rating, tmp_best_def_weapon = def_rating, def_weapon_number
                end
            end
        end

        local rating = att_damage * att_weapon.number
        if (max_def_rating > -9e99) then rating = rating - max_def_rating / 2. end

        if (rating > max_rating) then
            max_rating, best_att_weapon, best_def_weapon = rating, att_weapon_number, tmp_best_def_weapon
        end
    end

    if cache then
        cache[cind] = { best_att_weapon = best_att_weapon, best_def_weapon = best_def_weapon }
    end

    return best_att_weapon, best_def_weapon
end

function battle_calcs.add_next_strike(cfg, arr, n_att, n_def, att_strike, hit_miss_counts, hit_miss_str)
    -- Recursive function that sets up the sequences of strikes (misses and hits)
    -- Each call corresponds to one strike of one of the combattants and can be
    -- either miss (value 0) or hit (1)
    --
    -- Inputs:
    -- - @cfg: config table with sub-tables att/def for the attacker/defender with the following fields:
    --   - strikes: total number of strikes
    --   - max_hits: maximum number of hits the unit can survive
    --   - firststrike: set to true if attack has firststrike special
    -- - @arr: an empty array that will hold the output table
    -- - Other parameters of for recursion purposes only and are initialized below

    -- On the first call of this function, initialize variables
    -- Counts for hits/misses by both units:
    --  - Indices 1 & 2: hit/miss for attacker
    --  - Indices 3 & 4: hit/miss for defender
    hit_miss_counts = hit_miss_counts or { 0, 0, 0, 0 }
    hit_miss_str = hit_miss_str or ''  -- string with the hit/miss sequence; for visualization only

    -- Strike counts
    --  - n_att/n_def = number of strikes taken by attacker/defender
    --  - att_strike: if true, it's the attacker's turn, otherwise it's the defender's turn
    if (not n_att) then
        if cfg.def.firststrike and (not cfg.att.firststrike) then
            n_att = 0
            n_def = 1
            att_strike = false
        else
            n_att = 1
            n_def = 0
            att_strike = true
        end
    else
        if att_strike then
            if (n_def < cfg.def.strikes) then
                n_def = n_def + 1
                att_strike = false
            else
                n_att = n_att + 1
            end
        else
            if (n_att < cfg.att.strikes) then
                n_att = n_att + 1
                att_strike = true
            else
                n_def = n_def + 1
            end
        end
    end

    -- Create both a hit and a miss
    for i = 0,1 do  -- 0:miss, 1: hit
        -- hit/miss counts and string for this call
        local tmp_hmc = AH.table_copy(hit_miss_counts)
        local tmp_hmstr = ''

        -- Flag whether the opponent was killed by this strike
        local killed_opp = false  -- Defaults to falso
        if att_strike then
            tmp_hmstr = hit_miss_str .. i  -- attacker hit/miss in string: 0 or 1
            tmp_hmc[i+1] = tmp_hmc[i+1] + 1  -- Increment hit/miss counts
            -- Set variable if opponent was killed:
            if (tmp_hmc[2] > cfg.def.max_hits) then killed_opp = true end
        -- Even values of n are strikes by the defender
        else
            tmp_hmstr = hit_miss_str .. (i+2)  -- defender hit/miss in string: 2 or 3
            tmp_hmc[i+3] = tmp_hmc[i+3] + 1  -- Increment hit/miss counts
            -- Set variable if opponent was killed:
            if (tmp_hmc[4] > cfg.att.max_hits) then killed_opp = true end
        end

        -- If we've reached the total number of strikes, add this hit/miss combination to table,
        -- but only if the opponent wasn't killed, as that would end the battle
        if (n_att + n_def < cfg.att.strikes + cfg.def.strikes) and (not killed_opp) then
            battle_calcs.add_next_strike(cfg, arr, n_att, n_def, att_strike, tmp_hmc, tmp_hmstr)
        -- Otherwise, call the next recursion level
        else
            table.insert(arr, { hit_miss_str = tmp_hmstr, hit_miss_counts = tmp_hmc })
        end
    end
end

function battle_calcs.battle_outcome_coefficients(cfg, cache)
    -- Determine the coefficients needed to calculate the hitpoint probability distribution
    -- of a given battle
    -- Inputs:
    -- - @cfg: config table with sub-tables att/def for the attacker/defender with the following fields:
    --   - strikes: total number of strikes
    --   - max_hits: maximum number of hits the unit can survive
    --   - firststrike: whether the unit has firststrike weapon special on this attack
    -- The result can be cached if variable 'cache' is given
    --
    -- Output: table with the coefficients needed to calculate the distribution for both attacker and defender
    -- First index: number of hits landed on the defender. Each of those contains an array of
    -- coefficient tables, of format:
    -- { num = value, am = value, ah = value, dm = value, dh = value }
    -- This gives one term in a sum of form:
    -- num * ahp^ah * (1-ahp)^am * dhp^dh * (1-dhp)^dm
    -- where ahp is the probability that the attacker will land a hit
    -- and dhp is the same for the defender
    -- Terms that have exponents of 0 are omitted

    -- Set up the cache id
    local cind = 'coeff-' .. cfg.att.strikes .. '-' .. cfg.att.max_hits
    if cfg.att.firststrike then cind = cind .. 'fs' end
    cind = cind .. 'x' .. cfg.def.strikes .. '-' .. cfg.def.max_hits
    if cfg.def.firststrike then cind = cind .. 'fs' end

    -- If cache for this unit exists, return it
    if cache and cache[cind] then
        return cache[cind].coeffs_att, cache[cind].coeffs_def
    end

    -- Get the hit/miss counts for the battle
    local hit_miss_counts = {}
    battle_calcs.add_next_strike(cfg, hit_miss_counts)

    -- We first calculate the coefficients for the defender HP distribution
    -- so this is sorted by the number of hits the attacker lands

    -- 'counts' is an array 4 layers deep, where the indices are the number of misses/hits
    -- are the indices in order attacker miss, attacker hit, defender miss, defender hit
    -- This is so that they can be grouped by number of attacker hits/misses, for
    -- subsequent simplification
    -- The element value is number of times we get the given combination of hits/misses
    local counts = {}
    for _,count in ipairs(hit_miss_counts) do
        local i1 = count.hit_miss_counts[1]
        local i2 = count.hit_miss_counts[2]
        local i3 = count.hit_miss_counts[3]
        local i4 = count.hit_miss_counts[4]
        if not counts[i1] then counts[i1] = {} end
        if not counts[i1][i2] then counts[i1][i2] = {} end
        if not counts[i1][i2][i3] then counts[i1][i2][i3] = {} end
        counts[i1][i2][i3][i4] = (counts[i1][i2][i3][i4] or 0) + 1
    end

    local coeffs_def = {}
    for am,v1 in pairs(counts) do  -- attacker miss count
        for ah,v2 in pairs(v1) do  -- attacker hit count
            -- Set up the exponent coefficients for attacker hits/misses
        local exp = {}  -- Array for an individual set of coefficients
            -- Only populate those indices that have exponents > 0
            if (am > 0) then exp.am = am end
        if (ah > 0) then exp.ah = ah end

            -- We combine results by testing whether they produce the same sum
            -- with two very different hit probabilities, hp1 = 0.6, hp2 = 0.137
            -- This will only happen is the coefficients add up to multiples of 1
            local sum1, sum2 = 0,0
            local hp1, hp2 = 0.6, 0.137
            for dm,v3 in pairs(v2) do  -- defender miss count
                for dh,num in pairs(v3) do  -- defender hit count
                    sum1 = sum1 + num * hp1^dh * (1 - hp1)^dm
                    sum2 = sum2 + num * hp2^dh * (1 - hp2)^dm
                end
            end

            -- Now, coefficients are set up for each value of total hits by attacker
            -- This holds all the coefficients that need to be added to get the propability
            -- of the defender receiving this number of hits
            if (not coeffs_def[ah]) then coeffs_def[ah] = {} end

            -- If sum1 and sum2 are equal, that means all the defender probs added up to 1, or
            -- multiple thereof, which means the can all be combine in the calculation
            if (math.abs(sum1 - sum2) < 1e-9) then
                exp.num = sum1
            table.insert(coeffs_def[ah], exp)
            -- If not, the defender probs don't add up to something nice and all
            -- need to be calculated one by one
            else
                for dm,v3 in pairs(v2) do  -- defender miss count
                    for dh,num in pairs(v3) do  -- defender hit count
                        local tmp_exp = AH.table_copy(exp)
                        tmp_exp.num = num
                        if (dm > 0) then tmp_exp.dm = dm end
                        if (dh > 0) then tmp_exp.dh = dh end
                        table.insert(coeffs_def[ah], tmp_exp)
                    end
                end
            end
        end
    end

    -- Now we do the same for the HP distribution of the attacker,
    -- which means everything needs to be sorted by defender hits
    local counts = {}
    for _,count in ipairs(hit_miss_counts) do
    local i1 = count.hit_miss_counts[3] -- note that the order here is different from above
        local i2 = count.hit_miss_counts[4]
        local i3 = count.hit_miss_counts[1]
    local i4 = count.hit_miss_counts[2]
        if not counts[i1] then counts[i1] = {} end
        if not counts[i1][i2] then counts[i1][i2] = {} end
        if not counts[i1][i2][i3] then counts[i1][i2][i3] = {} end
        counts[i1][i2][i3][i4] = (counts[i1][i2][i3][i4] or 0) + 1
    end

    local coeffs_att = {}
    for dm,v1 in pairs(counts) do  -- defender miss count
        for dh,v2 in pairs(v1) do  -- defender hit count
            -- Set up the exponent coefficients for attacker hits/misses
            local exp = {}  -- Array for an individual set of coefficients
            -- Only populate those indices that have exponents > 0
            if (dm > 0) then exp.dm = dm end
            if (dh > 0) then exp.dh = dh end

            -- We combine results by testing whether they produce the same sum
            -- with two very different hit probabilities, hp1 = 0.6, hp2 = 0.137
            -- This will only happen is the coefficients add up to multiples of 1
            local sum1, sum2 = 0,0
            local hp1, hp2 = 0.6, 0.137
            for am,v3 in pairs(v2) do  -- attacker miss count
                for ah,num in pairs(v3) do  -- attacker hit count
                    sum1 = sum1 + num * hp1^ah * (1 - hp1)^am
                    sum2 = sum2 + num * hp2^ah * (1 - hp2)^am
                end
            end

            -- Now, coefficients are set up for each value of total hits by attacker
            -- This holds all the coefficients that need to be added to get the propability
            -- of the defender receiving this number of hits
            if (not coeffs_att[dh]) then coeffs_att[dh] = {} end

            -- If sum1 and sum2 are equal, that means all the defender probs added up to 1, or
            -- multiple thereof, which means the can all be combine in the calculation
            if (math.abs(sum1 - sum2) < 1e-9) then
                exp.num = sum1
                table.insert(coeffs_att[dh], exp)
            -- If not, the defender probs don't add up to something nice and all
            -- need to be calculated one by one
            else
                for am,v3 in pairs(v2) do  -- defender miss count
                    for ah,num in pairs(v3) do  -- defender hit count
                        local tmp_exp = AH.table_copy(exp)
                    tmp_exp.num = num
                        if (am > 0) then tmp_exp.am = am end
                        if (ah > 0) then tmp_exp.ah = ah end
                    table.insert(coeffs_att[dh], tmp_exp)
                    end
                end
            end
        end
    end

    -- The probability for the number of hits with the most terms can be skipped
    -- and 1-sum(other_terms) can be used instead. Set a flag for which term to skip
    local max_number, biggest_equation = 0, -1
    for hits,v in pairs(coeffs_att) do
        local number = 0
        for _,c in pairs(v) do number = number + 1 end
        if (number > max_number) then
            max_number, biggest_equation = number, hits
        end
    end
    coeffs_att[biggest_equation].skip = true

    local max_number, biggest_equation = 0, -1
    for hits,v in pairs(coeffs_def) do
        local number = 0
        for _,c in pairs(v) do number = number + 1 end
        if (number > max_number) then
            max_number, biggest_equation = number, hits
        end
    end
    coeffs_def[biggest_equation].skip = true

    if cache then cache[cind] = { coeffs_att = coeffs_att, coeffs_def = coeffs_def } end

    return coeffs_att, coeffs_def
end

function battle_calcs.print_coefficients()
    -- Print out the set of coefficients for a given number of attacker and defender strikes
    -- Also print numerical values for a given hit probability
    -- This function is for debugging purposes only

    -- Configure these values at will
    local attacker_strikes, defender_strikes = 3, 3  -- number of strikes
    local att_hit_prob, def_hit_prob = 0.8, 0.4  -- probability of landing a hit attacker/defender
    local attacker_coeffs = true -- attacker coefficients if set to true, defender coefficients otherwise
    local defender_firststrike, attacker_firststrike = true, false

    -- Go through all combinations of maximum hits either attacker or defender can survive
    -- Note how this has to be crossed between ahits and defender_strikes and vice versa
    for ahits = defender_strikes,0,-1 do
        for dhits = attacker_strikes,0,-1 do
            -- Get the coefficients for this case
            local cfg = {
                att = { strikes = attacker_strikes, max_hits = ahits, firststrike = attacker_firststrike },
                def = { strikes = defender_strikes, max_hits = dhits, firststrike = defender_firststrike }
            }

            local coeffs, dummy = {}, {}
            if attacker_coeffs then
                coeffs = battle_calcs.battle_outcome_coefficients(cfg)
            else
                dummy, coeffs = battle_calcs.battle_outcome_coefficients(cfg)
            end

            print()
            print('Attacker: ' .. cfg.att.strikes .. ' strikes, can survive ' .. cfg.att.max_hits .. ' hits')
            print('Defender: ' .. cfg.def.strikes .. ' strikes, can survive ' .. cfg.def.max_hits .. ' hits')
            print('Chance of hits on defender: ')

            -- The first indices of coeffs are the possible number of hits the attacker can land on the defender
            for hits = 0,#coeffs do
                local hit_prob = 0.  -- probability for this number of hits
                local str = ''  -- output string

                local combs = coeffs[hits]  -- the combinations of coefficients to be evaluated
                for i,exp in ipairs(combs) do  -- exp: exponents (and factor) for a set
                    local prob = exp.num  -- probability for this set
                    str = str .. exp.num
                    if exp.am then
                       prob = prob * (1 - att_hit_prob) ^ exp.am
                       str = str .. ' pma^' .. exp.am
                    end
                    if exp.ah then
                        prob = prob * att_hit_prob ^ exp.ah
                        str = str .. ' pha^' .. exp.ah
                    end
                    if exp.dm then
                        prob = prob * (1 - def_hit_prob) ^ exp.dm
                        str = str .. ' pmd^' .. exp.dm
                    end
                    if exp.dh then
                       prob = prob * def_hit_prob ^ exp.dh
                        str = str .. ' phd^' .. exp.dh
                    end

                    hit_prob = hit_prob + prob  -- total probabilty for this number of hits landed
                    if (i ~= #combs) then str = str .. '  +  ' end
                end

                local skip_str = ''
                if combs.skip then skip_str = ' (skip)' end

                print(hits .. skip_str .. ':  ' .. str)
                print('      = ' .. hit_prob)
            end
        end
    end
end

function battle_calcs.hp_distribution(coeffs, att_hit_prob, def_hit_prob, starting_hp, damage, opp_attack)
    -- Multiply out the coefficients from battle_calcs.battle_outcome_coefficients()
    -- For a given attacker and defender hit/miss probability
    -- Also needed: the starting HP for the unit and the damage done by the opponent
    -- and the opponent attack information @opp_attack

    local stats  = { hp_chance = {}, average_hp = 0 }
    local skip_hp, skip_prob = -1, 1
    for hits = 0,#coeffs do
        local hp = starting_hp - hits * damage
        if (hp < 0) then hp = 0 end

        -- Calculation of the outcome with the most terms can be skipped
        if coeffs[hits].skip then
            skip_hp = hp
        else
            local hp_prob = 0.  -- probability for this number of hits
            for _,exp in ipairs(coeffs[hits]) do  -- exp: exponents (and factor) for a set
                local prob = exp.num  -- probability for this set
                if exp.am then prob = prob * (1 - att_hit_prob) ^ exp.am end
                if exp.ah then prob = prob * att_hit_prob ^ exp.ah end
                if exp.dm then prob = prob * (1 - def_hit_prob) ^ exp.dm end
                if exp.dh then prob = prob * def_hit_prob ^ exp.dh end

                hp_prob = hp_prob + prob  -- total probabilty for this number of hits landed
            end

            stats.hp_chance[hp] = hp_prob
            stats.average_hp = stats.average_hp + hp * hp_prob

            -- Also subtract this probability from the total prob. (=1), to get prob. of skipped outcome
            skip_prob = skip_prob - hp_prob
        end
    end

    -- Add in the outcome that was skipped
    stats.hp_chance[skip_hp] = skip_prob
    stats.average_hp = stats.average_hp + skip_hp * skip_prob

    -- And always set hp_chance[0] since it is of such importance in the analysis
    stats.hp_chance[0] = stats.hp_chance[0] or 0

    -- Add poison probability
    if opp_attack and opp_attack.poison then
        stats.poisoned = 1. - stats.hp_chance[starting_hp]
    else
        stats.poisoned = 0
    end

    -- Add slow probability
    if opp_attack and opp_attack.slow then
        stats.slowed = 1. - stats.hp_chance[starting_hp]
    else
        stats.slowed = 0
    end

    return stats
end

function battle_calcs.battle_outcome(attacker, defender, cfg, cache)
    -- Calculate the stats of a combat by @attacker vs. @defender
    -- @cfg: optional input parameters
    --  - att_weapon/def_weapon: attacker/defender weapon number
    --      if not given, get "best" weapon (Note: both must be given, or they will both be determined)
    --  - dst: { x, y }: the attack location; defaults to { attacker.x, attacker.y }
    -- @cache: to be passed on to other functions. battle_outcome itself is not cached, too many factors enter

    cfg = cfg or {}

    local dst = cfg.dst or { attacker.x, attacker.y }

    local att_weapon, def_weapon = 0, 0
    if (not cfg.att_weapon) or (not cfg.def_weapon) then
        att_weapon, def_weapon = battle_calcs.best_weapons(attacker, defender, dst, cache)
    else
        att_weapon, def_weapon = cfg.att_weapon, cfg.def_weapon
    end

    -- Collect all the information needed for the calculation
    -- Strike damage and numbers
    local att_damage, def_damage, att_attack, def_attack =
        battle_calcs.strike_damage(attacker, defender, att_weapon, def_weapon, { dst[1], dst[2] }, cache)

    -- Take swarm into account
    local att_strikes, def_strikes = att_attack.number, 0
    if (def_damage > 0) then
        def_strikes = def_attack.number
    end

    if att_attack.swarm then
        att_strikes = math.floor(att_strikes * attacker.hitpoints / attacker.max_hitpoints)
    end
    if def_attack and def_attack.swarm then
        def_strikes = math.floor(def_strikes * defender.hitpoints / defender.max_hitpoints)
    end

    -- Maximum number of hits that either unit can survive
    local att_max_hits = math.floor((attacker.hitpoints - 1) / def_damage)
    if (att_max_hits > def_strikes) then att_max_hits = def_strikes end
    local def_max_hits = math.floor((defender.hitpoints - 1) / att_damage)
    if (def_max_hits > att_strikes) then def_max_hits = att_strikes end

    -- Probability of landing a hit
    local att_hit_prob = wesnoth.unit_defense(defender, wesnoth.get_terrain(defender.x, defender.y)) / 100.
    local def_hit_prob = wesnoth.unit_defense(attacker, wesnoth.get_terrain(dst[1], dst[2])) / 100.

    -- Magical: attack and defense, and under all circumstances
    if att_attack.magical then att_hit_prob = 0.7 end
    if def_attack and def_attack.magical then def_hit_prob = 0.7 end

    -- Marksman: attack only, and only if terrain defense is less
    if att_attack.marksman and (att_hit_prob < 0.6) then
        att_hit_prob = 0.6
    end

    -- Get the coefficients for this kind of combat
    local def_firstrike = false
    if def_attack and def_attack.firststrike then def_firstrike = true end

    local cfg = {
        att = { strikes = att_strikes, max_hits = att_max_hits, firststrike = att_attack.firststrike },
        def = { strikes = def_strikes, max_hits = def_max_hits, firststrike = def_firstrike }
    }
    local att_coeffs, def_coeffs = battle_calcs.battle_outcome_coefficients(cfg, cache)

    -- And multiply out the factors
    -- Note that att_hit_prob, def_hit_prob need to be in that order for both calls
    local att_stats = battle_calcs.hp_distribution(att_coeffs, att_hit_prob, def_hit_prob, attacker.hitpoints, def_damage, def_attack)
    local def_stats = battle_calcs.hp_distribution(def_coeffs, att_hit_prob, def_hit_prob, defender.hitpoints, att_damage, att_attack)

    return att_stats, def_stats
end

function battle_calcs.simulate_combat_fake()
    -- A function to return a fake simulate_combat result
    -- Debug function to test how long simulate_combat takes
    -- It doesn't need any arguments -> can be called with the arguments of other simulate_combat functions
    local att_stats, def_stats = { hp_chance = {} }, { hp_chance = {} }

    att_stats.hp_chance[0] = 0
    att_stats.hp_chance[21], att_stats.hp_chance[23], att_stats.hp_chance[25], att_stats.hp_chance[27] = 0.125, 0.375, 0.375, 0.125
    att_stats.poisoned, att_stats.slowed, att_stats.average_hp = 0.875, 0, 24

    def_stats.hp_chance[0], def_stats.hp_chance[2], def_stats.hp_chance[10] = 0.09, 0.42, 0.49
    def_stats.poisoned, def_stats.slowed, def_stats.average_hp = 0, 0, 1.74

    return att_stats, def_stats
end

function battle_calcs.simulate_combat_loc(attacker, dst, defender, weapon)
    -- Get simulate_combat results for unit @attacker attacking unit @defender
    -- when on terrain of same type as that at @dst, which is of form { x, y }
    -- If @weapon is set, use that weapon (Lua index starting at 1), otherwise use best weapon

    local attacker_dst = wesnoth.copy_unit(attacker)
    attacker_dst.x, attacker_dst.y = dst[1], dst[2]

    if weapon then
        return wesnoth.simulate_combat(attacker_dst, weapon, defender)
    else
        return wesnoth.simulate_combat(attacker_dst, defender)
    end
end

function battle_calcs.attack_rating(attacker, defender, dst, cfg, cache)
    -- Returns a common (but configurable) rating for attacks
    -- Inputs:
    -- @attacker: attacker unit
    -- @defender: defender unit
    -- @dst: the attack location in form { x, y }
    -- @cfg: table of optional inputs and configurable rating parameters
    --  Optional inputs:
    --    - att_stats, def_stats: if given, use these stats, otherwise calculate them here
    --        Note: these are calculated in combination, that is they either both need to be passed or both be omitted
    --    - att_weapon/def_weapon: the attacker/defender weapon to be used if calculating battle stats here
    --        This parameter is meaningless (unused) if att_stats/def_stats are passed
    --        Defaults to weapon that does most damage to the opponent
    --        Note: as with the stats, they either both need to be passed or both be omitted
    -- @cache: cache table to be passed to battle_calcs.battle_outcome
    --
    -- Returns:
    --   - Overall rating for the attack or attack combo
    --   - Defender rating: not additive for attack combos; needs to be calculated for the
    --     defender stats of the last attack in a combo (that works for everything except
    --     the rating whether the defender is about to level in the attack combo)
    --   - Attacker rating: this one is split up into two terms:
    --     - a term that is additive for individual attacks in a combo
    --     - a term that needs to be average for the individual attacks in a combo
    --   - att_stats, def_stats: useful if they were calculated here, rather than passed down

    cfg = cfg or {}

    -- Set up the config parameters for the rating
    local enemy_leader_weight = cfg.enemy_leader_weight or 5.
    local defender_starting_damage_weight = cfg.defender_starting_damage_weight or 0.33
    local xp_weight = cfg.xp_weight or 0.25
    local level_weight = cfg.level_weight or 1.0
    local defender_level_weight = cfg.defender_level_weight or 1.0
    local distance_leader_weight = cfg.distance_leader_weight or 0.002
    local defense_weight = cfg.defense_weight or 0.1
    local occupied_hex_penalty = cfg.occupied_hex_penalty or -0.001
    local own_value_weight = cfg.own_value_weight or 1.0

    -- Get att_stats, def_stats
    -- If they are passed in cfg, use those
    local att_stats, def_stats = {}, {}
    if (not cfg.att_stats) or (not cfg.def_stats) then
        -- If cfg specifies the weapons use those, otherwise use "best" weapons
        -- In the latter case, cfg.???_weapon will be nil, which will be passed on
        local battle_cfg = { att_weapon = cfg.att_weapon, def_weapon = cfg.def_weapon, dst = dst }
        att_stats,def_stats = battle_calcs.battle_outcome(attacker, defender, battle_cfg, cache)
    else
        att_stats, def_stats = cfg.att_stats, cfg.def_stats
    end

    -- We also need the leader (well, the location at least)
    -- because if there's no other difference, prefer location _between_ the leader and the defender
    local leader = wesnoth.get_units { side = attacker.side, canrecruit = 'yes' }[1]

    ------ All the attacker contributions: ------
    -- Add up rating for the attacking unit
    -- We add this up in units of fraction of max_hitpoints
    -- It is multiplied by unit cost later, to get a gold equivalent value

    -- Average damage to unit is negative rating
    local damage = attacker.hitpoints - att_stats.average_hp
    -- Count poisoned as additional 8 HP damage times probability of being poisoned
    if (att_stats.poisoned ~= 0) then
        damage = damage + 8 * (att_stats.poisoned - att_stats.hp_chance[0])
    end
    -- Count slowed as additional 6 HP damage times probability of being slowed
    if (att_stats.slowed ~= 0) then
        damage = damage + 6 * (att_stats.slowed - att_stats.hp_chance[0])
    end

    -- If attack is from a village, we count that as a 10 HP bonus
    local is_village = wesnoth.get_terrain_info(wesnoth.get_terrain(dst[1], dst[2])).village
    if is_village then
        damage = damage - 10.
    end

    -- If attack is adjacent to an unoccupied village, that's bad
    for xa,ya in H.adjacent_tiles(dst[1], dst[2]) do
        local is_adjacent_village = wesnoth.get_terrain_info(wesnoth.get_terrain(xa, ya)).village
        if is_adjacent_village and (not wesnoth.get_unit(xa, ya)) then
            damage = damage + 10
        end
    end

    if (damage < 0) then damage = 0 end

    -- Fraction damage (= fractional value of the unit)
    local value_fraction = - damage / attacker.max_hitpoints

    -- Additional, subtract the chance to die, in order to (de)emphasize units that might die
    value_fraction = value_fraction - att_stats.hp_chance[0]

    -- In addition, potentially leveling up in this attack is a huge bonus,
    -- proportional to the chance of it happening and the chance of not dying itself
    local level_bonus = 0.
    local defender_level = wesnoth.unit_types[defender.type].level
    if (attacker.max_experience - attacker.experience <= defender_level) then
        level_bonus = 1. - att_stats.hp_chance[0]
    else
        if (attacker.max_experience - attacker.experience <= defender_level * 8) then
            level_bonus = (1. - att_stats.hp_chance[0]) * def_stats.hp_chance[0]
        end
    end
    value_fraction = value_fraction + level_bonus * level_weight


    -- Now convert this into gold-equivalent value
    local attacker_value = wesnoth.unit_types[attacker.type].cost

    -- Being closer to leveling is good (this makes AI prefer units with lots of XP)
    local xp_bonus = attacker.experience / attacker.max_experience
    attacker_value = attacker_value * (1. + xp_bonus * xp_weight)

    local attacker_rating = value_fraction * attacker_value

    ------ Now (most of) the same for the defender ------
    -- Average damage to defender is positive rating
    local damage = defender.hitpoints - def_stats.average_hp
    -- Count poisoned as additional 8 HP damage times probability of being poisoned
    if (def_stats.poisoned ~= 0) then
        damage = damage + 8 * (def_stats.poisoned - def_stats.hp_chance[0])
    end
    -- Count slowed as additional 6 HP damage times probability of being slowed
    if (def_stats.slowed ~= 0) then
        damage = damage + 6 * (def_stats.slowed - def_stats.hp_chance[0])
    end

    -- If defender is on a village, we count that as a 10 HP bonus
    local is_village = wesnoth.get_terrain_info(wesnoth.get_terrain(defender.x, defender.y)).village
    if is_village then
        damage = damage - 10.
    end

    if (damage < 0) then damage = 0. end

    -- Fraction damage (= fractional value of the unit)
    local value_fraction = damage / defender.max_hitpoints

    -- Additional, add the chance to kill, in order to emphasize enemies we might be able to kill
    value_fraction = value_fraction + def_stats.hp_chance[0]

    -- In addition, the defender potentially leveling up in this attack is a huge penalty,
    -- proportional to the chance of it happening and the chance of not dying itself
    local defender_level_penalty = 0.
    local attacker_level = wesnoth.unit_types[attacker.type].level
    if (defender.max_experience - defender.experience <= attacker_level) then
        defender_level_penalty = 1. - def_stats.hp_chance[0]
    else
        if (defender.max_experience - defender.experience <= attacker_level * 8) then
            defender_level_penalty = (1. - def_stats.hp_chance[0]) * att_stats.hp_chance[0]
        end
    end
    value_fraction = value_fraction - defender_level_penalty * defender_level_weight

    -- Now convert this into gold-equivalent value
    local defender_value = wesnoth.unit_types[defender.type].cost

    -- If this is the enemy leader, make damage to it much more important
    if defender.canrecruit then
        defender_value = defender_value * enemy_leader_weight
    end

    -- And prefer to attack already damaged enemies
    local defender_starting_damage_fraction = (defender.max_hitpoints - defender.hitpoints) / defender.max_hitpoints
    defender_value = defender_value * (1. + defender_starting_damage_fraction * defender_starting_damage_weight)

    -- Being closer to leveling is good, we want to get rid of those enemies first
    local xp_bonus = defender.experience / defender.max_experience
    defender_value = defender_value * (1. + xp_bonus * xp_weight)

    -- If defender is on a village, add a bonus rating (we want to get rid of those preferentially)
    -- So yes, this is positive, even though it's a plus for the defender
    -- Defenders on villages also got a negative damage rating above (these don't exactly cancel each other though)
    local is_village = wesnoth.get_terrain_info(wesnoth.get_terrain(defender.x, defender.y)).village
    if is_village then
        defender_value = defender_value * (1. + 10. / attacker.max_hitpoints)
    end

    -- We also add a few contributions that are not directly attack/damage dependent
    -- These are added to the defender rating for two reasons:
    --   1. Defender rating is positive (and thus contributions can be made positive)
    --   2. It is then independent of value of aggression (cfg.own_value_weight)
    --
    -- These are kept small though, so they mostly only serve as tie breakers
    -- And yes, they might bring the overall rating from slightly negative to slightly positive
    -- or vice versa, but as that is only approximate anyway, we keep it this way for simplicity

    -- We don't need a bonus for good terrain for the attacker, as that is covered in the damage calculation
    -- However, we add a small bonus for good terrain defense of the _defender_ on the _attack_ hex
    -- This is in order to take good terrain away from defender on next move, all else being equal
    local defender_defense = - wesnoth.unit_defense(defender, wesnoth.get_terrain(dst[1], dst[2])) / 100.
    defender_value = defender_value + defender_defense * defense_weight

    -- Get a very small bonus for hexes in between defender and AI leader
    -- 'relative_distances' is larger for attack hexes closer to the side leader (possible values: -1 .. 1)
    if leader then
        local relative_distances =
            H.distance_between(defender.x, defender.y, leader.x, leader.y)
            - H.distance_between(dst[1], dst[2], leader.x, leader.y)
        defender_value = defender_value + relative_distances * distance_leader_weight
    end

    -- Add a very small penalty for attack hexes occupied by other units
    -- Note: it must be checked previously that the unit on the hex can move away
    if (dst[1] ~= attacker.x) or (dst[2] ~= attacker.y) then
        if wesnoth.get_unit(dst[1], dst[2]) then
            defender_value = defender_value + occupied_hex_penalty
        end
    end

    local defender_rating = value_fraction * defender_value

    -- Finally apply factor of own unit weight to defender unit weight
    attacker_rating = attacker_rating * own_value_weight

    local rating = defender_rating + attacker_rating

    return rating, defender_rating, attacker_rating, att_stats, def_stats
end

function battle_calcs.attack_combo_stats(tmp_attackers, tmp_dsts, defender, cache, cache_this_move)
    -- Calculate attack combination outcomes using
    -- @tmp_attackers: array of attacker units (this is done so that
    --   the units need not be found here, as likely doing it in the
    --   calling function is more efficient (because of repetition)
    -- @tmp_dsts: array of the hexes (format { x, y }) from which the attackers attack
    --   must be in same order as @attackers
    -- @defender: the unit being attacked
    -- @cache: the cache table to be passed through to other battle_calcs functions
    --   attack_combo_stats itself is not cached, except for in cache_this_move below
    -- @cache_this_move: an optional table of pre-calculated attack outcomes
    --   - This is different from the other cache tables used in this file
    --   - This table may only persist for this move (move, not turn !!!), as otherwise too many things change
    --
    -- Return values:
    --   - The rating for this attack combination calculated from battle_calcs.attack_rating() results
    --   - The sorted attackers and dsts arrays
    --   - att_stats: an array of stats for each attacker, in the same order as 'attackers'
    --   - defender combo stats: one set of stats containing the defender stats after the attack combination
    --   - def_stats: an array of defender stats for each individual attack, in the same order as 'attackers'

    cache_this_move = cache_this_move or {}

    -- We first simulate and rate the individual attacks
    local ratings, tmp_attacker_ratings = {}, {}
    local tmp_att_stats, tmp_def_stats = {}, {}
    local defender_ind = defender.x * 1000 + defender.y
    for i,attacker in ipairs(tmp_attackers) do
        -- Initialize or use the 'cache_this_move' table
        local att_ind = attacker.x * 1000 + attacker.y
        local dst_ind = tmp_dsts[i][1] * 1000 + tmp_dsts[i][2]
        if (not cache_this_move[defender_ind]) then cache_this_move[defender_ind] = {} end
        if (not cache_this_move[defender_ind][att_ind]) then cache_this_move[defender_ind][att_ind] = {} end

        if (not cache_this_move[defender_ind][att_ind][dst_ind]) then
            -- Get the base rating
            local base_rating, def_rating, att_rating, att_stats, def_stats =
                battle_calcs.attack_rating(attacker, defender, tmp_dsts[i], {}, cache )
            tmp_attacker_ratings[i] = att_rating
            tmp_att_stats[i], tmp_def_stats[i] = att_stats, def_stats

            -- But for combos, also want units with highest attack outcome uncertainties to go early
            -- So that we can change our mind in case of unfavorable outcome
            --local outcome_variance = 0
            --local av = tmp_def_stats[i].average_hp
            --local n_outcomes = 0

            --for hp,p in pairs(tmp_def_stats[i].hp_chance) do
            --    if (p > 0) then
            --        local dhp_norm = (hp - av) / defender.max_hitpoints * wesnoth.unit_types[defender.type].cost
            --        local dvar = p * dhp_norm^2
                    --print(hp,p,av, dvar)
            --        outcome_variance = outcome_variance + dvar
            --        n_outcomes = n_outcomes + 1
            --    end
            --end
            --outcome_variance = outcome_variance / n_outcomes
            --print('outcome_variance', outcome_variance)

            -- Note that this is a variance, not a standard deviations (as in, it's squared),
            -- so it does not matter much for low-variance attacks, but takes on large values for
            -- high variance attacks. I think that is what we want.
            local rating = base_rating --+ outcome_variance

            -- If attacker has attack with 'slow' special, it should always go first
            -- Almost, bonus should not be quite as high as a really high CTK
            -- This isn't quite true in reality, but can be refined later
            if AH.has_weapon_special(attacker, "slow") then
                rating = rating + wesnoth.unit_types[defender.type].cost / 2.
            end

            ratings[i] = { i, rating, base_rating, def_rating, att_rating }

            -- Now add this attack to the cache_this_move table, so that next time around, we don't have to do this again
            cache_this_move[defender_ind][att_ind][dst_ind] = {
                rating = { -1, rating, base_rating, def_rating, att_rating },  -- Cannot use { i, rating, ... } here, as 'i' might be different next time
                attacker_ratings = tmp_attacker_ratings[i],
                att_stats = tmp_att_stats[i],
                def_stats = tmp_def_stats[i]
            }
        else
            local tmp_rating = cache_this_move[defender_ind][att_ind][dst_ind].rating
            tmp_rating[1] = i
            ratings[i] = tmp_rating
            tmp_attacker_ratings[i] = cache_this_move[defender_ind][att_ind][dst_ind].attacker_ratings
            tmp_att_stats[i] = cache_this_move[defender_ind][att_ind][dst_ind].att_stats
            tmp_def_stats[i] = cache_this_move[defender_ind][att_ind][dst_ind].def_stats
        end
    end

    -- Now sort all the arrays based on this rating
    -- This will give the order in which the individual attacks are executed
    table.sort(ratings, function(a, b) return a[2] > b[2] end)

    -- Reorder attackers, dsts in this order
    local attackers, dsts, att_stats, def_stats, attacker_ratings = {}, {}, {}, {}, {}
    for i,rating in ipairs(ratings) do
        attackers[i], dsts[i] = tmp_attackers[rating[1]], tmp_dsts[rating[1]]
    end
    -- Only keep the stats/ratings for the first attacker, the rest needs to be recalculated
    att_stats[1], def_stats[1] = tmp_att_stats[ratings[1][1]], tmp_def_stats[ratings[1][1]]
    attacker_ratings[1] = tmp_attacker_ratings[ratings[1][1]]

    tmp_attackers, tmp_dsts, tmp_att_stats, tmp_def_stats, tmp_attacker_ratings = nil, nil, nil, nil, nil

    -- Then we go through all the other attacks and calculate the outcomes
    -- based on all the possible outcomes of the previous attacks
    for i = 2,#attackers do
        att_stats[i] = { hp_chance = {} }
        def_stats[i] = { hp_chance = {} }
        local dst_ind = dsts[i][1] * 1000 + dsts[i][2]

        for hp1,prob1 in pairs(def_stats[i-1].hp_chance) do -- Note: need pairs(), not ipairs() !!
            if (hp1 == 0) then
                att_stats[i].hp_chance[attackers[i].hitpoints] =
                    (att_stats[i].hp_chance[attackers[i].hitpoints] or 0) + prob1
                def_stats[i].hp_chance[0] = (def_stats[i].hp_chance[0] or 0) + prob1
            else
                local org_hp = defender.hitpoints
                defender.hitpoints = hp1
                local ast, dst
                local att_ind_i = attackers[i].x * 1000 + attackers[i].y

                if (not cache_this_move[defender_ind][att_ind_i][dst_ind][hp1]) then
                    ast, dst = battle_calcs.battle_outcome(attackers[i], defender, { dst = dsts[i] } , cache)
                    cache_this_move[defender_ind][att_ind_i][dst_ind][hp1] = { ast = ast, dst = dst }
                else
                    ast = cache_this_move[defender_ind][att_ind_i][dst_ind][hp1].ast
                    dst = cache_this_move[defender_ind][att_ind_i][dst_ind][hp1].dst
                end

                defender.hitpoints = org_hp

                for hp2,prob2 in pairs(ast.hp_chance) do
                    att_stats[i].hp_chance[hp2] = (att_stats[i].hp_chance[hp2] or 0) + prob1 * prob2
                end
                for hp2,prob2 in pairs(dst.hp_chance) do
                    def_stats[i].hp_chance[hp2] = (def_stats[i].hp_chance[hp2] or 0) + prob1 * prob2
                end

                -- Also do poisoned, slowed
                if (not att_stats[i].poisoned) then
                    att_stats[i].poisoned = ast.poisoned
                    att_stats[i].slowed = ast.slowed
                    def_stats[i].poisoned = 1. - (1. - dst.poisoned) * (1. - def_stats[i-1].poisoned)
                    def_stats[i].slowed = 1. - (1. - dst.slowed) * (1. - def_stats[i-1].slowed)
                end
            end
        end

        -- Get the average HP
        local av_hp = 0
        for hp,prob in pairs(att_stats[i].hp_chance) do av_hp = av_hp + hp * prob end
        att_stats[i].average_hp = av_hp
        local av_hp = 0
        for hp,prob in pairs(def_stats[i].hp_chance) do av_hp = av_hp + hp * prob end
        def_stats[i].average_hp = av_hp
    end

    -- Get the total rating for this attack combo:
    --   = sum of all the attacker ratings and the defender rating with the final def_stats
    -- Rating for first attack exists already
    local def_rating = ratings[1][4]
    local att_rating = ratings[1][5]

    -- The others need to be calculated with the new stats
    for i = 2,#attackers do
        local cfg = { att_stats = att_stats[i], def_stats = def_stats[i] }
        local r, dr, ar = battle_calcs.attack_rating(attackers[i], defender, dsts[i], cfg, cache)

        def_rating = dr
        att_rating = att_rating + ar
    end

    local rating = def_rating + att_rating

    return rating, attackers, dsts, att_stats, def_stats[#attackers], def_stats
end

function battle_calcs.get_attack_map_unit(unit, cfg)
    -- Get all hexes that @unit can attack
    -- Return value is a location set, where the values are tables, containing
    --   - units: the number of units (always 1 for this function)
    --   - hitpoints: the combined hitpoints of the units
    --   - srcs: an array containing the positions of the units
    -- @cfg: table with config parameters
    --  max_moves: if set use max_moves for units (this setting is always used for units on other sides)

    cfg = cfg or {}

    -- 'moves' can be either "current" or "max"
    -- For unit on current side: use "current" by default, or override by cfg.moves
    local max_moves = cfg.max_moves
    -- For unit on any other side, only max_moves=true makes sense
    if (unit.side ~= wesnoth.current.side) then max_moves = true end

    local old_moves = unit.moves
    if max_moves then unit.moves = unit.max_moves end

    local reach = {}
    reach.units = LS.create()
    reach.hitpoints = LS.create()

    -- Also for units on the other side, take all units on this side with
    -- MP left off the map (for enemy pathfinding)
    local units_MP = {}
    if (unit.side ~= wesnoth.current.side) then
        local all_units = wesnoth.get_units { side = wesnoth.current.side }
        for _,unit in ipairs(all_units) do
            if (unit.moves > 0) then
                table.insert(units_MP, unit)
                wesnoth.extract_unit(unit)
            end
        end
    end

    -- Find hexes the unit can reach
    local initial_reach = wesnoth.find_reach(unit, cfg)

    -- Put the units back out there
    if (unit.side ~= wesnoth.current.side) then
        for _,uMP in ipairs(units_MP) do wesnoth.put_unit(uMP) end
    end

    for _,loc in ipairs(initial_reach) do
        reach.units:insert(loc[1], loc[2], 1)
        reach.hitpoints:insert(loc[1], loc[2], unit.hitpoints)
        for xa,ya in H.adjacent_tiles(loc[1], loc[2]) do
            reach.units:insert(xa, ya, 1)
            reach.hitpoints:insert(xa, ya, unit.hitpoints)
        end
    end

    if max_moves then unit.moves = old_moves end

    return reach
end

function battle_calcs.get_attack_map(units, cfg)
    -- Get all hexes that @units can attack.  This is really just a wrapper
    -- function for battle_calcs.get_attack_map_unit()
    -- Return value is a location set, where the values are tables, containing
    --   - units: the number of units (always 1 for this function)
    --   - hitpoints: the combined hitpoints of the units
    --   - srcs: an array containing the positions of the units
    -- @cfg: table with config parameters
    --  max_moves: if set use max_moves for units (this setting is always used for units on other sides)

    local attack_map1 = {}
    attack_map1.units = LS.create()
    attack_map1.hitpoints = LS.create()

    for _,unit in ipairs(units) do
        local attack_map2 = battle_calcs.get_attack_map_unit(unit, cfg)
        attack_map1.units:union_merge(attack_map2.units, function(x, y, v1, v2)
            return (v1 or 0) + v2
        end)
        attack_map1.hitpoints:union_merge(attack_map2.hitpoints, function(x, y, v1, v2)
            return (v1 or 0) + v2
        end)
    end

    return attack_map1
end

function battle_calcs.relative_damage_map(units, enemies, cache)
    -- Returns a location set map containing the relative damage of
    -- @units vs. @enemies on the part of the map that the combined units
    -- can reach. The damage is calculated as the sum of defender_rating
    -- from attack_rating(), and thus (roughly) in gold units.
    -- Also returns the same maps for the own and enemy units only
    -- (with the enemy_damage_map having positive sign, while in the
    -- overall damage map it is subtracted)

    -- Get the attack maps for each unit in 'units' and 'enemies'
    local my_attack_maps, enemy_attack_maps = {}, {}
    for i,unit in ipairs(units) do
        my_attack_maps[i] = battle_calcs.get_attack_map_unit(unit, cfg)
    end
    for i,e in ipairs(enemies) do
        enemy_attack_maps[i] = battle_calcs.get_attack_map_unit(e, cfg)
    end

    -- Get the damage rating for each unit in 'units'. It is the maximum
    -- defender_rating (roughly the damage that it can do in units of gold)
    -- against any of the enemy units
    local unit_ratings = {}
    for i,unit in ipairs(units) do
        local max_rating, best_enemy = -9e99, {}
        for _,enemy in ipairs(enemies) do
            local rating, defender_rating, attacker_rating =
                battle_calcs.attack_rating(unit, enemy, { unit.x, unit.y }, { enemy_leader_weight = 1 }, cache)

            local eff_rating = defender_rating
            if (eff_rating > max_rating) then
                max_rating = eff_rating
                best_enemy = enemy
            end
        end
        unit_ratings[i] = { rating = max_rating, unit_id = u.id, enemy_id = best_enemy.id }
    end

    -- Then we want the same thing for all of the enemy units (for the counter attack on enemy turn)
    local enemy_ratings = {}
    for i,enemy in ipairs(enemies) do
        local max_rating, best_unit = -9e99, {}
        for _,unit in ipairs(units) do
            local rating, defender_rating, attacker_rating =
                battle_calcs.attack_rating(enemy, unit, { enemy.x, enemy.y }, { enemy_leader_weight = 1 }, cache)

            local eff_rating = defender_rating
            if (eff_rating > max_rating) then
                max_rating = eff_rating
                best_unit = unit
            end
        end
        enemy_ratings[i] = { rating = max_rating, unit_id = best_unit.id, enemy_id = enemy.id }
    end

    -- The damage map is now the sum of these ratings for each unit that can attack a given hex,
    -- counting own-unit ratings as positive, enemy ratings as negative
    local damage_map, own_damage_map, enemy_damage_map = LS.create(), LS.create(), LS.create()
    for i,_ in ipairs(units) do
        my_attack_maps[i].units:iter(function(x, y, v)
            own_damage_map:insert(x, y, (own_damage_map:get(x, y) or 0) + unit_ratings[i].rating)
            damage_map:insert(x, y, (damage_map:get(x, y) or 0) + unit_ratings[i].rating)
        end)
    end
    for i,_ in ipairs(enemies) do
        enemy_attack_maps[i].units:iter(function(x, y, v)
            enemy_damage_map:insert(x, y, (enemy_damage_map:get(x, y) or 0) + enemy_ratings[i].rating)
            damage_map:insert(x, y, (damage_map:get(x, y) or 0) - enemy_ratings[i].rating)
        end)
    end

    return damage_map, own_damage_map, enemy_damage_map
end

function battle_calcs.best_defense_map(units, cfg)
    -- Get a defense rating map of all hexes all units in @units can reach
    -- For each hex, the value is the maximum of any of the units that can reach that hex
    -- @cfg: table with config parameters
    --  max_moves: if set use max_moves for units (this setting is always used for units on other sides)
    --  ignore_these_units: table of enemy units whose ZoC is to be ignored for route finding

    cfg = cfg or {}

    local defense_map = LS.create()

    if cfg.ignore_these_units then
        for _,unit in ipairs(cfg.ignore_these_units) do wesnoth.extract_unit(unit) end
    end

    for _,unit in ipairs(units) do
        -- Set max_moves according to the cfg value
        local max_moves = cfg.max_moves
        -- For unit on other than current side, only max_moves=true makes sense
        if (unit.side ~= wesnoth.current.side) then max_moves = true end
        local old_moves = unit.moves
        if max_moves then unit.moves = unit.max_moves end
        local reach = wesnoth.find_reach(unit, cfg)
        if max_moves then unit.moves = old_moves end

        for _,loc in ipairs(reach) do
            local defense = 100 - wesnoth.unit_defense(unit, wesnoth.get_terrain(loc[1], loc[2]))

            if (defense > (defense_map:get(loc[1], loc[2]) or -9e99)) then
                defense_map:insert(loc[1], loc[2], defense)
            end
        end
    end

    if cfg.ignore_these_units then
        for _,unit in ipairs(cfg.ignore_these_units) do wesnoth.put_unit(unit) end
    end

    return defense_map
end

function battle_calcs.get_attack_combos_subset(units, enemy, cfg)
    -- Calculate combinations of attacks by @units on @enemy
    -- This method does *not* produce all possible attack combinations, but is
    -- meant to have a good chance to find either the best combination,
    -- or something close to it, by only considering a subset of all possibilities.
    -- It is also configurable to stop accumulating combinations when certain criteria are met.
    --
    -- The return value is an array of attack combinations, where each element is another
    -- array of tables containing 'dst' and 'src' fields of the attacking units. It can be
    -- specified whether the order of the attacks matters or not (see below).
    --
    -- Note: This function is optimized for speed, not elegance
    --
    -- Note 2: The structure of the returned table is different from the (current) return value
    -- of ai_helper.get_attack_combos(), since the order of attacks never matters for the latter.
    -- TODO: consider making the two consistent (not sure yet whether that is advantageous)
    --
    -- @cfg: Table of optional configuration parameters
    --   - order_matters: if set, keep attack combos that use the same units on the same
    --       hexes, but in different attack order (default: false)
    --   - max_combos: stop adding attack combos if this number of combos has been reached
    --       default: assemble all possible combinations
    --   - max_time: stop adding attack combos if this much time (in seconds) has passed
    --       default: assemble all possible combinations
    --       note: this counts the time from the first call to add_attack(), not to
    --         get_attack_combos_cfg(), so there's a bit of extra overhead in here.
    --         This is done to prevent the return of no combos at all
    --         Note 2: there is some overhead involved in reading the time from the system,
    --           so don't use this unless it's needed
    --   - skip_presort: by default, the units are presorted in order of the unit with
    --       the highest rating first. This has the advantage of likely finding the best
    --       (or at least close to the best) attack combination earlier, but it add overhead,
    --       so it's actually a disadvantage for small numbers of combinations. skip_presort
    --       specifies the number of units up to which the presorting is skipped. Default: 5

    cfg = cfg or {}
    cfg.order_matters = cfg.order_matters or false
    cfg.max_combos = cfg.max_combos or 9e99
    cfg.max_time = cfg.max_time or false
    cfg.skip_presort = cfg.skip_presort or 5

    ----- begin add_attack() -----
    -- Recursive local function adding another attack to the current combo
    -- and adding the current combo to the overall attack_combos array
    local function add_attack(attacks, reachable_hexes, n_reach, attack_combos, combos_str, current_combo, hexes_used, cfg)

        local time_up = false
        if cfg.max_time and (wesnoth.get_time_stamp() / 1000. - cfg.start_time >= cfg.max_time) then
            time_up = true
        end

        -- Go through all the units
        for ind_att,attack in ipairs(attacks) do  -- 'attack' is array of all attacks for the unit

            -- Then go through the individual attacks of the unit ...
            for _,att in ipairs(attack) do
                -- But only if this hex is not used yet and
                -- the cutoff criteria are not met
                if (not hexes_used[att.dst]) and (not time_up) and (#attack_combos < cfg.max_combos) then

                    -- Mark this hex as used by this unit
                    hexes_used[att.dst] = attack.src

                    -- Set up a string uniquely identifying the unit/attack hex pairs
                    -- for current_combo. This is used to exclude pairs that already
                    -- exist in a different order (if 'cfg.order_matters' is not set)
                    -- For this, we also add the numerical value of the attack_hex to
                    -- the 'hexes_used' table (in addition to the line above)
                    local str = ''
                    if (not cfg.order_matters) then
                        hexes_used[reachable_hexes[att.dst]] = attack.src
                        for ind_hex = 1,n_reach do
                            if hexes_used[ind_hex] then
                                str = str .. hexes_used[ind_hex] .. '-'
                            else
                                str = str .. '0-'
                            end
                        end
                    end

                    -- 'combos_str' contains all the strings of previous combos
                    -- (if 'cfg.order_matters' is not set)
                    -- Only add this combo if it does not yet exist
                    if (not combos_str[str]) then

                        -- Add the string identifyer to the array
                        if (not cfg.order_matters) then
                            combos_str[str] = true
                        end

                        -- Add the attack to 'current_combo'
                        table.insert(current_combo, { dst = att.dst, src = attack.src })

                        -- And *copy* the content of 'current_combo' into 'attack_combos'
                        local n_combos = #attack_combos + 1
                        attack_combos[n_combos] = {}
                        for ind_combo,combo in pairs(current_combo) do attack_combos[n_combos][ind_combo] = combo end

                        -- Finally, remove the current unit for 'attacks' for the call to the next recursion level
                        table.remove(attacks, ind_att)

                        add_attack(attacks, reachable_hexes, n_reach, attack_combos, combos_str, current_combo, hexes_used, cfg)

                        -- Reinsert the unit
                        table.insert(attacks, ind_att, attack)

                        -- And remove the last element (current attack) from 'current_combo'
                        table.remove(current_combo)
                    end

                    -- And mark the hex as usable again
                    if (not cfg.order_matters) then
                        hexes_used[reachable_hexes[att.dst]] = nil
                    end
                    hexes_used[att.dst] = nil

                    -- *** Important ***: We *only* consider one attack hex per unit, the
                    -- first that is found in the array of attacks for the unit. As they
                    -- are sorted by terrain defense, we simply use the first in the table
                    -- the unit can reach that is not occupied
                    -- That's what the 'break' does here:
                    break
                end
            end
        end
    end
    ----- end add_attack() -----

    -- For units on the current side, we need to make sure that
    -- there isn't a unit of the same side in the way that cannot move any more
    -- Set up an array of hexes blocked in such a way
    -- For units on other sides we always assume that they can move away
    local blocked_hexes = LS.create()
    if units[1] and (units[1].side == wesnoth.current.side) then
        for xa,ya in H.adjacent_tiles(enemy.x, enemy.y) do
            local unit_in_way = wesnoth.get_unit(xa, ya)
            if unit_in_way then
                -- Units on the same side are blockers if they cannot move away
                if (unit_in_way.side == wesnoth.current.side) then
                    local reach = wesnoth.find_reach(unit_in_way)
                    if (#reach <= 1) then
                        blocked_hexes:insert(unit_in_way.x, unit_in_way.y)
                    end
                else  -- Units on other sides are always blockers
                    blocked_hexes:insert(unit_in_way.x, unit_in_way.y)
                end
            end
        end
    end

    -- For sides other than the current, we always use max_moves,
    -- for the current side we always use current moves
    local old_moves = {}
    for i,unit in ipairs(units) do
        if (unit.side ~= wesnoth.current.side) then
            old_moves[i] = unit.moves
            unit.moves = unit.max_moves
        end
    end

    -- Now set up an array containing the attack locations for each unit
    local attacks = {}
    -- We also need a numbered array of the possible attack hex coordinates
    -- The order doesn't matter, as long as it is fixed
    local reachable_hexes = {}
    for i,unit in ipairs(units) do

        local locs = {}  -- attack locations for this unit

        for xa,ya in H.adjacent_tiles(enemy.x, enemy.y) do

            local loc = {}  -- attack location information for this unit for this hex

            -- Make sure the hex is not occupied by unit that cannot move out of the way
            if (not blocked_hexes:get(xa, ya) or ((xa == unit.x) and (ya == unit.y))) then

                -- Check whether the unit can get to the hex
                -- helper.distance_between() is much faster than wesnoth.find_path()
                --> pre-filter using the former
                local cost = H.distance_between(unit.x, unit.y, xa, ya)

                -- If the distance is <= the unit's MP, then see if it can actually get there
                -- This also means that only short paths have to be evaluated (in most situations)
                if (cost <= unit.moves) then
                    local path  -- since cost is already defined outside this block
                    path, cost = AH.find_path_with_shroud(unit, xa, ya)
                end

                -- If the unit can get to this hex
                if (cost <= unit.moves) then
                    -- Store information about it in 'loc' and add this to 'locs'
                    -- Want coordinates (dst) and terrain defense (for sorting)
                    loc.dst = xa * 1000 + ya
                    loc.hit_prob = wesnoth.unit_defense(unit, wesnoth.get_terrain(xa, ya))
                    table.insert(locs, loc)

                    -- Also mark this hex as usable
                    reachable_hexes[loc.dst] = true
                end
            end
        end

        -- Also add some top-level information for the unit
        if locs[1] then
            locs.src = unit.x * 1000 + unit.y  -- The current position of the unit
            locs.unit_i = i  -- The position of the unit in the 'units' array

            -- Now sort the possible attack locations for this unit by terrain defense
            table.sort(locs, function(a, b) return a.hit_prob < b.hit_prob end)

            -- Finally, add the attack locations for this unit to the 'attacks' array
            table.insert(attacks, locs)
        end

    end

    -- Reset moves for all units
    for i,unit in ipairs(units) do
        if (unit.side ~= wesnoth.current.side) then
            unit.moves = old_moves[i]
        end
    end

    -- If the number of units that can attack is greater than cfg.skip_presort:
    -- We also sort the attackers by their attack rating on their favorite hex
    -- The motivation is that by starting with the strongest unit, we'll find the
    -- best attack combo earlier, and it is more likely to find the best (or at
    -- least a good combo) even when not all attack combinations are collected.
    if (#attacks > cfg.skip_presort) then
        for _,attack in ipairs(attacks) do
            local dst = attack[1].dst
            local x, y = math.floor(dst / 1000), dst % 1000
            attack.rating = battle_calcs.attack_rating(units[attack.unit_i], enemy, { x, y })
        end
        table.sort(attacks, function(a, b) return a.rating > b.rating end)
    end

    -- To simplify and speed things up in the following, the field values
    -- 'reachable_hexes' table needs to be consecutive integers
    -- We also want a variable containing the number of elements in the array
    -- (#reachable_hexes doesn't work because they keys are location indices)
    local n_reach = 0
    for k,hex in pairs(reachable_hexes) do
        n_reach = n_reach + 1
        reachable_hexes[k] = n_reach
    end

    -- If cfg.max_time is set, record the start time
    -- For convenience, we store this in cfg
    if cfg.max_time then
        cfg.start_time = wesnoth.get_time_stamp() / 1000.
    end


    -- All this was just setting up the required information, now we call the
    -- recursive function setting up the array of attackcombinations
    local attack_combos = {}  -- This will contain the return value
    -- Temporary arrays (but need to be persistent across the recursion levels)
    local combos_str, current_combo, hexes_used  = {}, {}, {}

    add_attack(attacks, reachable_hexes, n_reach, attack_combos, combos_str, current_combo, hexes_used, cfg)

    cfg.start_time = nil

    return attack_combos
end

return battle_calcs
