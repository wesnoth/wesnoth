<?php
/**
 * ADOdb Lite Example Module
 * 
 * This example module can be used to create your own extention to ADOdb Lite.
 * 
 * The module prefix name and class prefix name should all be the same and lowercase.
 * If the new module name is 'blob' then rename this file to 'blob_module.inc'. Rename
 * the following class designators.
 * 
 * example_generic_EXTENDER renamed to blob_EXTENDER
 * example_generic_ADOConnection renamed to blob_ADOConnection
 * example_generic_resultset_EXTENDER renamed to blob_resultset_EXTENDER
 * example_generic_ResultSet renamed to blob_ResultSet
 * 
 * Functions that interact with the database should be placed inside the
 * blob_ADOConnection class.  These are functions accessed using
 * $db->function().
 * 
 * Functions that interact with the resetset created by a query should
 * be placed inside the blob_ResultSet class.  These are functions accessed using
 * $result->function().
 * 
 * Example:
 * $result = $db->Execute($sql);
 * echo $result->function();
 * 
 * Place the file into the approriate database driver directory.  You should create one
 * for each database you would like to support.
 * 
 * To use your blob module start ADOdb Lite using.
 * 
 * $db = ADONewConnection($databasetype, 'blob');
 * 
 * Remember to replace every instance of mysql in the statements below with the database name 
 * the module is to be used.
 * 
 */

eval('class example_generic_EXTENDER extends '. $last_module . '_ADOConnection { }');

class example_generic_ADOConnection extends example_generic_EXTENDER
{
}

eval('class example_generic_resultset_EXTENDER extends '. $last_module . '_ResultSet { }');

class example_generic_ResultSet extends example_generic_resultset_EXTENDER
{
}
?>