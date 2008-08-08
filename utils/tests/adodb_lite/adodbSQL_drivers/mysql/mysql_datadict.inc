<?php
/**
  V4.65 22 July 2005  (c) 2000-2005 John Lim (jlim@natsoft.com.my). All rights reserved.
  Released under both BSD license and Lesser GPL library license. 
  Whenever there is any discrepancy between the two licenses, 
  the BSD license will take precedence.
	
  Set tabs to 4 for best viewing.
  
  Modified 28 August, 2005 for use with ADOdb Lite by Mark Dickenson
  
*/

// security - hide paths
if (!defined('ADODB_DIR')) die();

class ADODB2_mysql extends ADODB_DataDict {

	var $dbtype = 'mysql';
	var $alterCol = ' MODIFY COLUMN';
	var $alterTableAddIndex = true;
	var $dropTable = 'DROP TABLE IF EXISTS %s'; // requires mysql 3.22 or later
	
	var $dropIndex = 'DROP INDEX %s ON %s';
	var $renameColumn = 'ALTER TABLE %s CHANGE COLUMN %s %s %s';	// needs column-definition!

	function ActualType($meta)
	{
		switch(strtoupper($meta)) {
		case 'C': return 'VARCHAR';
		case 'XL':return 'LONGTEXT';
		case 'X': return 'TEXT';
		
		case 'C2': return 'VARCHAR';
		case 'X2': return 'LONGTEXT';
		
		case 'B': return 'LONGBLOB';
			
		case 'D': return 'DATE';
		case 'DT': return 'DATETIME';
		case 'T': return 'TIME';
		case 'TS': return 'TIMESTAMP';
		case 'L': return 'TINYINT';
		
		case 'R':
		case 'I4':
		case 'I': return 'INTEGER';
		case 'I1': return 'TINYINT';
		case 'I2': return 'SMALLINT';
		case 'I8': return 'BIGINT';
		
		case 'F': return 'DOUBLE';
		case 'N': return 'NUMERIC';
		default:
			return $meta;
		}
	}

	// return string must begin with space
	function _CreateSuffix($fname,$ftype,$fnotnull,$fdefault,$fautoinc,$fconstraint,$funsigned)
	{	
		$suffix = '';
		if ($funsigned) $suffix .= ' UNSIGNED';
		if ($fnotnull) $suffix .= ' NOT NULL';
		if (strlen($fdefault)) $suffix .= " DEFAULT $fdefault";
		if ($fautoinc) $suffix .= ' AUTO_INCREMENT';
		if ($fconstraint) $suffix .= ' '.$fconstraint;
		return $suffix;
	}

	function _IndexSQL($idxname, $tabname, $flds, $idxoptions)
	{
		$sql = array();
		
		if ( isset($idxoptions['REPLACE']) || isset($idxoptions['DROP']) ) {
			if ($this->alterTableAddIndex) $sql[] = "ALTER TABLE $tabname DROP INDEX $idxname";
			else $sql[] = sprintf($this->dropIndex, $idxname, $tabname);

			if ( isset($idxoptions['DROP']) )
				return $sql;
		}
		
		if ( empty ($flds) ) {
			return $sql;
		}
		
		if (isset($idxoptions['FULLTEXT'])) {
			$unique = ' FULLTEXT';
		} elseif (isset($idxoptions['UNIQUE'])) {
			$unique = ' UNIQUE';
		} else {
			$unique = '';
		}
		
		if ( is_array($flds) ) $flds = implode(', ',$flds);
		
		if ($this->alterTableAddIndex) $s = "ALTER TABLE $tabname ADD $unique INDEX $idxname ";
		else $s = 'CREATE' . $unique . ' INDEX ' . $idxname . ' ON ' . $tabname;
		
		$s .= ' (' . $flds . ')';
		
		if ( isset($idxoptions[$this->upperName]) )
			$s .= $idxoptions[$this->upperName];
		
		$sql[] = $s;
		
		return $sql;
	}

}

?>