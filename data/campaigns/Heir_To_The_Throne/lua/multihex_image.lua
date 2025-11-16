
-- credit to lhybrideur and the Stable Corpse Mod
function wesnoth.wml_actions.multihex_image(cfg)
	local locs = wesnoth.map.find(cfg)
	cfg = wml.parsed(cfg)
	if not cfg.image then
		wml.error "[multihex_image] missing required image= attribute."
	end
	local submerge = cfg.submerge
	cfg.submerge=0
	local redraw = cfg.redraw
	cfg.redraw = false
	local width, height = filesystem.image_size(cfg.image)
	for i, loc in ipairs(locs) do
		local ref_image = cfg.image
		if submerge then
			cfg.image ="misc/blank-hex.png~BLIT("..ref_image.."~CROP("..tostring(width/2-36)..","..tostring(height/2-36)..",72,72),0,0)"..submerge_tile_string(cfg.x,cfg.y,true,0,height/2-36)
		else
			cfg.image ="misc/blank-hex.png~BLIT("..ref_image.."~CROP("..tostring(width/2-36)..","..tostring(height/2-36)..",72,72),0,0)"
		end
		wesnoth.wml_actions.item(cfg)

		local n_hex, ne_hex, se_hex, s_hex, sw_hex, nw_hex = wesnoth.map.get_adjacent_hexes(loc)
		if height>72 then
			cfg.x=n_hex[1]
			cfg.y=n_hex[2]
			cfg.image = "misc/blank-hex.png~BLIT("..ref_image.."~CROP("..tostring(width/2-36)..",0,72,"..tostring(height/2-36).."),0,"..tostring(72-(height/2-36))..")"
			wesnoth.wml_actions.item(cfg)
		end
		cfg.x=ne_hex[1]
		cfg.y=ne_hex[2]
		cfg.image = "misc/blank-hex.png~BLIT("..ref_image.."~CROP("..tostring(width/2+18)..","..tostring(math.max(0,height/2-72))..","..tostring(width/2-18)..","..tostring(height/2).."),0,"..tostring(math.max(0,72-height/2))..")"
		wesnoth.wml_actions.item(cfg)
		cfg.x=se_hex[1]
		cfg.y=se_hex[2]
		if submerge then
			cfg.image = "misc/blank-hex.png~BLIT("..ref_image.."~CROP("..tostring(width/2+18)..","..tostring(height/2)..","..tostring(width/2-18)..","..tostring(math.min(height/2,72)).."),0,0)"..submerge_tile_string(cfg.x,cfg.y,true,0,height/2-72)
		else
			cfg.image = "misc/blank-hex.png~BLIT("..ref_image.."~CROP("..tostring(width/2+18)..","..tostring(height/2)..","..tostring(width/2-18)..","..tostring(math.min(height/2,72)).."),0,0)"
		end
		wesnoth.wml_actions.item(cfg)
		if height>72 then
			cfg.x=s_hex[1]
			cfg.y=s_hex[2]
			if submerge then
				cfg.image = "misc/blank-hex.png~BLIT("..ref_image.."~CROP("..tostring(width/2-36)..","..tostring(height/2+36)..",72,"..tostring(height/2-36).."),0,0)"..submerge_tile_string(cfg.x,cfg.y,true,0,height/2-108)
			else
				cfg.image = "misc/blank-hex.png~BLIT("..ref_image.."~CROP("..tostring(width/2-36)..","..tostring(height/2+36)..",72,"..tostring(height/2-36).."),0,0)"
			end
			wesnoth.wml_actions.item(cfg)
		end
		cfg.x=sw_hex[1]
		cfg.y=sw_hex[2]
		if submerge then
			cfg.image = "misc/blank-hex.png~BLIT("..ref_image.."~CROP(0,"..tostring(height/2)..","..tostring(width/2-18)..","..tostring(math.min(height/2,72)).."),"..tostring(72-(width/2-18))..",0)"..submerge_tile_string(cfg.x,cfg.y,true,0,height/2-72)
		else
			cfg.image = "misc/blank-hex.png~BLIT("..ref_image.."~CROP(0,"..tostring(height/2)..","..tostring(width/2-18)..","..tostring(math.min(height/2,72)).."),"..tostring(72-(width/2-18))..",0)"
		end
		wesnoth.wml_actions.item(cfg)
		cfg.x=nw_hex[1]
		cfg.y=nw_hex[2]
		cfg.image = "misc/blank-hex.png~BLIT("..ref_image.."~CROP(0,"..tostring(math.max(0,height/2-72))..","..tostring(width/2-18)..","..tostring(math.min(height/2,72)).."),"..tostring(72-(width/2-18))..","..tostring(math.max(0,72-height/2))..")"
		wesnoth.wml_actions.item(cfg)
	end
	if redraw then
		wesnoth.wml_actions.redraw{}
	end
end
