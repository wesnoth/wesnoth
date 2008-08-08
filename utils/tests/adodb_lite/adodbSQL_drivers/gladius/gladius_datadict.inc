<?php
## Gladius Database Engine
# @author legolas558
# @version 0.3.94
# Licensed under GNU General Public License (GPL)
# contains parts of mysql_datadict.inc v4.65 by John Lim (jlim@natsoft.com.my)
#
#
# ADODB driver

// security - hide paths
if (!defined('ADODB_DIR')) die();

class ADODB2_gladius extends ADODB_DataDict {
	var $dbtype = 'gladius';
	var $alterCol = ' MODIFY COLUMN';	// not yet supported on Gladius
	var $alterTableAddIndex = true;
	var $dropTable = 'DROP TABLE %s';

	var $dropIndex = 'DROP INDEX %s ON %s';	// not yet supported on Gladius
	var $renameColumn = 'ALTER TABLE %s CHANGE COLUMN %s %s %s';	// needs column-definition!

 	function ActualType($meta)
	{
		switch($meta) {
		case 'C': return 'VARCHAR';
		case 'XL':
		case 'X': return 'VARCHAR(250)';
		
		case 'C2': return 'VARCHAR';
		case 'X2': return 'VARCHAR(250)';
		
		case 'B': return 'VARCHAR';
			
		case 'D': return 'DATE';
		case 'T': return 'DATE';
		
		case 'L': return 'DECIMAL(1)';
		case 'I': return 'DECIMAL(10)';
		case 'I1': return 'DECIMAL(3)';
		case 'I2': return 'DECIMAL(5)';
		case 'I4': return 'DECIMAL(10)';
		case 'I8': return 'DECIMAL(20)';
		
		case 'F': return 'DECIMAL(32,8)';
		case 'N': return 'DECIMAL';
		default:
			return $meta;
		}
	}

	function AlterColumnSQL($tabname, $flds)
	{
		if ($this->debug) $this->outp("AlterColumnSQL not supported");
		return array();
	}

	function DropColumnSQL($tabname, $flds)
	{
		if ($this->debug) $this->outp("DropColumnSQL not supported");
		return array();
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