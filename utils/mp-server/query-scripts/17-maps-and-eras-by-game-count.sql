select c1.ID as MAP, c2.ID as ERA, count(*) as COUNT
from wesnothd_game_info game, wesnothd_game_content_info c1, wesnothd_game_content_info c2
where YEAR(game.START_TIME) = YEAR(CURRENT_DATE - INTERVAL 1 MONTH)
  and MONTH(game.START_TIME) = MONTH(CURRENT_DATE - INTERVAL 1 MONTH)
  and game.END_TIME is not NULL
  and TIMESTAMPDIFF(MINUTE, game.START_TIME, game.END_TIME) > 5
  and game.INSTANCE_UUID = c1.INSTANCE_UUID
  and game.GAME_ID = c1.GAME_ID
  and c1.TYPE = 'scenario'
  and game.INSTANCE_UUID = c2.INSTANCE_UUID
  and game.GAME_ID = c2.GAME_ID
  and c2.TYPE = 'era'
group by c1.ID, c2.ID
order by count(*) desc
