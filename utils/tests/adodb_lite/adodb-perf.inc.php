<?php
/* 
V4.65 22 July 2005  (c) 2000-2005 John Lim (jlim@natsoft.com.my). All rights reserved.
  Released under both BSD license and Lesser GPL library license. 
  Whenever there is any discrepancy between the two licenses, 
  the BSD license will take precedence. See License.txt. 
  Set tabs to 4 for best viewing.
  
  Latest version is available at http://adodb.sourceforge.net
  
  Library for basic performance monitoring and tuning.
  
  My apologies if you see code mixed with presentation. The presentation suits
  my needs. If you want to separate code from presentation, be my guest. Patches
  are welcome.
  
  Modified 17 March 2006 for use with ADOdb Lite by Pádraic Brady
  Such modifications (c) 2006 Pádraic Brady (maugrimtr@hotmail.com)
  
*/

if (!defined(ADODB_DIR)) include_once(dirname(__FILE__).'/adodb.inc.php');
include_once(ADODB_DIR . '/tohtml.inc.php');

define( 'ADODB_OPT_HIGH', 2);
define( 'ADODB_OPT_LOW', 1);

// returns in K the memory of current process, or 0 if not known
function adodb_getmem()
{
	if (function_exists('memory_get_usage'))
		return (integer) ((memory_get_usage()+512)/1024);
	
	$pid = getmypid();
	
	if ( strncmp(strtoupper(PHP_OS),'WIN',3)==0) {
		$output = array();
	
		exec('tasklist /FI "PID eq ' . $pid. '" /FO LIST', $output); 
		return substr($output[5], strpos($output[5], ':') + 1);
	} 
	
	/* Hopefully UNIX */
	exec("ps --pid $pid --no-headers -o%mem,size", $output);
	if (sizeof($output) == 0) return 0;
	
	$memarr = explode(' ',$output[0]);
	if (sizeof($memarr)>=2) return (integer) $memarr[1];
	
	return 0;
}

// avoids localization problems where , is used instead of .
function adodb_round($n,$prec)
{
	return number_format($n, $prec, '.', '');
}

/* return microtime value as a float */
function adodb_microtime()
{
	$t = microtime();
	$t = explode(' ',$t);
	return (float)$t[1]+ (float)$t[0];
}

