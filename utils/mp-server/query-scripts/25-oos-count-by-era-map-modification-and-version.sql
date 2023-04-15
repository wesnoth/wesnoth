select count(*) as OOS_COUNT, eras.ID as ERA, eras.ADDON_ID as ERA_ADDON, eras.ADDON_VERSION as ERA_VERSION, scenarios.ID as MAP, scenarios.ADDON_ID as MAP_ADDON, scenarios.ADDON_VERSION as MAP_VERSION, IFNULL(GROUP_CONCAT(distinct concat(mods.ID,'(',mods.ADDON_ID,':',mods.ADDON_VERSION,')') SEPARATOR '|'), 'No Modifications') AS MODIFICATIONS
from wesnothd_game_info game
inner join wesnothd_game_content_info scenarios
   on game.INSTANCE_UUID = scenarios.INSTANCE_UUID
	and game.GAME_ID = scenarios.GAME_ID
	and scenarios.TYPE = 'scenario'
inner join wesnothd_game_content_info eras
   on game.INSTANCE_UUID = eras.INSTANCE_UUID
	and game.GAME_ID = eras.GAME_ID
	and eras.TYPE = 'era'
left join wesnothd_game_content_info mods
  on game.INSTANCE_UUID = mods.INSTANCE_UUID
 and game.GAME_ID = mods.GAME_ID
 and mods.TYPE = 'modification'
where game.OOS = 1
	and YEAR(game.START_TIME) = YEAR(CURRENT_DATE - INTERVAL 1 MONTH)
  and MONTH(game.START_TIME) = MONTH(CURRENT_DATE - INTERVAL 1 MONTH)
  and game.END_TIME is not NULL
  and TIMESTAMPDIFF(MINUTE, game.START_TIME, game.END_TIME) > 5
group by eras.ID, eras.ADDON_ID, eras.ADDON_VERSION, scenarios.ID, scenarios.ADDON_ID, scenarios.ADDON_VERSION
order by count(*) desc, eras.ID, scenarios.ID, mods.ID
