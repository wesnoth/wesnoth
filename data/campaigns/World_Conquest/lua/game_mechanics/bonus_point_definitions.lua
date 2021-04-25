local _ = wesnoth.textdomain 'wesnoth-wc'

local STR_BONUS_SHIP_NAMES = { _"Point", _"Rest", _"Ruin", _"Rocks", _"Bane", _"Waters", _"Route", _"Vestige", _"Disaster", _"Massacre", _"Expedition", _"Cargo" }

local STR_BONUS_BONES_NAMES = { _"Point", _"Rest", _"Lair", _"Exile", _"Bane", _"Passage", _"Curse" }

local STR_BONUS_BONES_SWAMP_NAMES = { _"Point", _"Rest", _"Lair", _"Exile", _"Bane", _"Death", _"Passage", _"Curse", _"Quagmire" }

local STR_BONUS_DEAD_OAK_NAMES = { _"Rest", _"Point", _"Lair", _"Exile", _"Bane", _"Death", _"Curse", _"Wasteland", _"Desolation", _"Despair", _"Desert", _"Route", _"Passage", _"Cemetery" }

local STR_BONUS_VILLAGE_RUINED_NAMES = { _"Rest", _"Ruin", _"Lair", _"Exile", _"Bane", _"Curse", _"Destruction", _"Desolation", _"Vestige", _"Disaster" }

local STR_BONUS_BURIAL_NAMES = { _"Point", _"Rest", _"Lair", _"Passage", _"Bane", _"Massacre", _"Curse", _"Disaster", _"Cemetery" }

local STR_BONUS_BURIAL_SAND_NAMES = { _"Point", _"Rest", _"Lair", _"Exile", _"Passage", _"Bane", _"Massacre", _"Curse", _"Disaster" }

local STR_BONUS_BURIAL_CAVE_NAMES = { _"Rest", _"Lair", _"Passage", _"Bane", _"Grotto", _"Curse", _"Cave", _"Cemetery", _"Graveyard" }

local STR_BONUS_DETRITUS_NAMES = { _"Point", _"Rest", _"Ruin", _"Lair", _"Exile", _"Passage", _"Bane", _"Massacre", _"Curse", _"Quagmire", _"Bog", _"Hideout", _"Desolation" }

local STR_BONUS_TRAPDOOR_NAMES = { _"Rest", _"Ruin", _"Lair", _"Passage", _"Bane", _"Despair", _"Curse", _"Cave", _"Dungeon", _"Secret", _"Hideout", _"Mine", _"Grotto", _"Den" }

local STR_BONUS_COFFIN_NAMES = { _"Rest", _"Ruin", _"Lair", _"Exile", _"Passage", _"Bane", _"Dream", _"Crypt", _"Grotto", _"Graveyard", _"Secret", _"Curse", _"Mausoleum", _"Ossuary" }

local STR_BONUS_MINE_NAMES = { _"Lair", _"Passage", _"Bane", _"Peak", _"Mine", _"Curse", _"Den", _"Site", _"Quarry", _"Dungeon" }

local STR_BONUS_DOORS_NAMES = { _"Rest", _"Lair", _"Passage", _"Bane", _"Peak", _"Mine", _"Destiny", _"Door", _"Gate", _"Curse", _"Domain", _"Secret", _"Dungeon" }

local STR_BONUS_SHOP_NAMES = { _"Path", _"Road", _"Route", _"Market", _"Fair", _"Festival", _"Tournament", _"Enclave", _"Armory" }

local STR_BONUS_SHOP_GRASS_NAMES = { _"Route", _"Market", _"Enclave", _"Fair", _"Festival", _"Tournament", _"Armory", _"Refuge" }

local STR_BONUS_SIGNPOST_NAMES = { _"Rest", _"Path", _"Point", _"Passage", _"Road", _"Route", _"Domain" }

local STR_BONUS_ROCK_CAIRN_NAMES = { _"Rest", _"Memory", _"Triumph", _"Vestige", _"Point", _"Passage", _"Domain", _"Route", _"Rock" }

local STR_BONUS_ROCK_CAIRN_CAVE_NAMES = { _"Rest", _"Memory", _"Vestige", _"Grotto", _"Passage", _"Domain", _"Secret", _"Curse", _"Rock" }

