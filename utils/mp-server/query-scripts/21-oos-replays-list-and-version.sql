select game.ERA_NAME, game.ERA_SOURCE_ADDON, game.ERA_VERSION, game.MAP_NAME, game.MAP_SOURCE_ADDON, game.MAP_VERSION, IFNULL(GROUP_CONCAT(distinct concat(modif.MODIFICATION_NAME,'(',modif.SOURCE_ADDON,':',modif.VERSION,')') SEPARATOR '|'), 'No Modifications') AS MODIFICATIONS, concat('https://replays.wesnoth.org/', substring(INSTANCE_VERSION, 1, 4), '/', year(game.END_TIME), '/', lpad(month(game.END_TIME), 2, '0'), '/', lpad(day(game.END_TIME), 2, '0'), '/', game.REPLAY_NAME) AS REPLAY_LOCATION
from wesnothd_game_info game
left join wesnothd_game_modification_info modif
	 on game.INSTANCE_UUID = modif.INSTANCE_UUID
	and game.GAME_ID = modif.GAME_ID
where game.OOS = 1
	and YEAR(game.START_TIME) = YEAR(CURRENT_DATE - INTERVAL 1 MONTH)
  and MONTH(game.START_TIME) = MONTH(CURRENT_DATE - INTERVAL 1 MONTH)
  and game.END_TIME is not NULL
  and TIMESTAMPDIFF(MINUTE, game.START_TIME, game.END_TIME) > 5
group by game.INSTANCE_UUID, game.GAME_ID
order by game.ERA_NAME, game.MAP_NAME, modif.MODIFICATION_NAME
