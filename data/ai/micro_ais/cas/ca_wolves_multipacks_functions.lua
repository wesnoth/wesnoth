local H = wesnoth.require "helper"
local W = H.set_wml_action_metatable {}
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"
local M = wesnoth.map

local wolves_multipacks_functions = {}

function wolves_multipacks_functions.clear_label(x, y)
    W.label{ x = x, y = y, text = "" }
end

function wolves_multipacks_functions.put_label(x, y, text)
    -- For displaying the wolf pack number underneath each wolf
    -- Only use gray for now, but easily expandable to add a color option
    text = "<span color='#c0c0c0'>" .. text .. "</span>"
    W.label{ x = x, y = y, text = text }
end

function wolves_multipacks_functions.assign_packs(cfg)
    -- Assign the pack numbers to each wolf. Keeps numbers of existing packs
    -- (unless pack size is down to one). Pack number is stored in wolf unit variables
    -- Also returns a table with the packs (locations and id's of each wolf in a pack)

    local pack_size = cfg.pack_size or 3

    local wolves = wesnoth.get_units { side = wesnoth.current.side, type = cfg.type or "Wolf" }
    local packs = {}

    -- Find wolves that already have a pack number assigned
    for _,w in ipairs(wolves) do
        local pack = MAIUV.get_mai_unit_variables(w, cfg.ai_id, "pack_number")
        if pack then
            if (not packs[pack]) then packs[pack] = {} end
            table.insert(packs[pack], { x = w.x, y = w.y, id = w.id })
        end
    end

    -- Remove packs of one
    -- Do not change numbers of existing packs -> pack numbers might not be consecutive afterward
    for pack_number,pack in pairs(packs) do
        if (#pack == 1) then
            local wolf = wesnoth.get_unit(pack[1].x, pack[1].y)
            MAIUV.delete_mai_unit_variables(wolf, cfg.ai_id)
            packs[pack_number] = nil
        end
    end

    -- Find wolves that are not in a pack (new ones or those removed above)
    local nopack_wolves = {}
    for _,w in ipairs(wolves) do
        local pack_number = MAIUV.get_mai_unit_variables(w, cfg.ai_id, "pack_number")
        if (not pack_number) then
            table.insert(nopack_wolves, w)
        end
    end

    -- Now assign the nopack wolves to packs
    -- First, go through packs that have less than pack_size members
    for pack_number,pack in pairs(packs) do
        if (#pack < pack_size) then
            local min_dist, best_wolf, best_ind = 9e99
            for ind,wolf in ipairs(nopack_wolves) do
                -- Criterion is distance from the first two wolves of the pack
                local dist1 = M.distance_between(wolf.x, wolf.y, pack[1].x, pack[1].y)
                local dist2 = M.distance_between(wolf.x, wolf.y, pack[2].x, pack[2].y)
                if (dist1 + dist2 < min_dist) then
                    min_dist = dist1 + dist2
                    best_wolf, best_ind = wolf, ind
                end
            end
            if best_wolf then
                table.insert(packs[pack_number], { x = best_wolf.x, y = best_wolf.y, id = best_wolf.id })
                MAIUV.set_mai_unit_variables(best_wolf, cfg.ai_id, { pack_number = pack_number })
                table.remove(nopack_wolves, best_ind)
            end
        end
    end

    -- Second, group remaining single wolves
    -- At the beginning of the scenario, this means all wolves
    while (#nopack_wolves > 0) do
        -- Find the first available pack number
        new_pack_number = 1
        while packs[new_pack_number] do new_pack_number = new_pack_number + 1 end

        -- If there are <=pack_size wolves left, that's the pack
        if (#nopack_wolves <= pack_size) then
            packs[new_pack_number] = {}
            for _,w in ipairs(nopack_wolves) do
                table.insert(packs[new_pack_number], { x = w.x, y = w.y, id = w.id })
                MAIUV.set_mai_unit_variables(w, cfg.ai_id, { pack_number = new_pack_number })
            end
            break
        end

        -- If more than pack_size wolves left, find those that are closest together
        -- They form the next pack
        local new_pack_wolves = {}
        while (#new_pack_wolves < pack_size) do
            local min_dist, best_wolf, best_wolf_ind = 9e99
            for ind,nopack_wolf in ipairs(nopack_wolves) do
                local dist = 0
                for _,pack_wolf in ipairs(new_pack_wolves) do
                    dist = dist + M.distance_between(nopack_wolf.x, nopack_wolf.y, pack_wolf.x, pack_wolf.y)
                end
                if dist < min_dist then
                    min_dist, best_wolf, best_wolf_ind = dist, nopack_wolf, ind
                end
            end
            table.insert(new_pack_wolves, best_wolf)
            table.remove(nopack_wolves, best_wolf_ind)
        end

        -- Now insert the best pack into that 'packs' array
        packs[new_pack_number] = {}
        for ind = 1,pack_size do
            table.insert(
                packs[new_pack_number],
                { x = new_pack_wolves[ind].x, y = new_pack_wolves[ind].y, id = new_pack_wolves[ind].id }
            )
            MAIUV.set_mai_unit_variables(new_pack_wolves[ind], cfg.ai_id, { pack_number = new_pack_number })
        end
    end

    -- Put labels out there for all wolves
    if cfg.show_pack_number then
        for pack_number,pack in pairs(packs) do
            for _,wolf in ipairs(pack) do
                wolves_multipacks_functions.put_label(wolf.x, wolf.y, pack_number)
            end
        end
    end

    return packs
end

return wolves_multipacks_functions
