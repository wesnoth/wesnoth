local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"

local wolves_multipacks_functions = {}

function wolves_multipacks_functions.color_label(x, y, text)
    -- For displaying the wolf pack number in color underneath each wolf
    -- only using gray for the time being
    text = "<span color='#c0c0c0'>" .. text .. "</span>"
    W.label{ x = x, y = y, text = text }
end


function wolves_multipacks_functions.assign_packs(cfg)
    local unit_type = cfg.type or "Wolf"
    local pack_size = cfg.pack_size or 3

    -- Assign the pack numbers to each wolf. Keeps numbers of existing packs
    -- (unless pack size is down to one). Pack number is stored in wolf unit variables
    -- Also returns a table with the packs (locations and id's of each wolf in a pack)
    local wolves = wesnoth.get_units { side = wesnoth.current.side, type = unit_type }
    --print('#wolves:', #wolves)

    -- Array for holding the packs
    local packs = {}
    -- Find wolves that already have a pack number assigned
    for i,w in ipairs(wolves) do
        local pack = MAIUV.get_mai_unit_variables(w, cfg.ai_id, "pack")
        if pack then
            if (not packs[pack]) then packs[pack] = {} end
            table.insert(packs[pack], { x = w.x, y = w.y, id = w.id })
        end
    end

    -- Remove packs of one
    -- Pack numbers might not be consecutive after a while -> need pairs(), not ipairs()
    for k,p in pairs(packs) do
        --print(' have pack:', k, ' #members:', #p)
        if (#p == 1) then
            local wolf = wesnoth.get_unit(p[1].x, p[1].y)
            MAIUV.delete_mai_unit_variables(wolf, cfg.ai_id)
            packs[k] = nil
        end
    end
    --print('After removing packs of 1')
    --for k,p in pairs(packs) do print(' have pack:', k, ' #members:', #p) end

    -- Wolves that are not in a pack (new ones or those removed above)
    local nopack_wolves = {}
    for i,w in ipairs(wolves) do
        local pack = MAIUV.get_mai_unit_variables(w, cfg.ai_id, "pack")
        if (not pack) then
            table.insert(nopack_wolves, w)
            -- Also erase any goal one of these might have
            MAIUV.delete_mai_unit_variables(w, cfg.ai_id)
        end
    end
    --print('#nopack_wolves:', #nopack_wolves)

    -- Now assign the nopack wolves to packs
    -- First, go through packs that have less than pack_size members
    for k,p in pairs(packs) do
        if (#p < pack_size) then
            local min_dist, best_wolf, best_ind = 9e99, {}, -1
            for i,w in ipairs(nopack_wolves) do
                local d1 = H.distance_between(w.x, w.y, p[1].x, p[1].y)
                local d2 = H.distance_between(w.x, w.y, p[2].x, p[2].y)
                if (d1 + d2 < min_dist) then
                    min_dist = d1 + d2
                    best_wolf, best_ind = w, i
                end
            end
            if (min_dist < 9e99) then
                table.insert(packs[k], { x = best_wolf.x, y = best_wolf.y, id = best_wolf.id })
                MAIUV.set_mai_unit_variables(best_wolf, cfg.ai_id, { pack = k })
                table.remove(nopack_wolves, best_ind)
            end
        end
    end
    --print('After completing packs of 2')
    --for k,p in pairs(packs) do print(' have pack:', k, ' #members:', #p) end

    -- Second, group remaining single wolves
    -- At the beginning of the scenario, this is all wolves
    while (#nopack_wolves > 0) do
        --print('Grouping the remaining wolves', #nopack_wolves)
        -- First find the first available pack number
        new_pack = 1
        while packs[new_pack] do new_pack = new_pack + 1 end
        --print('Building pack', new_pack)

        -- If there are <=pack_size wolves left, that's the pack (we also assign a single wolf to a 1-wolf pack here)
        if (#nopack_wolves <= pack_size) then
            --print('<=pack_size nopack wolves left', #nopack_wolves)
            packs[new_pack] = {}
            for i,w in ipairs(nopack_wolves) do
                table.insert(packs[new_pack], { x = w.x, y = w.y, id = w.id })
                MAIUV.set_mai_unit_variables(w, cfg.ai_id, { pack = new_pack })
            end
            break
        end

        -- If more than pack_size wolves left, find those that are closest together
        -- They form the next pack
        --print('More than pack_size nopack wolves left', #nopack_wolves)
        local best_wolves = {}
        while #best_wolves < pack_size do
            local min_dist, best_wolf, best_wolf_i = 9999, {}, -1
            for i,tw in ipairs(nopack_wolves) do
                local dist = 0
                for j,sw in ipairs(best_wolves) do
                    dist = dist + H.distance_between(tw.x, tw.y, sw.x, sw.y)
                end
                if dist < min_dist then
                    min_dist, best_wolf, best_wolf_i = dist, tw, i
                end
            end
            table.insert(best_wolves, best_wolf)
            table.remove(nopack_wolves, best_wolf_i)
        end
        -- Now insert the best pack into that 'packs' array
        packs[new_pack] = {}
        for i = 1,pack_size do
            table.insert(packs[new_pack], { x = best_wolves[i].x, y = best_wolves[i].y, id = best_wolves[i].id })
            MAIUV.set_mai_unit_variables(best_wolves[i], cfg.ai_id, { pack = new_pack })
        end
    end
    --print('After grouping remaining single wolves')
    --for k,p in pairs(packs) do print(' have pack:', k, ' #members:', #p) end

    -- Put labels out there for all wolves
    if cfg.show_pack_number then
        for k,p in pairs(packs) do
            for i,loc in ipairs(p) do
                wolves_multipacks_functions.color_label(loc.x, loc.y, k)
            end
        end
    end

    return packs
end

return wolves_multipacks_functions

