-- a minimal users table, if not using a phpbb3 installation
-- create table users
-- (
--     USER_ID       INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
--     USER_TYPE     TINYINT(2) NOT NULL DEFAULT 0,
--     USERNAME      VARCHAR(255) COLLATE utf8_bin NOT NULL DEFAULT '',
--     USER_PASSWORD VARCHAR(255) COLLATE utf8_bin NOT NULL DEFAULT '',
--     USER_EMAIL    VARCHAR(100) COLLATE utf8_bin NOT NULL DEFAULT '',
--     PRIMARY KEY (USER_ID),
--     KEY USER_TYPE (USER_TYPE)
-- ) ENGINE=InnoDB AUTO_INCREMENT=50 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- a minimal groups table, if not using a phpbb3 installation
-- create table user_groups
-- (
--     GROUP_ID MEDIUMINT(8) UNSIGNED NOT NULL,
--     USER_ID  MEDIUMINT(8) UNSIGNED NOT NULL,
--     PRIMARY KEY (USER_ID, GROUP_ID)
-- ) ENGINE=InnoDB;

-- table which the forum inserts bans into, which wesnothd checks during login
-- create table ban
-- (
--     BAN_USERID  VARCHAR(100) NOT NULL,
--     BAN_END     INT(10) UNSIGNED NOT NULL DEFAULT 0,
--     BAN_IP      VARCHAR(100) DEFAULT NULL,
--     BAN_EMAIL   VARCHAR(100) DEFAULT NULL,
--     BAN_EXCLUDE INT(10) UNSIGNED NOT NULL DEFAULT 0,
--     PRIMARY KEY (BAN_USERID)
-- ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

------

-- extra information as necessary per user
-- USERNAME: who this information is about
-- USER_LASTVISIT: used by the phpbb extension displaying the last time the user logged in to the MP server
-- USER_IS_MODERATOR: determines people who have the abilities granted to MP Moderators
create table extra
(
    USERNAME          VARCHAR(100) NOT NULL,
    USER_LASTVISIT    INT(10) UNSIGNED NOT NULL DEFAULT 0,
    USER_IS_MODERATOR TINYINT(1) NOT NULL DEFAULT 0,
    PRIMARY KEY (USERNAME)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- information about a single game
-- INSTANCE_UUID: retrieved from the UUID() function on wesnothd start up
-- GAME_ID: a sequential id wesnoth generates, resets on restart
-- INSTANCE_VERSION: the version of the server
-- GAME_NAME: the game's displayed title in the lobby
-- START_TIME: when the players enter the game and begin playing
-- END_TIME: when the game ends, for any particular reason
-- MAP_NAME: the mp_scenario attribute value
-- ERA_NAME: the mp_era attribute value
-- REPLAY_NAME: the file name of the replay create when the game is ended
-- OOS: Y/N flag of whether the game encountered an OOS error
create table game_info
(
    INSTANCE_UUID    CHAR(36) NOT NULL,
    GAME_ID          INT UNSIGNED NOT NULL,
    INSTANCE_VERSION VARCHAR(255) NOT NULL,
    GAME_NAME        VARCHAR(255) NOT NULL,
    START_TIME       TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    END_TIME         TIMESTAMP NULL DEFAULT NULL,
    MAP_NAME         VARCHAR(255) NOT NULL,
    ERA_NAME         VARCHAR(255) NOT NULL,
    REPLAY_NAME      VARCHAR(255),
    OOS              BIT(1) NOT NULL DEFAULT 0,
    RELOAD           BIT(1) NOT NULL,
    OBSERVERS        BIT(1) NOT NULL,
    PASSWORD         BIT(1) NOT NULL,
    PUBLIC           BIT(1) NOT NULL,
    PRIMARY KEY (INSTANCE_UUID, GAME_ID)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- information about the players in a particular game present in game_info
-- this is accurate at the start of the game, but is not currently updated if a side changes owners, someone disconnects, etc
-- USER_ID: the ID of the player, taken from the USERS table
-- SIDE_NUMBER: the side controlled by USER_ID
-- IS_HOST: if USER_ID is the game's host
-- FACTION: the faction being played by this side
-- STATUS: the status of the side, currently only updated at game end
-- CLIENT_VERSION: the version of the wesnoth client used to connect
-- CLIENT_SOURCE: where the wesnoth client was downloaded from - SourceForge, Steam, etc
create table game_player_info
(
    INSTANCE_UUID  CHAR(36) NOT NULL,
    GAME_ID        INT UNSIGNED NOT NULL,
    USER_ID        INT NOT NULL,
    SIDE_NUMBER    SMALLINT UNSIGNED NOT NULL,
    IS_HOST        BIT(1) NOT NULL,
    FACTION        VARCHAR(255) NOT NULL,
    CLIENT_VERSION VARCHAR(255) NOT NULL DEFAULT '',
    CLIENT_SOURCE  VARCHAR(255) NOT NULL DEFAULT '',
    USER_NAME      VARCHAR(255) NOT NULL DEFAULT '',
    PRIMARY KEY (INSTANCE_UUID, GAME_ID, SIDE_NUMBER)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- information about any modifications that the game present in game_info has enabled
create table game_modification_info
(
    INSTANCE_UUID     CHAR(36) NOT NULL,
    GAME_ID           INT UNSIGNED NOT NULL,
    MODIFICATION_NAME VARCHAR(255) NOT NULL,
    PRIMARY KEY (INSTANCE_UUID, GAME_ID, MODIFICATION_NAME)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
