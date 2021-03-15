select count(distinct player.USER_ID) as PLAYER_COUNT
from wesnothd_game_info game, wesnothd_game_player_info player
where YEAR(game.START_TIME) = YEAR(CURRENT_DATE - INTERVAL 1 MONTH)
  and MONTH(game.START_TIME) = MONTH(CURRENT_DATE - INTERVAL 1 MONTH)
  and game.END_TIME is not NULL
  and TIMESTAMPDIFF(MINUTE, game.START_TIME, game.END_TIME) > 5
  and game.INSTANCE_UUID = player.INSTANCE_UUID
  and game.GAME_ID = player.GAME_ID
  and player.USER_ID != -1
