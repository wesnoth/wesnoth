<?php
/**
 * ADOdb Lite Configuration File
 */

/**
* Set the $dbtype variable to the database designator.
* If this variable is enabled it will override the database designator
* entered in the ADONewConnection( $dbtype ) function. The database
* designator in a DSN string will be overridden but the rest of the DSN
* string will be used.
* 
* You can place a DSN entry in the $dbtype variable if you would like to
* auto connect to your database.
* 
* Example: 
* 
* $dbtype = "driver://username:password@hostname/database?options[=value]#modules";
* 
* driver = Databasetype Designator listed in the table at the start of this page. 
* username = The Username needed to access the database
* password = Optional password for accessing the database 
* hostname = localhost or url/ip address IE: http://0.0.0.0 or http://www.dbserver.com
* database = The name of the database you will be accessing
* options = All Drivers - 'persist', 'persistent', 'debug', 'fetchmode'
*                      Mysql (all) - 'port', 'clientflags'
*                      Mysqli - 'socket'
*                      Postgress (all) - 'port'
* modules = The modules that should be loaded. IE: pear, cache, extend, ect. 
* 
*/

// $dbtype = "mysql";

/**
* If you want to maintain compatability with the ADOdb ADONewConnection( $dbtype )
* function you should designate the modules you need loaded below. If you designate
* the modules below you do not need to designate them in
* ADONewConnection( $dbtype, $modules ).
* 
* If you would like more than one module loaded at the same time concatinate the 
* module names using a colon (:).
* 
* Example:
* $modules = "pear:transaction:extend";
* 
* The above example would load the Pear, Transaction and Extend modules
* automatically.
*/

// $modules = "pear";
?>