local STR_BONUS_DOLMEN_NAMES = { _"Rest", _"Memory", _"Triumph", _"Vestige", _"Point", _"Passage", _"Domain", _"Route", _"Ruin", _"Road", _"Path" }

local STR_BONUS_MONOLITH_NAMES = { _"Rest", _"Memory", _"Triumph", _"Vestige", _"Point", _"Path", _"Domain", _"Route", _"Monolith", _"Road" }

local STR_BONUS_OBELISK_NAMES = { _"Rest", _"Memory", _"Triumph", _"Vestige", _"Point", _"Path", _"Domain", _"Route", _"Obelisk", _"Road" }

local STR_BONUS_WELL_GRASS_NAMES = { _"Rest", _"Lair", _"Exile", _"Point", _"Passage", _"Ruin", _"Route", _"Refuge", _"Enclave", _"Domain" }

local STR_BONUS_WELL_ROAD_NAMES = { _"Rest", _"Point", _"Desire", _"Route", _"Passage", _"Refuge", _"Enclave", _"Path", _"Road", _"Market" }

local STR_BONUS_WELL_CAVE_NAMES = { _"Rest", _"Lair", _"Exile", _"Passage", _"Ruin", _"Well", _"Mine", _"Grotto", _"Den", _"Secret" }

local STR_BONUS_WELL_CRYSTAL_NAMES = { _"Bane", _"Lair", _"Curse", _"Passage", _"Grotto", _"Cave", _"Mine", _"Site", _"Quarry", _"Secret" }

local STR_BONUS_MONOLITH_HILLS_NAMES = { _"Rest", _"Point", _"Passage", _"Ruin", _"Lair", _"Exile", _"Plateau", _"Cliff", _"Domain", _"Route", _"Vestige" }

local STR_BONUS_LIGHTHOUSE_NAMES = { _"Port", _"Seaport", _"Harbor", _"Enclave", _"Bay", _"Coast", _"Shore", _"Lighthouse", _"Domain" }

local STR_BONUS_CAMPFIRE_NAMES = { _"Port", _"Seaport", _"Harbor", _"Bay", _"Coast", _"Shore", _"Cliff", _"Domain", _"Signal", _"Beacon", _"Enclave" }

local STR_BONUS_LILIES_NAMES = { _"Point", _"Passage", _"Lair", _"Rest", _"Waters", _"Domain", _"Curse", _"Secret", _"Hideout", _"Bane" }

local STR_BONUS_LILIES_SWAMP_NAMES = { _"Point", _"Passage", _"Rest", _"Lair", _"Exile", _"Domain", _"Curse", _"Secret", _"Hideout", _"Bane", _"Marsh" }

local STR_BONUS_ROCK_SAND_NAMES = { _"Rest", _"Point", _"Passage", _"Ruin", _"Lair", _"Exile", _"Delirium", _"Graveyard", _"Cemetery", _"Ossuary", _"Domain", _"Route", _"Vestige" }

local STR_BONUS_ROCK_NAMES = { _"Rest", _"Point", _"Passage", _"Ruin", _"Lair", _"Exile", _"Domain", _"Route", _"Vestige" }

local STR_BONUS_DOLMEN_GRASS_NAMES = { _"Rest", _"Point", _"Passage", _"Ruin", _"Lair", _"Domain", _"Route", _"Vestige", _"Conclave", _"Graveyard" }

local STR_BONUS_WINDMILL_NAMES = { _"Meadow", _"Prairie", _"Grassland", _"Refuge", _"Windmill", _"Enclave", _"Domain", _"Farm", _"Granary", _"Field", _"Shire" }

local STR_BONUS_TENT_NAMES = { _"Exile", _"Route", _"Enclave", _"Domain", _"Outpost", _"Refuge" }

local STR_BONUS_TENT_FANCY_GRASS_NAMES = { _"Exile", _"Route", _"Enclave", _"Domain", _"Outpost", _"Refuge", _"Festival", _"Fair", _"Tournament" }

local STR_BONUS_TENT_FANCY_ROAD_NAMES = { _"Route", _"Road", _"Path", _"Enclave", _"Domain", _"Outpost", _"Refuge", _"Festival", _"Fair", _"Tournament", _"Market", _"Armory" }

