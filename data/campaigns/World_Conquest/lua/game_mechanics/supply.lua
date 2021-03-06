
local supply_images = 
{
	"misc/blank-hex.png~BLIT(items/straw-bale2.png~CROP(24,15,48,57))~BLIT(items/straw-bale1.png~CROP(0,9,72,63))",
	"misc/blank-hex.png~BLIT(items/straw-bale1.png~CROP(8,15,64,57))~BLIT(items/leather-pack.png~CROP(0,10,62,62),10,0)~BLIT(items/leather-pack.png~CROP(0,0,69,72),3,0)",
	"misc/blank-hex.png~BLIT(items/straw-bale2.png~CROP(19,18,53,54))~BLIT(items/straw-bale2.png~CROP(7,6,65,66))~BLIT(items/leather-pack.png~CROP(4,23,68,49))~BLIT(items/straw-bale2.png~CROP(7,15,65,57))",
	-- custom image
	"terrain/wct-supply2.png",
	-- custom image
	"terrain/wct-supply1.png",
	"misc/blank-hex.png~BLIT(items/box.png~SCALE(54,54)~CROP(0,11,54,43),4,0)~BLIT(items/straw-bale2.png~CROP(5,24,67,48),0,24)~BLIT(items/leather-pack.png~CROP(0,9,63,63),9,0)",
	"misc/blank-hex.png~BLIT(items/box.png~SCALE(54,54)~CROP(0,11,54,43),4,0)~BLIT(items/box.png~SCALE(54,54)~CROP(0,1,54,53),17,0)",
	-- custom image
	"terrain/wct-supply3.png~BLIT(items/box.png~SCALE(54,54)~CROP(0,1,54,53),17,0)",
}

function wesnoth.wml_actions.wc2_map_supply_village(t)
	local unit = wesnoth.units.get(t.x, t.y)
	local loc = unit.loc
	wesnoth.current.map[loc] = "^Vov"
	
	wesnoth.map.set_owner(loc, unit.side, false)
	
	local supply_image = ((wml.variables.wc2_supply_image_counter or 0) % #supply_images ) + 1
	wml.variables.wc2_supply_image_counter = supply_image
	wesnoth.wml_actions.item {
		x = loc[1],
		y = loc[2],
		image = supply_images[supply_image],
		z_order = -10,
	}
end