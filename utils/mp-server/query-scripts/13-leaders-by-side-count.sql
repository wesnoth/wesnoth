select content.ID, player.LEADERS, count(*) as LEADER_COUNT
from wesnothd_game_info game
inner join wesnothd_game_player_info player
   on game.INSTANCE_UUID = player.INSTANCE_UUID
  and game.GAME_ID = player.GAME_ID
  and player.USER_ID != -1
  and player.FACTION != ''
inner join wesnothd_game_content_info content
   on game.INSTANCE_UUID = content.INSTANCE_UUID
  and game.GAME_ID = content.GAME_ID
  and content.TYPE = 'era'
where YEAR(game.START_TIME) = YEAR(CURRENT_DATE - INTERVAL 1 MONTH)
  and MONTH(game.START_TIME) = MONTH(CURRENT_DATE - INTERVAL 1 MONTH)
  and game.END_TIME is not NULL
  and TIMESTAMPDIFF(MINUTE, game.START_TIME, game.END_TIME) > 5
group by content.ID, player.LEADERS
order by count(*) desc