local STR_BONUS_SHELTER_NAMES = { _"Passage", _"Exile", _"Route", _"Refuge", _"Domain", _"Shelter", _"Shelter", _"Quarry", _"Plateau", _"Site", _"Outpost", _"Enclave", _"Passage" }

local STR_BONUS_TENT_OUTPOST_NAMES = { _"Exile", _"Route", _"Refuge", _"Domain", _"Outpost", _"Site", _"Expedition", _"Enclave" }

local STR_BONUS_ALTAR_NAMES = { _"Rest", _"Passage", _"Ruin", _"Lair", _"Oracle", _"Altar", _"Secret", _"Ossuary", _"Conclave", _"Curse", _"Grotto" }

local STR_BONUS_ALTAR_CRYSTAL_NAMES = { _"Passage", _"Lair", _"Curse", _"Domain", _"Bane", _"Cave", _"Secret" }

local STR_BONUS_TEMPLE_NAMES = { _"Rest", _"Ruin", _"Lair", _"Oracle", _"Temple", _"Sanctuary", _"Shrine", _"Conclave", _"Refuge", _"Library", _"Academy", _"School", _"Mausoleum", _"Crypt", _"Memory" }

local STR_BONUS_TEMPLE_ROAD_NAMES = { _"Rest", _"Route", _"Oracle", _"Temple", _"Sanctuary", _"Shrine", _"Refuge", _"Road", _"Library", _"Path", _"Market", _"Academy", _"School", _"Conclave", _"Mausoleum", _"Crypt" }

local STR_BONUS_TEMPLE_GRASS_NAMES = { _"Rest", _"Ruin", _"Oracle", _"Temple", _"Sanctuary", _"Shrine", _"Refuge", _"Library", _"Academy", _"School", _"Conclave", _"Mausoleum", _"Crypt", _"Market" }

local STR_BONUS_TEMPLE_MOUNTAIN_NAMES = { _"Rest", _"Passage", _"Ruin", _"Lair", _"Exile", _"Oracle", _"Temple", _"Sanctuary", _"Shrine", _"Refuge", _"Peak", _"Domain", _"Conclave", _"Mausoleum", _"Crypt", _"Memory" }

local STR_BONUS_TEMPLE_HILLS_NAMES = { _"Rest", _"Passage", _"Ruin", _"Lair", _"Refuge", _"Cliff", _"Domain", _"Route", _"Sanctuary", _"Conclave", _"Temple", _"Shrine", _"Oracle", _"Crypt", _"Mausoleum", _"Route" }

local STR_BONUS_TEMPLE_GREEN_NAMES = { _"Rest", _"Passage", _"Ruin", _"Lair", _"Exile", _"Oracle", _"Domain", _"Curse", _"Bane", _"Vestige", _"Perversion", _"Decay", _"Moor", _"Crypt", _"Mausoleum", _"Ossuary" }

local STR_BONUS_TREE_NAMES = { _"Conclave", _"Enclave", _"Meadow", _"Prairie", _"Field", _"Route", _"Refuge", _"Tree", _"Glade", _"Domain", _"Shire" }

local STR_BONUS_ROCK_TOWER_NAMES = { _"Rest", _"Vestige", _"Lair", _"Domain", _"Tower", _"Ruin", _"Refuge", _"Sanctuary", _"Observatory", _"Observatory", _"Enclave", _"Conclave", _"Crypt" }

local STR_BONUS_ROCK_SWAMP_NAMES = { _"Rest", _"Point", _"Decay", _"Passage", _"Ruin", _"Lair", _"Exile", _"Bog", _"Domain", _"Bane", _"Curse", _"Vestige", _"Secret" }

local STR_BONUS_GENERIC_NAMES = { _"Rest", _"Point", _"Passage", _"Ruin", _"Lair", _"Exile", _"Vestige" }

