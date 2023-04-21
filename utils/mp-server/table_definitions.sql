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

-- a minimal topics table, if not using a phpbb3 installation
-- create table topics
-- (
--     TOPIC_ID MEDIUMINT(8) UNSIGNED NOT NULL AUTO_INCREMENT,
--     PRIMARY KEY (TOPIC_ID)
-- ) ENGINE=InnoDB;

-- table which the forum inserts bans into, which wesnothd checks during login
-- create table ban
-- (
--     BAN_USERID  INT(10) UNSIGNED NOT NULL,
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
-- REPLAY_NAME: the file name of the replay create when the game is ended
-- OOS: Y/N flag of whether the game encountered an OOS error
-- RELOAD: Y/N flag for whether this is a new game or a reload of a previous game
-- OBSERVERS: Y/N flag for whether the game allows observers
-- PASSWORD: Y/N flag for whether the game had a password set
-- PUBLIC: Y/N flag for whether the game will have a publicly accesible replay created for it
create table game_info
(
    INSTANCE_UUID    CHAR(36) NOT NULL,
    GAME_ID          INT UNSIGNED NOT NULL,
    INSTANCE_VERSION VARCHAR(255) NOT NULL,
    GAME_NAME        VARCHAR(255) NOT NULL,
    START_TIME       TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    END_TIME         TIMESTAMP NULL DEFAULT NULL,
    REPLAY_NAME      VARCHAR(255),
    OOS              BIT(1) NOT NULL DEFAULT 0,
    RELOAD           BIT(1) NOT NULL,
    OBSERVERS        BIT(1) NOT NULL,
    PASSWORD         BIT(1) NOT NULL,
    PUBLIC           BIT(1) NOT NULL,
    PRIMARY KEY (INSTANCE_UUID, GAME_ID)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE INDEX START_TIME_IDX ON game_info(START_TIME);

-- information about the players in a particular game present in game_info
-- this is accurate at the start of the game, but is not currently updated if a side changes owners, someone disconnects, etc
-- USER_ID: the ID of the player, taken from the USERS table
-- SIDE_NUMBER: the side controlled by USER_ID
-- IS_HOST: if USER_ID is the game's host
-- FACTION: the faction being played by this side
-- CLIENT_VERSION: the version of the wesnoth client used to connect
-- CLIENT_SOURCE: where the wesnoth client was downloaded from - SourceForge, Steam, etc
-- USER_NAME: the username logged in with
-- LEADERS: the leader(s) for the side. if multiple leaders are found, then they are comma delimited.
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
    LEADERS        VARCHAR(255) NOT NULL DEFAULT '',
    PRIMARY KEY (INSTANCE_UUID, GAME_ID, SIDE_NUMBER)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE INDEX USER_ID_IDX ON game_player_info(USER_ID);

-- information about the scenario/era/modifications for the game
-- TYPE: one of era/scenario/modification/campaign
-- ID: the id of the content
-- NAME: the content's user-visible name
-- ADDON_ID: the id of the add-on that the particular content came from
-- ADDON_VERSION: the version of the add-on
create table game_content_info
(
    INSTANCE_UUID     CHAR(36) NOT NULL,
    GAME_ID           INT UNSIGNED NOT NULL,
    TYPE              VARCHAR(100) NOT NULL,
    ID                VARCHAR(100) NOT NULL,
    NAME              VARCHAR(255),
    ADDON_ID          VARCHAR(100) NOT NULL,
    ADDON_VERSION     VARCHAR(255) NOT NULL,
    PRIMARY KEY (INSTANCE_UUID, GAME_ID, TYPE, ID, ADDON_ID)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- information about an uploaded addon
-- INSTANCE_VERSION: the version of the addons server instance
-- ADDON_ID: the ID of the addon (folder name)
-- ADDON_NAME: the name of the addon
-- TYPE: the type of the addon
-- VERSION: the version of the addon
-- FORUM_AUTH: whether forum authentication is to be used when uploading
-- UPLOADED_ON: when the addon was uploaded
-- FEEDBACK_TOPIC: the forum topic ID where feedback for the addon can be posted, 0 if not set
-- DOWNLOAD_COUNT: the number of times the add-on has been downloaded by players (does not count downloads from https://addons.wesnoth.org)
-- UPLOADER: the author attribute or the chosen secondary_author from the _server.pbl
create table addon_info
(
    INSTANCE_VERSION VARCHAR(255) NOT NULL,
    ADDON_ID         VARCHAR(255) NOT NULL,
    ADDON_NAME       VARCHAR(255) NOT NULL,
    TYPE             VARCHAR(255) NOT NULL,
    VERSION          VARCHAR(255) NOT NULL,
    FORUM_AUTH       BIT(1) NOT NULL,
    UPLOADED_ON      TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FEEDBACK_TOPIC   INT UNSIGNED NOT NULL,
    DOWNLOAD_COUNT   INT UNSIGNED NOT NULL DEFAULT 0,
    UPLOADER         VARCHAR(255),
    PRIMARY KEY (INSTANCE_VERSION, ADDON_ID, VERSION)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- listing of the primary author and any secondary authors when forum_auth is used
-- INSTANCE_VERSION: the version of the addons server instance
-- ADDON_ID: the ID of the addon (folder name)
-- AUTHOR: the author attribute or the chosen secondary_author from the _server.pbl
-- IS_PRIMARY: whether this is the primary author or not
create table addon_authors
(
    INSTANCE_VERSION VARCHAR(255) NOT NULL,
    ADDON_ID         VARCHAR(255) NOT NULL,
    AUTHOR           VARCHAR(255) NOT NULL,
    IS_PRIMARY       BIT(1) NOT NULL,
    PRIMARY KEY (INSTANCE_VERSION, ADDON_ID, AUTHOR)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- history of user sessions
-- LOGIN_ID: auto generated ID to use as a primary key
-- USER_NAME: the username logged in with
-- IP: the IP address the login originated from
-- LOGIN_TIME: when the user logged in
-- LOGOUT_TIME: when the user logged out
-- VERSION: the version the user logged in with
create table connection_history
(
    LOGIN_ID    BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    USER_NAME   VARCHAR(255) NOT NULL,
    IP          VARCHAR(255) NOT NULL,
    LOGIN_TIME  TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    LOGOUT_TIME TIMESTAMP NULL DEFAULT NULL,
    VERSION     VARCHAR(255),
    PRIMARY KEY (LOGIN_ID)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE INDEX CONNECTION_IP_IDX ON connection_history(IP);
CREATE INDEX CONNECTION_USERNAME_IDX ON connection_history(USER_NAME);
