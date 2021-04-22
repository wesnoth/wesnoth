
-- The rules for enemy recruitlist are as follows:
-- each side Has a 'faction' (here: group) a leader from that faction and maybe a leader from another faction.
-- the enemy side can then recruit all units from its faction and from all it's leaders, that why having
-- unit {recruit= that are already contained in {recruit= is not redundant.
local enemy_army = {}
enemy_army.group = {
	{
		id = "northerners",
		recruit = {"Orcish Grunt","Orcish Archer","Wolf Rider","Orcish Assassin","Troll Whelp"},
		recall = {
			level2 = {"Orcish Ruler","Orcish Slayer","Orcish Crossbowman","Troll Rocklobber","Troll","Orcish Warrior","Goblin Pillager","Goblin Knight","Orcish Crossbowman","Troll","Orcish Warrior","Troll Hero"},
			level3 = {"Orcish Warlord","Troll Warrior","Orcish Slurbow","Orcish Sovereign","Direwolf Rider","Orcish Warlord","Troll Warrior","Orcish Slurbow","Great Troll","Direwolf Rider"},
		},
		commander = {
			level1 = {"Orcish Leader","Orcish Grunt","Orcish Archer","Orcish Assassin"},
			level2 = {"Orcish Ruler","Troll Hero","Orcish Slayer","Orcish Crossbowman","Troll Rocklobber","Troll","Orcish Warrior"},
			level3 = {"Orcish Warlord","Troll Warrior","Orcish Slurbow","Orcish Sovereign","Great Troll"},
		},
		leader = {
			{
				level2 = "Orcish Warrior",
				level3 = "Orcish Warlord",
				recruit = {"Orcish Grunt","Orcish Archer","Wolf Rider","Orcish Assassin"},
			},
			{
				level2 = "Troll",
				level3 = "Troll Warrior",
				recruit = {"Orcish Grunt","Orcish Archer","Orcish Assassin","Troll Whelp"},
			},
			{
				level2 = "Troll Rocklobber",
				level3 = "Great Troll",
				recruit = {"Orcish Grunt","Orcish Assassin","Wolf Rider","Troll Whelp"},
			},
			{
				level2 = "Orcish Crossbowman",
				level3 = "Orcish Slurbow",
				recruit = {"Orcish Grunt","Orcish Archer","Wolf Rider","Orcish Assassin"},
			},
			{
				level2 = "Orcish Slayer",
				level3 = "Naga Myrmidon",
				recruit = {"Orcish Grunt","Orcish Archer","Wolf Rider","Orcish Assassin","Naga Fighter"},
			},
			{
				level2 = "Goblin Knight",
				level3 = "Direwolf Rider",
				recruit = {"Goblin Spearman","Orcish Leader","Wolf Rider","Orcish Archer","Troll Whelp"},
			},
			{
				level2 = "Orcish Ruler",
				level3 = "Orcish Sovereign",
				recruit = {"Orcish Grunt","Orcish Leader","Wolf Rider","Orcish Archer"},
			},
		}
	},
	{
		id = "loyalists",
		recruit = {"Spearman","Bowman","Cavalryman","Fencer","Mage"},
		recall = {
			level2 = {"White Mage","Red Mage","Duelist","Longbowman","Shock Trooper","Pikeman","Swordsman","Lieutenant","Dragoon","Knight","Javelineer","Pikeman","Swordsman","Longbowman"},
			level3 = {"General","Halberdier","Royal Guard","Silver Mage","Iron Mauler","Master Bowman","Master at Arms","Arch Mage","Mage of Light","Paladin","Grand Knight","Cavalier","General","Halberdier","Royal Guard","Iron Mauler","Master Bowman","Master at Arms"},
		},
		commander = {
			level1 = {"Sergeant","Spearman","Bowman","Mage","Fencer","Heavy Infantryman"},
			level2 = {"White Mage","Red Mage","Duelist","Longbowman","Shock Trooper","Pikeman","Swordsman","Lieutenant","Javelineer"},
			level3 = {"General","Halberdier","Royal Guard","Silver Mage","Iron Mauler","Master Bowman","Master at Arms","Arch Mage","Mage of Light"},
		},
		leader = {
			{
				level2 = "Lieutenant",
				level3 = "General",
				recruit = {"Spearman","Bowman","Cavalryman","Fencer","Mage"},
			},
			{
				level2 = "Swordsman",
				level3 = "Royal Guard",
				recruit = {"Spearman","Bowman","Cavalryman","Heavy Infantryman","Mage"},
			},
			{
				level2 = "Pikeman",
				level3 = "Halberdier",
				recruit = {"Spearman","Bowman","Cavalryman","Horseman","Mage"},
			},
			{
				level2 = "Javelineer",
				level3 = "Silver Mage",
				recruit = {"Spearman","Bowman","Horseman","Fencer","Mage"},
			},
			{
				level2 = "Shock Trooper",
				level3 = "Iron Mauler",
				recruit = {"Spearman","Mage","Cavalryman","Heavy Infantryman"},
			},
			{
				level2 = "Longbowman",
				level3 = "Master Bowman",
				recruit = {"Spearman","Bowman","Cavalryman","Horseman","Heavy Infantryman"},
			},
			{
				level2 = "Duelist",
				level3 = "Master at Arms",
				recruit = {"Spearman","Bowman","Cavalryman","Fencer","Mage"},
			},
			{
				level2 = "Red Mage",
				level3 = "Arch Mage",
				recruit = {"Spearman","Bowman","Mage","Cavalryman","Heavy Infantryman"},
			},
			{
				level2 = "White Mage",
				level3 = "Mage of Light",
				recruit = {"Spearman","Heavy Infantryman","Cavalryman","Mage","Bowman"},
			},
		}
	},
	{
		id = "elves",
		recruit = {"Elvish Fighter","Elvish Archer","Elvish Shaman","Elvish Scout","Wose"},
		recall = {
			level2 = {"Elder Wose","Elvish Sorceress","Elvish Druid","Elvish Marksman","Elvish Ranger","Elvish Hero","Elvish Captain","Elvish Rider","Elder Wose","Elvish Hero","Elder Wose","Elvish Sorceress","Red Mage","Elvish Marksman","Elvish Ranger","Elvish Hero","Elvish Captain","Elvish Rider","Elvish Ranger","Elvish Hero"},
			level3 = {"Elvish Marshal","Elvish Champion","Elvish Avenger","Elvish Sharpshooter","Elvish Shyde","Elvish Enchantress","Ancient Wose","Elvish Outrider","Ancient Wose","Elvish Champion","Elvish Marshal","Elvish Champion","Elvish Avenger","Elvish Sharpshooter","Arch Mage","Elvish Enchantress","Ancient Wose","Elvish Outrider","Elvish Avenger","Elvish Champion"},
		},
		commander = {
			level1 = {"Elvish Shaman","Elvish Fighter","Elvish Archer","Wose"},
			level2 = {"Elder Wose","Elvish Sorceress","Elvish Druid","Elvish Marksman","Elvish Ranger","Elvish Hero","Elvish Captain","Elvish Lord"},
			level3 = {"Elvish Marshal","Elvish Champion","Elvish Avenger","Elvish Sharpshooter","Elvish Shyde","Elvish Enchantress","Ancient Wose","Arch Mage","Elvish High Lord"},
		},
		leader = {
			{
				level2 = "Elvish Captain",
				level3 = "Elvish Marshal",
				recruit = {"Elvish Fighter","Elvish Archer","Elvish Shaman","Elvish Scout"},
			},
			{
				level2 = "Elvish Hero",
				level3 = "Elvish Champion",
				recruit = {"Elvish Fighter","Elvish Archer","Elvish Shaman","Elvish Scout"},
			},
			{
				level2 = "Elvish Ranger",
				level3 = "Elvish Avenger",
				recruit = {"Elvish Fighter","Elvish Archer","Wose","Elvish Scout"},
			},
			{
				level2 = "Elvish Marksman",
				level3 = "Elvish Sharpshooter",
				recruit = {"Elvish Fighter","Elvish Archer","Mage","Elvish Scout"},
			},
			{
				level2 = "Elvish Druid",
				level3 = "Elvish Shyde",
				recruit = {"Elvish Fighter","Elvish Archer","Elvish Shaman","Wose"},
			},
			{
				level2 = "Elvish Sorceress",
				level3 = "Elvish Enchantress",
				recruit = {"Elvish Fighter","Elvish Archer","Elvish Shaman","Elvish Scout","Mage"},
			},
			{
				level2 = "Elder Wose",
				level3 = "Ancient Wose",
				recruit = {"Elvish Shaman","Elvish Fighter","Mage"," Wose"},
			},
		}
	},
	{
		id = "knalgans",
		recruit = {"Dwarvish Fighter","Dwarvish Thunderer","Thief","Footpad","Poacher"},
		recall = {
			level2 = {"Dwarvish Stalwart","Dwarvish Thunderguard","Dwarvish Steelclad","Rogue","Trapper","Gryphon Master","Bandit","Outlaw","Dwarvish Stalwart","Dwarvish Thunderguard","Dwarvish Steelclad","Dwarvish Berserker"},
			level3 = {"Dwarvish Lord","Dwarvish Dragonguard","Dwarvish Sentinel","Assassin","Huntsman","Fugitive","Highwayman","Dwarvish Lord","Dwarvish Dragonguard","Dwarvish Sentinel"},
		},
		commander = {
			level1 = {"Dwarvish Thunderer","Dwarvish Fighter","Dwarvish Guardsman","Dwarvish Scout","Thief","Poacher"},
			level2 = {"Dwarvish Stalwart","Dwarvish Thunderguard","Dwarvish Steelclad","Dwarvish Pathfinder","Rogue","Trapper"},
			level3 = {"Dwarvish Lord","Dwarvish Dragonguard","Dwarvish Sentinel","Assassin","Huntsman","Highwayman","Dwarvish Explorer"},
		},
		leader = {
			{
				level2 = "Dwarvish Steelclad",
				level3 = "Dwarvish Lord",
				recruit = {"Dwarvish Fighter","Dwarvish Thunderer","Dwarvish Ulfserker","Dwarvish Guardsman"},
			},
			{
				level2 = "Dwarvish Thunderguard",
				level3 = "Dwarvish Dragonguard",
				recruit = {"Dwarvish Fighter","Dwarvish Thunderer","Gryphon Rider","Poacher"},
			},
			{
				level2 = "Dwarvish Stalwart",
				level3 = "Dwarvish Sentinel",
				recruit = {"Footpad","Dwarvish Thunderer","Gryphon Rider","Dwarvish Guardsman"},
			},
			{
				level2 = "Rogue",
				level3 = "Assassin",
				recruit = {"Thug","Thief","Footpad","Dwarvish Ulfserker","Gryphon Rider"},
			},
			{
				level2 = "Trapper",
				level3 = "Huntsman",
				recruit = {"Thug","Thief","Poacher","Dwarvish Fighter","Dwarvish Thunderer"},
			},
			{
				level2 = "Bandit",
				level3 = "Highwayman",
				recruit = {"Thug","Thief","Dwarvish Thunderer","Dwarvish Ulfserker","Dwarvish Fighter"},
			},
		}
	},
	{
		id = "drakes",
		recruit = {"Drake Fighter","Drake Clasher","Drake Glider","Drake Burner","Saurian Augur"},
		recall = {
			level2 = {"Drake Arbiter","Drake Thrasher","Drake Warrior","Fire Drake","Drake Flare","Saurian Oracle","Saurian Soothsayer","Saurian Ambusher","Drake Warrior","Drake Thrasher","Sky Drake"},
			level3 = {"Drake Flameheart","Inferno Drake","Drake Blademaster","Drake Blademaster","Drake Enforcer","Drake Warden","Saurian Flanker","Saurian Flanker","Hurricane Drake"},
		},
		commander = {
			level1 = {"Drake Burner","Drake Fighter","Drake Clasher","Saurian Augur","Saurian Skirmisher"},
			level2 = {"Drake Arbiter","Drake Thrasher","Drake Warrior","Fire Drake","Drake Flare","Saurian Ambusher","Saurian Oracle"},
			level3 = {"Drake Flameheart","Inferno Drake","Drake Blademaster","Drake Enforcer","Drake Warden","Saurian Flanker"},
		},
		leader = {
			{
				level2 = "Drake Flare",
				level3 = "Drake Flameheart",
				recruit = {"Drake Fighter","Drake Clasher","Drake Glider","Drake Burner"},
			},
			{
				level2 = "Fire Drake",
				level3 = "Inferno Drake",
				recruit = {"Drake Fighter","Saurian Augur","Drake Glider","Drake Burner"},
			},
			{
				level2 = "Drake Warrior",
				level3 = "Drake Blademaster",
				recruit = {"Drake Fighter","Saurian Skirmisher","Drake Glider","Drake Burner"},
			},
			{
				level2 = "Drake Thrasher",
				level3 = "Drake Enforcer",
				recruit = {"Drake Fighter","Drake Clasher","Saurian Skirmisher","Drake Burner"},
			},
			{
				level2 = "Drake Arbiter",
				level3 = "Drake Warden",
				recruit = {"Saurian Skirmisher","Drake Clasher","Drake Glider","Drake Burner"},
			},
			{
				level2 = "Saurian Oracle",
				level3 = "Saurian Flanker",
				recruit = {"Saurian Skirmisher","Saurian Augur","Drake Clasher","Drake Burner"},
			},
			{
				level2 = "Saurian Soothsayer",
				level3 = "Hurricane Drake",
				recruit = {"Saurian Skirmisher","Saurian Augur","Drake Glider","Drake Fighter"},
			},
		}
	},
	{
		id = "undead",
		recruit = {"Skeleton","Skeleton Archer","Ghost","Ghoul","Dark Adept"},
		recall = {
			level2 = {"Revenant","Deathblade","Bone Shooter","Dark Sorcerer","Necrophage","Wraith","Shadow","Revenant","Bone Shooter","Dark Sorcerer","Necrophage","Chocobone"},
			level3 = {"Draug","Death Knight","Necromancer","Lich","Ghast","Banebow","Spectre","Nightgaunt","Draug","Ghast","Banebow"},
		},
		commander = {
			level1 = {"Dark Adept","Skeleton Archer","Skeleton","Ghoul"},
			level2 = {"Revenant","Deathblade","Bone Shooter","Dark Sorcerer","Necrophage"},
			level3 = {"Draug","Death Knight","Necromancer","Lich","Ghast","Banebow"},
		},
		leader = {
			{
				level2 = "Revenant",
				level3 = "Draug",
				recruit = {"Skeleton","Skeleton Archer","Ghost","Ghoul"},
			},
			{
				level2 = "Deathblade",
				level3 = "Death Knight",
				recruit = {"Skeleton","Skeleton Archer","Dark Adept","Ghoul"},
			},
			{
				level2 = "Bone Shooter",
				level3 = "Banebow",
				recruit = {"Skeleton","Skeleton Archer","Ghost","Ghoul"},
			},
			{
				level2 = "Dark Sorcerer",
				level3 = "Lich",
				recruit = {"Skeleton Archer","Dark Adept","Ghoul","Vampire Bat"},
			},
			{
				level2 = "Necrophage",
				level3 = "Ghast",
				recruit = {"Skeleton","Dark Adept","Ghoul","Ghost"},
			},
		}
	},
	{
		id = "dunefolk",
		recruit = {"Dune Soldier","Dune Burner","Dune Rider","Dune Skirmisher","Dune Rover","Naga Dirkfang"},
		recall = {
			level2 = {"Dune Captain","Dune Strider","Dune Scorcher","Dune Alchemist","Dune Falconer","Dune Swordsman","Dune Horse Archer","Dune Sunderer","Dune Scorcher","Dune Apothecary","Dune Swordsman","Dune Spearguard"},
			level3 = {"Dune Blademaster","Dune Sky Hunter","Dune Firetrooper","Dune Warmaster","Dune Cataphract","Dune Blademaster","Dune Luminary","Dune Firetrooper","Dune Spearmaster","Dune Cataphract"},
		},
		commander = {
			level1 = {"Dune Soldier","Dune Soldier","Dune Burner","Dune Skirmisher"},
			level2 = {"Dune Captain","Dune Spearguard","Dune Strider","Dune Scorcher","Dune Alchemist","Dune Apothecary","Dune Swordsman"},
			level3 = {"Dune Blademaster","Dune Luminary","Dune Firetrooper","Dune Warmaster","Dune Spearmaster"},
		},
		leader = {
			{
				level2 = "Dune Swordsman",
				level3 = "Dune Blademaster",
				recruit = {"Dune Soldier","Dune Burner","Dune Rider","Dune Skirmisher"},
			},
			{
				level2 = "Dune Apothecary",
				level3 = "Dune Luminary",
				recruit = {"Dune Soldier","Dune Burner","Dune Skirmisher","Dune Herbalist"},
			},
			{
				level2 = "Dune Alchemist",
				level3 = "Dune Spearmaster",
				recruit = {"Dune Soldier","Dune Skirmisher","Dune Rider","Dune Herbalist"},
			},
			{
				level2 = "Dune Scorcher",
				level3 = "Dune Firetrooper",
				recruit = {"Dune Soldier","Dune Burner","Dune Rider","Dune Skirmisher"},
			},
			{
				level2 = "Dune Strider",
				level3 = "Dune Harrier",
				recruit = {"Dune Soldier","Dune Burner","Dune Rider","Dune Skirmisher","Naga Dirkfang"},
			},
			{
				level2 = "Dune Raider",
				level3 = "Dune Marauder",
				recruit = {"Dune Skirmisher","Dune Soldier","Dune Rider","Dune Burner","Dune Herbalist"},
			},
			{
				level2 = "Dune Captain",
				level3 = "Dune Warmaster",
				recruit = {"Dune Soldier","Dune Rover","Dune Rider","Dune Burner"},
			},
		}
	},
}
enemy_army.factions_available = {}
-- each faction can be picked up to 4 times along campaign
for i = 1, #enemy_army.group do
	for j = 1, 4 do
		table.insert(enemy_army.factions_available, i)
	end
end
-- each faction pick any faction as ally just 1 time
for i = 1, #enemy_army.group do
	local ally = {}
	enemy_army.group[i].allies_available = ally
	for j = 1, #enemy_army.group do
		if j ~= i then
			table.insert(ally, j)
		end
	end
end

return enemy_army
