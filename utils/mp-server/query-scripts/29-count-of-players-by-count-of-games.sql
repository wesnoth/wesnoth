select COUNT_OF_GAMES, count(*) as COUNT_OF_USERS
from
(
  select USER_ID, count(*) as COUNT_OF_GAMES
  from
  (
    select player.USER_ID
    from wesnothd_game_player_info player, wesnothd_game_info game
    where player.USER_ID != -1
      and player.INSTANCE_UUID = game.INSTANCE_UUID
      and player.GAME_ID = game.GAME_ID
      and game.END_TIME is not NULL
      and TIMESTAMPDIFF(MINUTE, game.START_TIME, game.END_TIME) > 5
    group by player.INSTANCE_UUID, player.GAME_ID
  ) i1
  group by USER_ID
) i2
group by COUNT_OF_GAMES
order by COUNT_OF_GAMES
