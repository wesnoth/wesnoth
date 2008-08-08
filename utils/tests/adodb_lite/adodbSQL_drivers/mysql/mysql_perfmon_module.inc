<?php
/* 
V4.65 22 July 2005  (c) 2000-2005 John Lim (jlim@natsoft.com.my). All rights reserved.
  Released under both BSD license and Lesser GPL library license. 
  Whenever there is any discrepancy between the two licenses, 
  the BSD license will take precedence. See License.txt. 
  Set tabs to 4 for best viewing.
  
  Library for basic performance monitoring and tuning.
  
  Modified 17 March 2006 for use with ADOdb Lite by Pádraic Brady
  Such modifications as listed (c) 2006 Pádraic Brady (maugrimtr@hotmail.com)
  
  Modifications:
  	- Moved adodb_perf class to $dbtype_perfmon_module.inc as the $dbtype_perfmon_ADOConnection class
  	- restricted adodb_perf class to tracking table name to use (for BC) - it's an alias for perfmon_parent_ADOConnection::table()
  	- Added LogSQL() method, and _logsql property
  	- general formatting changes and replacement of htmlspecialchars() woth htmlentities()
  	- parent class for specific driver modules added in adodb-perf-module.inc.php
  
*/

// required to allow a common parent for all perfmon dbtype specific modules to extend
$thisDBType = 'mysql';

require_once(ADODB_DIR . '/adodb-perf.inc.php');
require_once(ADODB_DIR . '/adodb-perf-module.inc.php');

// not really needed, we already know the parent class name?
eval('class mysql_perfmon_EXTENDER extends perfmon_parent_ADOConnection { }');

class mysql_perfmon_ADOConnection extends mysql_perfmon_EXTENDER
{	
	var $upperCase = 'upper';
	var $substr = "substring";
	
	var $tablesSQL = 'show table status';
	
