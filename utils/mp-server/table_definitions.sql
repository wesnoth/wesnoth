-- a minimal users table, if not using a phpbb3 installation
-- CREATE TABLE users
-- (
--     user_id int(10) unsigned NOT NULL AUTO_INCREMENT,
--     user_type tinyint(2) NOT NULL DEFAULT '0',
--     username varchar(255) COLLATE utf8_bin NOT NULL DEFAULT '',
--     user_password varchar(255) COLLATE utf8_bin NOT NULL DEFAULT '',
--     user_email varchar(100) COLLATE utf8_bin NOT NULL DEFAULT '',
--     PRIMARY KEY (user_id),
--     KEY user_type (user_type)
-- ) ENGINE=InnoDB AUTO_INCREMENT=50 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- table which the forum inserts bans into, which wesnothd checks during login
-- CREATE TABLE ban
-- (
--     ban_userid varchar(100) NOT NULL,
--     ban_end int(10) unsigned NOT NULL DEFAULT '0',
--     ban_ip varchar(100) DEFAULT NULL,
--     ban_email varchar(100) DEFAULT NULL,
--     ban_exclude int(10) unsigned NOT NULL DEFAULT '0',
--     PRIMARY KEY (ban_userid)
-- ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

------

-- extra information as necessary per user
-- user_lastvisit is used by the phpbb extension displaying the last time the user logged in to the MP server
-- user_is_moderator determines people who have the abilities granted to MP Moderators
CREATE TABLE extra
(
    username varchar(100) NOT NULL,
    user_lastvisit int(10) unsigned NOT NULL DEFAULT '0',
    user_is_moderator tinyint(4) NOT NULL DEFAULT '0',
    PRIMARY KEY (username)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- information about a single game
-- INSTANCE_UUID: retrieved from the UUID() function on wesnothd start up
-- GAME_ID: a sequential id wesnoth generates, resets on restart
-- INSTANCE_VERSION: the version of the server
-- GAME_NAME: the game's displayed title in the lobby
-- CREATE_TIME: when the game is made available in the lobby
-- START_TIME: when the players enter the game and begin playing
-- END_TIME: when the game ends, for any particular reason
-- MAP_NAME: the mp_scenario attribute value
-- ERA_NAME: the mp_era attribute value
-- REPLAY_NAME: the file name of the replay create when the game is ended
create table game_info
(
    INSTANCE_UUID      CHAR(36) NOT NULL,
    GAME_ID            INT UNSIGNED NOT NULL,
    INSTANCE_VERSION   VARCHAR(255) NOT NULL,
    GAME_NAME          VARCHAR(255) NOT NULL,
    CREATE_TIME        TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    START_TIME         TIMESTAMP NULL DEFAULT NULL,
    END_TIME           TIMESTAMP NULL DEFAULT NULL,
    MAP_NAME           VARCHAR(255),
    ERA_NAME           VARCHAR(255),
    REPLAY_NAME        VARCHAR(255),
    primary key (INSTANCE_UUID, GAME_ID)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- information about the players in a particular game present in game_info
-- this is accurate at the start of the game, but is not currently updated if a side changes owners, someone disconnects, etc
-- USER_ID: the ID of the player, taken from the USERS table
-- SIDE_NUMBER: the side controlled by USER_ID
-- IS_HOST: if USER_ID is the game's host
-- FACTION: the faction being played by this side
-- STATUS: the status of the side, currently only updated at game end
create table game_player_info
(
    INSTANCE_UUID CHAR(36) NOT NULL,
    GAME_ID       INT UNSIGNED NOT NULL,
    USER_ID       INT NOT NULL,
    SIDE_NUMBER   SMALLINT UNSIGNED NOT NULL,
    IS_HOST       VARCHAR(255) NOT NULL,
    FACTION       VARCHAR(255) NOT NULL,
    primary key (INSTANCE_UUID, GAME_ID, SIDE_NUMBER)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- information about any modifications that the game present in game_info has enabled
create table game_modification_info
(
    INSTANCE_UUID                  CHAR(36) NOT NULL,
    GAME_ID                        INT UNSIGNED NOT NULL,
    MODIFICATION_NAME              VARCHAR(255) NOT NULL,
    primary key (INSTANCE_UUID, GAME_ID, MODIFICATION_NAME)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