local strings = {
	detritus = STR_BONUS_DETRITUS_NAMES,
	detritus2 = STR_BONUS_DETRITUS_NAMES,
	lilies = STR_BONUS_LILIES_NAMES,
	lilies_s = STR_BONUS_LILIES_SWAMP_NAMES,
	oak1 = STR_BONUS_TREE_NAMES,
	oak2 = STR_BONUS_TREE_NAMES,
	oak3 = STR_BONUS_TREE_NAMES,
	oak4 = STR_BONUS_TREE_NAMES,
	oak5 = STR_BONUS_TREE_NAMES,
	oak6 = STR_BONUS_TREE_NAMES,
	oak7 = STR_BONUS_TREE_NAMES,
	oak_dead = STR_BONUS_DEAD_OAK_NAMES,
	oak_dead2 = STR_BONUS_DEAD_OAK_NAMES,
	obelisk = STR_BONUS_OBELISK_NAMES,
	rock1 = STR_BONUS_ROCK_SAND_NAMES,
	rock3 = STR_BONUS_ROCK_SWAMP_NAMES,
	rock4 = STR_BONUS_ROCK_NAMES,
	tower_r1 = STR_BONUS_ROCK_TOWER_NAMES,
	tower_r4 = STR_BONUS_GENERIC_NAMES,
	crystal = STR_BONUS_WELL_CRYSTAL_NAMES,
	crystal3 = STR_BONUS_ALTAR_CRYSTAL_NAMES,
	ship1 = STR_BONUS_SHIP_NAMES,
	ship2 = STR_BONUS_SHIP_NAMES,
	bones = STR_BONUS_BONES_NAMES,
	bones_s = STR_BONUS_BONES_SWAMP_NAMES,
	village = STR_BONUS_VILLAGE_RUINED_NAMES,
	burial = STR_BONUS_BURIAL_NAMES,
	burial_s = STR_BONUS_BURIAL_SAND_NAMES,
	burial_c = STR_BONUS_BURIAL_CAVE_NAMES,
	trapdoor = STR_BONUS_TRAPDOOR_NAMES,
	coffin = STR_BONUS_COFFIN_NAMES,
	mine = STR_BONUS_MINE_NAMES,
	doors = STR_BONUS_DOORS_NAMES,
	tent1 = STR_BONUS_TENT_NAMES,
	shelter = STR_BONUS_SHELTER_NAMES,
	tent2 = STR_BONUS_TENT_NAMES,
	tent2_g = STR_BONUS_TENT_FANCY_GRASS_NAMES,
	tent2_r = STR_BONUS_TENT_FANCY_ROAD_NAMES,
	shop = STR_BONUS_SHOP_NAMES,
	shop_g = STR_BONUS_SHOP_GRASS_NAMES,
	outpost = STR_BONUS_TENT_OUTPOST_NAMES,
	signpost = STR_BONUS_SIGNPOST_NAMES,
	rock_cairn = STR_BONUS_ROCK_CAIRN_NAMES,
	rock_cairn_c = STR_BONUS_ROCK_CAIRN_CAVE_NAMES,
	dolmen = STR_BONUS_DOLMEN_NAMES,
	monolith1 = STR_BONUS_MONOLITH_HILLS_NAMES,
	monolith1_r = STR_BONUS_GENERIC_NAMES,
	monolith2 = STR_BONUS_MONOLITH_NAMES,
	monolith3 = STR_BONUS_ROCK_NAMES,
	dolmen_g = STR_BONUS_DOLMEN_GRASS_NAMES,
	monolith4 = STR_BONUS_MONOLITH_HILLS_NAMES,
	monolith4_r = STR_BONUS_GENERIC_NAMES,
	well_g = STR_BONUS_WELL_GRASS_NAMES,
	well_r = STR_BONUS_WELL_ROAD_NAMES,
	well = STR_BONUS_WELL_CAVE_NAMES,
	lighthouse = STR_BONUS_LIGHTHOUSE_NAMES,
	campfire = STR_BONUS_CAMPFIRE_NAMES,
	temple = STR_BONUS_TEMPLE_NAMES,
	temple2 = STR_BONUS_TEMPLE_ROAD_NAMES,
	temple2_g = STR_BONUS_TEMPLE_GRASS_NAMES,
	temple3 = STR_BONUS_TEMPLE_MOUNTAIN_NAMES,
	temple4 = STR_BONUS_TEMPLE_HILLS_NAMES,
	temple_green_h = STR_BONUS_TEMPLE_GREEN_NAMES,
	temple_green_h2 = STR_BONUS_TEMPLE_GREEN_NAMES,
	temple_green_g = STR_BONUS_TEMPLE_GREEN_NAMES,
	temple_green_g2 = STR_BONUS_TEMPLE_GREEN_NAMES,
	altar = STR_BONUS_ALTAR_NAMES,
	windmill = STR_BONUS_WINDMILL_NAMES
}

