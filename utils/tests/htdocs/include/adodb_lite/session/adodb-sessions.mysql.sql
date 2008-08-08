-- $CVSHeader$

CREATE DATABASE /*! IF NOT EXISTS */ adodb_sessions;

DROP TABLE /*! IF EXISTS */ sessions;

CREATE TABLE /*! IF NOT EXISTS */ sessions (
	ID INT NOT NULL AUTO_INCREMENT PRIMARY KEY, 
	SessionID VARCHAR(64), 
	session_data TEXT DEFAULT '', 
	expiry INT(11),
	expireref	VARCHAR(250)	DEFAULT '',
	INDEX (SessionID),
	INDEX expiry (expiry)
);