/* sql code timing */
function& adodb_log_sql(&$conn,$sql,$inputarr)
{
	
    $perf_table = adodb_perf::table();
    $conn->_logsql = false; // replaces setting ::fnExecute=false in ADOdb
	$t0 = microtime();
	$rs =& $conn->Execute($sql,$inputarr);
	$t1 = microtime();
	$conn->_logsql = true; // reverse setting ::_logsql=false

	if (!empty($conn->_logsql)) {
		$conn->_logsql = false; // disable logsql error simulation
		$dbT = $conn->dbtype;
		
		$a0 = split(' ',$t0);
		$a0 = (float)$a0[1]+(float)$a0[0];
		
		$a1 = split(' ',$t1);
		$a1 = (float)$a1[1]+(float)$a1[0];
		
		$time = $a1 - $a0;
	
		if (!$rs) {
			$errM = $conn->ErrorMsg();
			$errN = $conn->ErrorNo();
			$tracer = substr('ERROR: '.htmlspecialchars($errM),0,250);
		} else {
			$tracer = '';
			$errM = '';
			$errN = 0;
		}
		if(isset($_SERVER['HTTP_HOST']))
		{
			$tracer .= '<br>'.$_SERVER['HTTP_HOST'];
			if(isset($_SERVER['PHP_SELF']))
			{
				$tracer .= $_SERVER['PHP_SELF'];
			}
		}
		elseif(isset($_SERVER['PHP_SELF']))
		{
			$tracer .= '<br>'.$_SERVER['PHP_SELF'];
		}
		
		$tracer = (string) substr($tracer,0,500);
		
		if(is_array($inputarr))
		{
			if(is_array(reset($inputarr)))
			{
				$params = 'Array sizeof=' . sizeof($inputarr);
			}
			else
			{
				// Quote string parameters so we can see them in the
				// performance stats. This helps spot disabled indexes.
				$xar_params = $inputarr;
				foreach($xar_params as $xar_param_key => $xar_param)
				{
					if (gettype($xar_param) == 'string')
					$xar_params[$xar_param_key] = '"' . $xar_param . '"';
				}
				$params = implode(', ', $xar_params);
				if (strlen($params) >= 3000) $params = substr($params, 0, 3000);
			}
		}
		else
		{
			$params = '';
		}
		
		if (is_array($sql)) $sql = $sql[0];
		$arr = array('b'=>strlen($sql).'.'.crc32($sql),
					'c'=>substr($sql,0,3900), 'd'=>$params,'e'=>$tracer,'f'=>adodb_round($time,6));

		$saved = $conn->debug;
		$conn->debug = 0;
		
		$d = $conn->sysTimeStamp;
		if (empty($d))
		{
			$d = date("'Y-m-d H:i:s'");
		}
		
		/*
		// OCI/Informix/ODBC_MSSQL - not sure if/how available in adodb-lite so I've commented out the section for now - (Pádraic)
		*/
		
		/*if ($dbT == 'oci8' && $dbT != 'oci8po')
		{
			$isql = "insert into $perf_table values($d,:b,:c,:d,:e,:f)";
		}
		elseif($dbT == 'odbc_mssql' || $dbT == 'informix')
		{
			$timer = $arr['f'];
			if ($dbT == 'informix') $sql2 = substr($sql2,0,230);

			$sql1 = $conn->qstr($arr['b']);
			$sql2 = $conn->qstr($arr['c']);
			$params = $conn->qstr($arr['d']);
			$tracer = $conn->qstr($arr['e']);
			
			$isql = "insert into $perf_table (created,sql0,sql1,params,tracer,timer) values($d,$sql1,$sql2,$params,$tracer,$timer)";
			if ($dbT == 'informix') $isql = str_replace(chr(10),' ',$isql);
			$arr = false;
		} else {*/
			$isql = "insert into $perf_table (created,sql0,sql1,params,tracer,timer) values( $d,?,?,?,?,?)";
		//}

		$ok = $conn->Execute($isql,$arr);
		$conn->debug = $saved;
		
		if ($ok) {
			$conn->_logsql = true; 
		} else {
			$err2 = $conn->ErrorMsg();
			$conn->_logsql = true; // enable logsql error simulation
			$perf =& NewPerfMonitor($conn);
			if ($perf) {
				if ($perf->CreateLogTable()) $ok = $conn->Execute($isql,$arr);
			} else {
				$ok = $conn->Execute("create table $perf_table (
				created varchar(50),
				sql0 varchar(250), 
				sql1 varchar(4000),
				params varchar(3000),
				tracer varchar(500),
				timer decimal(16,6))");
			}
			/*if (!$ok) {
				ADOConnection::outp( "<p><b>LOGSQL Insert Failed</b>: $isql<br>$err2</p>");
				$conn->_logsql = false;
			}*/
		}
		$conn->_errorMsg = $errM;
		$conn->_errorCode = $errN;
	}
	return $rs;
}

	
/*
The settings data structure is an associative array that database parameter per element.

Each database parameter element in the array is itself an array consisting of:

0: category code, used to group related db parameters
1: either
	a. sql string to retrieve value, eg. "select value from v\$parameter where name='db_block_size'", 
	b. array holding sql string and field to look for, e.g. array('show variables','table_cache'),
	c. a string prefixed by =, then a PHP method of the class is invoked, 
		e.g. to invoke $this->GetIndexValue(), set this array element to '=GetIndexValue',
2: description of the database parameter
*/

class adodb_perf {

	function adodb_perf() {}

    /**
	 * Alias for perfmon_parent_ADOConnection::table()
	 * 
	 * @access public 
	 * @param string $newtable The name for the table to use; optional.
	 * @return string
	 */ 
    function table($newtable = false) {
    	$rt = perfmon_parent_ADOConnection::table($newtable);
    	return $rt;
    }
    
 }

?>