	var $createTableSQL = "CREATE TABLE adodb_logsql (
		  created datetime NOT NULL,
		  sql0 varchar(250) NOT NULL,
		  sql1 text NOT NULL,
		  params text NOT NULL,
		  tracer text NOT NULL,
		  timer decimal(16,6) NOT NULL
		)";
		
	var $settings = array(
	'Ratios',
		'MyISAM cache hit ratio' => array('RATIO',
			'=GetKeyHitRatio',
			'=WarnCacheRatio'),
		'InnoDB cache hit ratio' => array('RATIO',
			'=GetInnoDBHitRatio',
			'=WarnCacheRatio'),
		'data cache hit ratio' => array('HIDE', # only if called
			'=FindDBHitRatio',
			'=WarnCacheRatio'),
		'sql cache hit ratio' => array('RATIO',
			'=GetQHitRatio',
			''),
	'IO',
		'data reads' => array('IO',
			'=GetReads',
			'Number of selects (Key_reads is not accurate)'),
		'data writes' => array('IO',
			'=GetWrites',
			'Number of inserts/updates/deletes * coef (Key_writes is not accurate)'),
		
	'Data Cache',
		'MyISAM data cache size' => array('DATAC',
			array("show variables", 'key_buffer_size'),
			'' ),
		'BDB data cache size' => array('DATAC',
			array("show variables", 'bdb_cache_size'),
			'' ),
		'InnoDB data cache size' => array('DATAC',
			array("show variables", 'innodb_buffer_pool_size'),
			'' ),
	'Memory Usage',
		'read buffer size' => array('CACHE',
			array("show variables", 'read_buffer_size'),
			'(per session)'),
		'sort buffer size' => array('CACHE',
			array("show variables", 'sort_buffer_size'),
			'Size of sort buffer (per session)' ),
		'table cache' => array('CACHE',
			array("show variables", 'table_cache'),
			'Number of tables to keep open'),
	'Connections',	
		'current connections' => array('SESS',
			array('show status','Threads_connected'),
			''),
		'max connections' => array( 'SESS',
			array("show variables",'max_connections'),
			''),
	
		false
	);
	
	/**
		Get server version info...
		
		@returns An array with 2 elements: $arr['description'] is the description string, 
			and $arr['version'] is the version (also a string).
	*/
	function ServerInfo()
	{
		$arr = array();
		$result = $this->do_query('select version()', -1, -1, false);
		if (!$result->EOF) $data = $result->fields;
				else $data = array();
		$arr['version'] = $arr['description'] = $data[0];
		//****// Where does $arr come from in ADOdb - doesn't appear global, maybe null? A possible bug?
		return $arr;
	}

	/* 
		Explain Plan for $sql.
		If only a snippet of the $sql is passed in, then $partial will hold the crc32 of the 
			actual sql.
	*/
	function Explain($sql,$partial=false)
	{
		$perf_table = perfmon_parent_ADOConnection::table(); // this was missing in the original ADOdb perf class - bug?
		
		if (strtoupper(substr(trim($sql),0,6)) !== 'SELECT') return '<p>Unable to EXPLAIN a non-select statement</p>';
		$save = $this->LogSQL(false);
		if ($partial) {
			$sqlq = $this->qstr($sql.'%');
			$arr = $this->GetArray("select distinct sql1 from $perf_table where sql1 like $sqlq");
			if ($arr)
			{
				foreach($arr as $row)
				{
					$sql = reset($row);
					if (crc32($sql) == $partial) break;
				}
			}
		}
		$sql = str_replace('?',"''",$sql);
		
		if ($partial)
		{
			$sqlq = $this->qstr($sql.'%');
			$sql = $this->GetOne("select sql1 from $perf_table where sql1 like $sqlq");
		}
		
		$s = '<p><b>Explain</b>: '.htmlentities($sql, ENT_QUOTES, 'UTF-8').'</p>';
		$rs = $this->Execute('EXPLAIN '.$sql);
		$s .= rs2html($rs,false,false,false,false);
		$this->LogSQL($save);
		$s .= $this->Tracer($sql);
		return $s;
	}
	
	function Tables()
	{
		if (!$this->tablesSQL) return false;
		
		$rs = $this->Execute($this->tablesSQL);
		if (!$rs) return false;
		
		$html = rs2html($rs,false,false,false,false);
		return $html;
	}
	

	function CreateLogTable()
	{
		if (!$this->createTableSQL) return false;
		
		$savelog = $this->LogSQL(false);
		$ok = $this->Execute($this->createTableSQL);
		$this->LogSQL($savelog);
		return ($ok) ? true : false;
	}
	
	
	function GetReads()
	{
		global $ADODB_FETCH_MODE;
		$save = $ADODB_FETCH_MODE;
		$ADODB_FETCH_MODE = ADODB_FETCH_NUM;
		
		$rs = $this->Execute('show status');
		
		$ADODB_FETCH_MODE = $save;
		
		if (!$rs) return 0;
		$val = 0;
		while (!$rs->EOF)
		{
			switch($rs->fields[0])
			{
				case 'Com_select': 
				$val = $rs->fields[1];
				$rs->Close();
				return $val;
			}
			$rs->MoveNext();
		} 
		
		$rs->Close();
		return $val;
	}
	
	function GetWrites()
	{
		global $ADODB_FETCH_MODE;
		$save = $ADODB_FETCH_MODE;
		$ADODB_FETCH_MODE = ADODB_FETCH_NUM;
		
		$rs = $this->Execute('show status');
		
		$ADODB_FETCH_MODE = $save;
		
		if (!$rs) return 0;
		$val = 0.0;
		while (!$rs->EOF)
		{
			switch($rs->fields[0])
			{
				case 'Com_insert': 
					$val += $rs->fields[1];
					break;
				case 'Com_delete': 
					$val += $rs->fields[1];
					break;
				case 'Com_update': 
					$val += $rs->fields[1]/2;
					$rs->Close();
					return $val;
			}
			$rs->MoveNext();
		} 
		
		$rs->Close();
		return $val;
	}
	
	function FindDBHitRatio()
	{	
		global $ADODB_FETCH_MODE;
		$save = $ADODB_FETCH_MODE;
		$ADODB_FETCH_MODE = ADODB_FETCH_NUM;
		
		$rs = $this->Execute('show table status');

		$ADODB_FETCH_MODE = $save;
		
		if (!$rs) return '';
		$type = strtoupper($rs->fields[1]);
		$rs->Close();
		switch($type)
		{
			case 'MYISAM':
			case 'ISAM':
				return $this->DBParameter('MyISAM cache hit ratio').' (MyISAM)';
			case 'INNODB':
				return $this->DBParameter('InnoDB cache hit ratio').' (InnoDB)';
			default:
				return $type.' not supported';
		}	
	}
	
	function GetQHitRatio()
	{
		//Total number of queries = Qcache_inserts + Qcache_hits + Qcache_not_cached
		$hits = $this->_DBParameter(array("show status","Qcache_hits"));
		$total = $this->_DBParameter(array("show status","Qcache_inserts"));
		$total += $this->_DBParameter(array("show status","Qcache_not_cached"));
		
		$total += $hits;
		if ($total) return round(($hits*100)/$total,2);
		return 0;
	}
	
	/*
		Use session variable to store Hit percentage, because MySQL
		does not remember last value of SHOW INNODB STATUS hit ratio
		
		# 1st query to SHOW INNODB STATUS
		0.00 reads/s, 0.00 creates/s, 0.00 writes/s
		Buffer pool hit rate 1000 / 1000
		
		# 2nd query to SHOW INNODB STATUS
		0.00 reads/s, 0.00 creates/s, 0.00 writes/s
		No buffer pool activity since the last printout
	*/
	function GetInnoDBHitRatio()
	{
		global $ADODB_FETCH_MODE;
	
		$save = $ADODB_FETCH_MODE;
		$ADODB_FETCH_MODE = ADODB_FETCH_NUM;

		$rs = $this->Execute('show innodb status');
;
		$ADODB_FETCH_MODE = $save;
		
		if (!$rs || $rs->EOF) return 0;
		$stat = $rs->fields[0];
		$rs->Close();
		$at = strpos($stat,'Buffer pool hit rate');
		$stat = substr($stat,$at,200);
		if (preg_match('!Buffer pool hit rate\s*([0-9]*) / ([0-9]*)!',$stat,$arr))
		{
			$val = 100*$arr[1]/$arr[2];
			$_SESSION['INNODB_HIT_PCT'] = $val;
			return round($val,2);
		}
		else
		{
			if (isset($_SESSION['INNODB_HIT_PCT'])) return $_SESSION['INNODB_HIT_PCT'];
			return 0;
		}
		return 0;
	}
	
	function GetKeyHitRatio()
	{
		$hits = $this->_DBParameter(array("show status","Key_read_requests"));
		$reqs = $this->_DBParameter(array("show status","Key_reads"));
		if ($reqs == 0) return 0;
		
		return round(($hits/($reqs+$hits))*100,2);
	}
	
    // start hack 
    var $optimizeTableLow = 'CHECK TABLE %s FAST QUICK';
    var $optimizeTableHigh = 'OPTIMIZE TABLE %s';
    
    /** 
     * @see adodb_perf#optimizeTable
     */
     function optimizeTable( $table, $mode = ADODB_OPT_LOW) 
     {
        if ( !is_string( $table)) return false;
        
        $sql = '';
        switch( $mode)
        {
            case ADODB_OPT_LOW:
            	$sql = $this->optimizeTableLow;
            	break;
            case ADODB_OPT_HIGH:
            	$sql = $this->optimizeTableHigh;
            	break;
            default: 
            {
                // May dont use __FUNCTION__ constant for BC (__FUNCTION__ Added in PHP 4.3.0)
                trigger_error(sprintf( "<p>%s: '%s' using of undefined mode '%s'</p>", __CLASS__, __FUNCTION__, $mode), E_USER_ERROR);
                return false;
            }
        }
        $sql = sprintf( $sql, $table);
        
        return $this->Execute( $sql) !== false;
     }

}