local IMG_DETRITUS = "misc/blank-hex.png~BLIT(terrain/misc/detritus/detritusC-3.png~CROP(0,18,66,54),6,13)~BLIT(terrain/misc/detritus/detritusC-2.png~CROP(8,0,35,70),0,2)~BLIT(terrain/misc/detritus/detritusC-2.png~CROP(43,44,29,26),36,46)"

local IMG_DETRITUS_2 = "misc/blank-hex.png~BLIT(terrain/misc/detritus/detritusC-2.png~CROP(0,4,59,68),13,0)~BLIT(terrain/misc/detritus/detritusC-5.png~CROP(17,0,55,63),0,9)"

local IMG_LILIES = "terrain/embellishments/water-lilies-flower-small4.png~BLIT(terrain/embellishments/water-lilies-flower-small5.png~CROP(40,28,17,17),40,28)"

local IMG_LILIES_2 = "misc/blank-hex.png~BLIT(terrain/embellishments/water-lilies-flower5.png~CROP(21,12,72,72))~BLIT(terrain/embellishments/water-lilies-flower-small2.png~CROP(0,8,55,64),17,0)"

local IMG_OAK_1 = "terrain/embellishments/flowers-mixed4.png~MASK(scenery/whirlpool.png)~BLIT(scenery/oak-leaning.png)"

local IMG_OAK_2 = "terrain/embellishments/flowers-mixed3.png~MASK(scenery/whirlpool.png)~BLIT(scenery/oak-leaning.png)"

-- custom image
local IMG_OAK_3 = "scenery/treehouse.png"

-- custom image
local IMG_OAK_4 = "scenery/treehouse.png"

local IMG_OAK_5 = "terrain/embellishments/flower-purple.png~MASK(scenery/whirlpool.png)~BLIT(scenery/oak-leaning.png)"

-- custom image
local IMG_OAK_6 = "scenery/treehouse.png"

-- custom image
local IMG_OAK_7 = "scenery/treehouse.png"

local IMG_ROCK_1 = "misc/blank-hex.png~BLIT(scenery/rock1.png~CROP(1,11,71,61),0,0)"

local IMG_ROCK_3 = "misc/blank-hex.png~BLIT(scenery/rock3.png~CROP(0,11,66,61),6,0)"

local IMG_ROCK_4 = "misc/blank-hex.png~BLIT(scenery/rock4.png~CROP(0,8,66,64),6,0)"

-- custom image
local IMG_ROCK_1_TOWER = "misc/blank-hex.png~BLIT(scenery/wct-tower.png)"

-- custom image
local IMG_ROCK_4_TOWER = "misc/blank-hex.png~BLIT(scenery/wct-tower.png)"

-- custom image
local IMG_OBELISK_POST = "misc/blank-hex.png~BLIT(scenery/wct-obelisk.png)"

-- custom image
local IMG_ROCK_CAIRN_DOLMEN = "misc/blank-hex.png~BLIT(scenery/wct-dolmen.png~SCALE(75,75)~CROP(0,0,72,62),0,10)"

-- custom image
local IMG_MONOLITH_DOLMEN = "misc/blank-hex.png~BLIT(scenery/wct-dolmen2.png~SCALE(75,75)~CROP(0,0,72,62),0,10)"

-- custom image
local IMG_TEMPLE_2 = "misc/blank-hex.png~BLIT(scenery/wct-temple.png~SCALE(60,60),6,3)"

-- custom image
local IMG_TEMPLE_3 = "scenery/wct-temple2.png"

-- custom image
local IMG_TEMPLE_GREEN_HILLS = "misc/blank-hex.png~BLIT(scenery/wct-temple4.png~SCALE(65,65),4,4)"

-- custom image
local IMG_TEMPLE_GREEN_HILLS_2 = "misc/blank-hex.png~BLIT(scenery/wct-temple5.png~SCALE(63,63),6,2)"

-- custom image
local IMG_TEMPLE_GREEN_GRASS = "misc/blank-hex.png~BLIT(scenery/wct-temple3.png)"

