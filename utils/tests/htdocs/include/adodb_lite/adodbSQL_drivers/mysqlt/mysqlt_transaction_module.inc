<?php
/**
 * ADOdb Lite Transaction Module for Mysqlt
 * 
 */

eval('class mysqlt_transaction_EXTENDER extends '. $last_module . '_ADOConnection { }');

class mysqlt_transaction_ADOConnection extends mysqlt_transaction_EXTENDER
{
}

eval('class mysqlt_transaction_resultset_EXTENDER extends '. $last_module . '_ResultSet { }');

class mysqlt_transaction_ResultSet extends mysqlt_transaction_resultset_EXTENDER
{
}
?>