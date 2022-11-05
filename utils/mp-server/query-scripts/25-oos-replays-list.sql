select eras.ID as ERA, scenarios.ID as MAP, IFNULL(GROUP_CONCAT(distinct mods.ID SEPARATOR '|'), 'No Modifications') AS MODIFICATIONS, concat('https://replays.wesnoth.org/', substring(game.INSTANCE_VERSION, 1, 4), '/', year(game.END_TIME), '/', lpad(month(game.END_TIME), 2, '0'), '/', lpad(day(game.END_TIME), 2, '0'), '/', game.REPLAY_NAME) AS REPLAY_LOCATION
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
group by game.INSTANCE_UUID, game.GAME_ID
order by eras.ID, scenarios.ID, mods.ID