-- custom image
local IMG_TEMPLE_GREEN_GRASS_2 = "misc/blank-hex.png~BLIT(scenery/wct-temple5.png~SCALE(63,63),6,2)"

-- custom image
local IMG_DETRITUS_OAK_1 = "scenery/wct-oak-dead.png"

-- custom image
local IMG_DETRITUS_OAK_2 = "scenery/wct-oak-dead2.png"

-- custom image
local IMG_CRYSTAL_WELL = "scenery/wct-crystal.png"

-- custom image
local IMG_CRYSTALS_ALTAR = "misc/blank-hex.png~BLIT(scenery/wct-crystal3.png~SCALE(59,59),6,4)"

-- custom image
local IMG_TENT_OUTPOST = "scenery/wct-outpost.png"


local images = {
	rock1 = IMG_ROCK_1,
	rock3 = IMG_ROCK_3,
	rock4 = IMG_ROCK_4,
	tower_r1 = IMG_ROCK_1_TOWER,
	tower_r4 = IMG_ROCK_4_TOWER,
	lilies = IMG_LILIES,
	lilies_s = IMG_LILIES_2,
	detritus = IMG_DETRITUS,
	detritus2 = IMG_DETRITUS_2,
	oak1 = IMG_OAK_1,
	oak2 = IMG_OAK_2,
	oak3 = IMG_OAK_3,
	oak4 = IMG_OAK_4,
	oak5 = IMG_OAK_5,
	oak6 = IMG_OAK_6,
	oak7 = IMG_OAK_7,
	obelisk = IMG_OBELISK_POST,
	dolmen = IMG_ROCK_CAIRN_DOLMEN,
	dolmen_g = IMG_MONOLITH_DOLMEN,
	oak_dead = IMG_DETRITUS_OAK_1,
	oak_dead2 = IMG_DETRITUS_OAK_2,
	crystal = IMG_CRYSTAL_WELL,
	crystal3 = IMG_CRYSTALS_ALTAR,
	outpost = IMG_TENT_OUTPOST,
	ship1 = "scenery/shipwreck-1.png",
	ship2 = "scenery/wreck.png",
	bones = "items/bones.png",
	bones_s = "items/bones.png",
	village = "scenery/village-human-burned2.png",
	burial = "items/burial.png",
	burial_s = "items/burial.png",
	burial_c = "items/burial.png",
	trapdoor = "scenery/trapdoor-open.png",
	coffin = "items/coffin-closed.png",
	mine = "scenery/mine-abandoned.png",
	doors = "scenery/dwarven-doors-closed.png",
	tent1 = "scenery/tent-ruin-1.png",
	shelter = "scenery/tent-ruin-1.png",
	tent2 = "scenery/tent-fancy-red.png",
	tent2_g = "scenery/tent-fancy-red.png",
	tent2_r = "scenery/tent-fancy-red.png",
	shop = "scenery/tent-shop-weapons.png",
	shop_g = "scenery/tent-shop-weapons.png",
	signpost = "scenery/signpost.png",
	rock_cairn = "scenery/rock-cairn.png",
	rock_cairn_c = "scenery/rock-cairn.png",
	monolith1 = "scenery/monolith1.png",
	monolith1_r = "scenery/monolith1.png",
	monolith2 = "scenery/monolith2.png",
	monolith3 = "scenery/monolith3.png",
	monolith4 = "scenery/monolith4.png",
	monolith4_r = "scenery/monolith4.png",
	well_g = "scenery/well.png",
	well_r = "scenery/well.png",
	well = "scenery/well.png",
	lighthouse = "scenery/lighthouse.png",
	temple = "scenery/temple1.png",
	temple2 = IMG_TEMPLE_2,
	temple2_g = IMG_TEMPLE_2,
	temple3 = IMG_TEMPLE_2,
	temple4 = IMG_TEMPLE_3,
	temple_green_h = IMG_TEMPLE_GREEN_HILLS,
	temple_green_h2 = IMG_TEMPLE_GREEN_HILLS_2,
	temple_green_g = IMG_TEMPLE_GREEN_GRASS,
	temple_green_g2 = IMG_TEMPLE_GREEN_GRASS_2,
	altar = "items/altar.png",
}
return strings, images