eval('class mysql_perfmon_resultset_EXTENDER extends '. $last_module . '_ResultSet { }');

class mysql_perfmon_ResultSet extends mysql_perfmon_resultset_EXTENDER
{

	function MetaType($t,$len=-1,$fieldobj=false)
	{
		if (is_object($t)) {
			$fieldobj = $t;
			$t = $fieldobj->type;
			$len = $fieldobj->max_length;
		}
		
		$len = -1; // mysql max_length is not accurate
		switch (strtoupper($t)) {
		case 'STRING': 
		case 'CHAR':
		case 'VARCHAR': 
		case 'TINYBLOB': 
		case 'TINYTEXT': 
		case 'ENUM': 
		case 'SET': 
			if ($len <= $this->blobSize) return 'C';
			
		case 'TEXT':
		case 'LONGTEXT': 
		case 'MEDIUMTEXT':
			return 'X';
			
		// php_mysql extension always returns 'blob' even if 'text'
		// so we have to check whether binary...
		case 'IMAGE':
		case 'LONGBLOB': 
		case 'BLOB':
		case 'MEDIUMBLOB':
			return !empty($fieldobj->binary) ? 'B' : 'X';
			
		case 'YEAR':
		case 'DATE': return 'D';
		
		case 'TIME':
		case 'DATETIME':
		case 'TIMESTAMP': return 'T';
		
		case 'INT': 
		case 'INTEGER':
		case 'BIGINT':
		case 'TINYINT':
		case 'MEDIUMINT':
		case 'SMALLINT': 
			
			if (!empty($fieldobj->primary_key)) return 'R';
			else return 'I';
		
		default: return 'N';
		}
	}

}

?>
