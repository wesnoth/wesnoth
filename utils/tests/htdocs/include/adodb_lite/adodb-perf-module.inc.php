<?php
/* 
V4.65 22 July 2005  (c) 2000-2005 John Lim (jlim@natsoft.com.my). All rights reserved.
  Released under both BSD license and Lesser GPL library license. 
  Whenever there is any discrepancy between the two licenses, 
  the BSD license will take precedence. See License.txt.
  
  Library for basic performance monitoring and tuning.
  
  Modified 23 April 2006 for use with ADOdb Lite by Pádraic Brady
  Such modifications as listed (c) 2006 Pádraic Brady (maugrimtr@hotmail.com)
  
  Modifications:
  	- Moved below methods from adodb_perf class to create a common parent from which all driver
  	specific perfmon modules will extend to prevent duplicate code.
  	- See specific driver module files for other changes
  
*/

eval('class perfmon_parent_EXTENDER extends ' . $last_module . '_ADOConnection { }');

class perfmon_parent_ADOConnection extends perfmon_parent_EXTENDER
{

	var $color = '#F0F0F0';
	var $table = '<table style="border: 2px groove #000000; background-color: #FFFFFF;">';
	var $titles = '<tr><td><strong>Parameter</strong></td><td><strong>Value</strong></td><td><strong>Description</strong></td></tr>';
	var $warnRatio = 90;
	var $tablesSQL = false;
	var $cliFormat = "%32s => %s \r\n";
	var $sql1 = 'sql1';  // used for casting sql1 to text for mssql
	var $explain = true;
	var $helpurl = '<a href="javascript:void();">LogSQL help</a>';
	var $createTableSQL = false;
	var $maxLength = 2000;
	var $settings = false;
	var $_logsql = false;
	var $_lastload;
	
	/**
	 * Sets the table name to use for SQL logging. Returns value of current table when called.
	 * Usage: perfmon_parent_ADOConnection::table('custom_log_sql');
	 *        $currentTable = perfmon_parent_ADOConnection::table();
	 * 
	 * @access public 
	 * @param string $newtable The name for the table to use; optional.
	 * @return string
	 */            
    function table($newtable = false)
    {
        static $_table;
        if (!empty($newtable)) $_table = $newtable;
		if (empty($_table)) $_table = 'adodb_logsql';
        return $_table;
    }
	
	/**
	 * Enables SQL logging to database for Performance Monitor use.
	 * Usage: $oldValue = $db->LogSQL( $enable );
	 *        $enable is optional; defaults to TRUE enabling logging. FALSE disables logging.
	 * 
	 * @access public 
	 * @param bool $enable 
	 * @return bool
	 */
	function LogSQL($enable=true)
	{
		$old = $this->_logsql;	
		$this->_logsql = $enable;
		return $old;
	}

	/**
	 * Returns an array with information to calculate CPU Load
	 * 
	 * @access private
	 * @return mixed
	 */
	function _CPULoad() {
		// Algorithm is taken from
		// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wmisdk/wmi/example__obtaining_raw_performance_data.asp
		if (strncmp(PHP_OS,'WIN',3)==0)
		{
			if (PHP_VERSION == '5.0.0') return false;
			if (PHP_VERSION == '5.0.1') return false;
			if (PHP_VERSION == '5.0.2') return false;
			if (PHP_VERSION == '5.0.3') return false;
			if (PHP_VERSION == '4.3.10') return false; # see http://bugs.php.net/bug.php?id=31737
			
			@$c = new COM("WinMgmts:{impersonationLevel=impersonate}!Win32_PerfRawData_PerfOS_Processor.Name='_Total'");
			if (!$c) return false;
			
			$info[0] = $c->PercentProcessorTime;
			$info[1] = 0;
			$info[2] = 0;
			$info[3] = $c->TimeStamp_Sys100NS;
			return $info;
		}
		
		// Algorithm - Steve Blinch (BlitzAffe Online, http://www.blitzaffe.com)
		$statfile = '/proc/stat';
		if (!file_exists($statfile)) return false;
		
		$fd = fopen($statfile,"r");
		if (!$fd) return false;
		
		$statinfo = explode("\n",fgets($fd, 1024));
		fclose($fd);
		foreach($statinfo as $line)
		{
			$info = explode(" ",$line);
			if($info[0]=="cpu")
			{
				array_shift($info);  // pop off "cpu"
				if(!$info[0]) array_shift($info); // pop off blank space (if any)
				return $info;
			}
		}
		
		return false;
	}
	
	/* NOT IMPLEMENTED */
	function MemInfo()
	{

	}
	
	
	/**
	 * Returns CPU Load
	 * 
	 * @access public 
	 * @return mixed
	 */
	function CPULoad()
	{
		$info = $this->_CPULoad();
		if (!$info) return false;
			
		if (empty($this->_lastLoad))
		{
			sleep(1);
			$this->_lastLoad = $info;
			$info = $this->_CPULoad();
		}
		
		$last = $this->_lastLoad;
		$this->_lastLoad = $info;
		
		$d_user = $info[0] - $last[0];
		$d_nice = $info[1] - $last[1];
		$d_system = $info[2] - $last[2];
		$d_idle = $info[3] - $last[3];

		if (strncmp(PHP_OS,'WIN',3)==0)
		{
			if ($d_idle < 1) $d_idle = 1;
			return 100*(1-$d_user/$d_idle);
		}
		else
		{
			$total=$d_user+$d_nice+$d_system+$d_idle;
			if ($total<1) $total=1;
			return 100*($d_user+$d_nice+$d_system)/$total; 
		}
	}
	
	function Tracer($sql)
	{
        $perf_table = perfmon_parent_ADOConnection::table();
		$saveE = $this->LogSQL(false);
		
		global $ADODB_FETCH_MODE;
		$save = $ADODB_FETCH_MODE;
		$ADODB_FETCH_MODE = ADODB_FETCH_NUM;
				
		$sqlq = $this->qstr($sql);
		$arr = $this->GetArray(
			"select count(*),tracer 
			from $perf_table where sql1=$sqlq 
			group by tracer
			order by 1 desc"
		);
		$s = '';
		if ($arr)
		{
			$s .= '\n<h3>Scripts Affected</h3>\n';
			foreach($arr as $k)
			{
				$s .= sprintf("%4d",$k[0]).' &nbsp; '.strip_tags($k[1]).'<br />';
			}
		}
		$this->LogSQL($saveE);
		return $s;
	}

	/* 
		Explain Plan for $sql.
		If only a snippet of the $sql is passed in, then $partial will hold the crc32 of the 
			actual sql.
	*/
	function Explain($sql, $partial=false)
	{	
		return false;
	}
	
	function InvalidSQL($numsql = 10)
	{
	
		if (isset($_GET['sql'])) return;
		$s = '<h3>Invalid SQL</h3>';
		$saveE = $this->LogSQL(false);
        $perf_table = perfmon_parent_ADOConnection::table();
		$rs =& $this->SelectLimit(
			"select distinct count(*), sql1, tracer as error_msg
			from $perf_table
			where tracer like 'ERROR:%'
			group by sql1, tracer
			order by 1 desc"
			,$numsql
		);
		$this->LogSQL($saveE);
		if ($rs)
		{
			$s .= rs2html($rs,false,false,false,false);
		}
		else
		{
			return "\n<p>$this->helpurl. ".$this->ErrorMsg()."</p>\n";
		}
		return $s;
	}

	
	/*
		This script identifies the longest running SQL
	*/	
	function _SuspiciousSQL($numsql = 10)
	{
		global $ADODB_FETCH_MODE;
		
        $perf_table = perfmon_parent_ADOConnection::table();
		$saveE = $this->LogSQL(false);
		
		if (isset($_GET['exps']) && isset($_GET['sql']))
		{
			$partial = !empty($_GET['part']);
			echo '<a name="explain"></a>' . $this->Explain($_GET['sql'], $partial) . "\n";
		}
		
		if (isset($_GET['sql'])) return;
		$sql1 = $this->sql1;
		
		$save = $ADODB_FETCH_MODE;
		$ADODB_FETCH_MODE = ADODB_FETCH_NUM;

		$rs =& $this->SelectLimit(
			"select avg(timer) as avg_timer, $sql1, count(*), max(timer) as max_timer, min(timer) as min_timer
			from $perf_table
			where {$this->upperCase}({$this->substr}(sql0,1,5)) not in ('DROP ','INSER','COMMI','CREAT')
			and (tracer is null or tracer not like 'ERROR:%')
			group by sql1
			order by 1 desc"
			,$numsql
		);

		$ADODB_FETCH_MODE = $save;
		$this->LogSQL($saveE);
		
		if (!$rs) return "<p>$this->helpurl. ".$this->ErrorMsg()."</p>";
		$s = "<h3>Suspicious SQL</h3>\n<span style=\"font-size: 8pt;\">The following SQL have high average execution times</span><br />\n<table style=\"border: 2px groove #000000; background-color: #FFFFFF;\">\n<tr>\n<td><strong>Avg Time</strong></td>\n<td><strong>Count</strong></td>\n<td><strong>SQL</strong></td>\n<td><strong>Max</strong></td>\n<td><strong>Min</strong></td>\n</tr>\n";
		
		$max = $this->maxLength;
		while (!$rs->EOF)
		{
			$sql = $rs->fields[1];
			$raw = urlencode($sql);
			if (strlen($raw)>$max-100)
			{
				$sql2 = substr($sql,0,$max-500);
				$raw = urlencode($sql2).'&part='.crc32($sql);
			}
			$prefix = "<a target=\"sql".rand()."\" href=\"?hidem=1&amp;exps=1&amp;sql=".$raw."&amp;x#explain\">";
			$suffix = "</a>";
			if ($this->explain == false || strlen($prefix) > $max)
			{
				$suffix = ' ... <em>String too long for GET parameter: '.strlen($prefix).'</em>';
				$prefix = '';
			}
			
			$s .= "\n<tr>\n<td>\n" 
			.adodb_round($rs->fields[0],6)
			."\n</td><td style='text-align: right;'>\n"
			.$rs->fields[2]
			."\n</td><td>\n<span style=\"font-size: 10pt;\">"
			.$prefix
			.htmlentities($sql, ENT_QUOTES, 'UTF-8')
			.$suffix
			."</span>\n</td><td>\n"
			.$rs->fields[3]
			."\n</td><td>\n"
			.$rs->fields[4]
			."\n</tr>";
			
			$rs->MoveNext();
		}
		return $s."\n</table>\n";
		
	}
	
	function CheckMemory()
	{
		return '';
	}
	
	
	function SuspiciousSQL($numsql=10)
	{
		return perfmon_parent_ADOConnection::_SuspiciousSQL($numsql);
	}

	function ExpensiveSQL($numsql=10)
	{
		return perfmon_parent_ADOConnection::_ExpensiveSQL($numsql);
	}

	
	/*
		This reports the percentage of load on the instance due to the most 
		expensive few SQL statements. Tuning these statements can often 
		make huge improvements in overall system performance. 
	*/
	function _ExpensiveSQL($numsql = 10)
	{
		global $ADODB_FETCH_MODE;
        $perf_table = perfmon_parent_ADOConnection::table();
        
		$saveE = $this->LogSQL(false);
		
		if (isset($_GET['expe']) && isset($_GET['sql']))
		{
			$partial = !empty($_GET['part']);
			echo "\n<a name=\"explain\"></a>" . $this->Explain($_GET['sql'], $partial) . "\n";
		}
		
		if (isset($_GET['sql'])) return;
		
		$sql1 = $this->sql1;
		$save = $ADODB_FETCH_MODE;
		$ADODB_FETCH_MODE = ADODB_FETCH_NUM;
		
		$rs =& $this->SelectLimit(
			"select sum(timer) as total,$sql1,count(*),max(timer) as max_timer,min(timer) as min_timer
			from $perf_table
			where {$this->upperCase}({$this->substr}(sql0,1,5))  not in ('DROP ','INSER','COMMI','CREAT')
			and (tracer is null or tracer not like 'ERROR:%')
			group by sql1
			having count(*)>1
			order by 1 desc"
			,$numsql
		);

		$this->LogSQL($saveE);
		$ADODB_FETCH_MODE = $save;
		
		if (!$rs) return "<p>$this->helpurl. " . $this->ErrorMsg() . "</p>\n";
		$s = "\n<h3>Expensive SQL</h3>\n<span style=\"font-size: 8pt;\">Tuning the following SQL could reduce the server load substantially</span><br />\n<table style=\"border: 2px groove #000000;\">\n<tr>\n<td><strong>Load</strong></td>\n<td><strong>Count</strong></td>\n<td><strong>SQL</strong></td>\n<td><strong>Max</strong></td>\n<td><strong>Min</strong></td>\n</tr>\n";
		
		$max = $this->maxLength;
		while (!$rs->EOF)
		{
			$sql = $rs->fields[1];
			$raw = urlencode($sql);
			if (strlen($raw)>$max-100)
			{
				$sql2 = substr($sql,0,$max-500);
				$raw = urlencode($sql2).'&part='.crc32($sql);
			}
			$prefix = "<a target=\"sqle" . rand() . "\" href=\"?hidem=1&amp;expe=1&amp;sql=" . $raw . "&amp;x#explain\">";
			$suffix = "</a>\n";
			if($this->explain == false || strlen($prefix > $max))
			{
				$prefix = '';
				$suffix = '';
			}
			$s .= "\n<tr>\n<td>\n"
			.adodb_round($rs->fields[0],6)
			."\n</td><td style='text-align: right;'>\n"
			.$rs->fields[2]
			."\n</td>\n<td><span style=\"font-size: 10pt;\">"
			.$prefix
			.htmlentities($sql, ENT_QUOTES, 'UTF-8')
			.$suffix
			."</span>"
			."\n</td><td>"
			.$rs->fields[3]
			."\n</td><td>"
			.$rs->fields[4]
			."\n</tr>";
			$rs->MoveNext();
		}
		return $s."\n</table>\n";
	}
	
	/*
		Raw function to return parameter value from $settings.
	*/
	function DBParameter($param)
	{
		if (empty($this->settings[$param])) return false;
		$sql = $this->settings[$param][1];
		return $this->_DBParameter($sql);
	}
	
	/*
		Raw function returning array of poll paramters
	*/
	function &PollParameters()
	{
		$arr[0] = (float)$this->DBParameter('data cache hit ratio');
		$arr[1] = (float)$this->DBParameter('data reads');
		$arr[2] = (float)$this->DBParameter('data writes');
		$arr[3] = (integer) $this->DBParameter('current connections');
		return $arr;
	}
	
	/*
		Low-level Get Database Parameter
	*/
	function _DBParameter($sql)
	{
		$savelog = $this->LogSQL(false);
		if (is_array($sql))
		{
			global $ADODB_FETCH_MODE;
		
			$sql1 = $sql[0];
			$key = $sql[1];
			if (sizeof($sql)>2) $pos = $sql[2];
			else $pos = 1;
			if (sizeof($sql)>3) $coef = $sql[3];
			else $coef = false;
			$ret = false;
			$save = $ADODB_FETCH_MODE;
			$ADODB_FETCH_MODE = ADODB_FETCH_NUM;
			
			$rs = $this->Execute($sql1);
			
			$ADODB_FETCH_MODE = $save;
			if($rs)
			{
				while (!$rs->EOF)
				{
					$keyf = reset($rs->fields);
					if (trim($keyf) == $key)
					{
						$ret = $rs->fields[$pos];
						if ($coef) $ret *= $coef;
						break;
					}
					$rs->MoveNext();
				}
				$rs->Close();
			}
			$this->LogSQL($savelog);
			return $ret;
		}
		else
		{
			if (strncmp($sql,'=',1) == 0)
			{
				$fn = substr($sql,1);
				return $this->$fn();
			}
			$sql = str_replace('$DATABASE',$this->database,$sql);
			$ret = $this->GetOne($sql);
			$this->LogSQL($savelog);
			
			return $ret;
		}
	}
	
	/*
		Warn if cache ratio falls below threshold. Displayed in "Description" column.
	*/
	function WarnCacheRatio($val)
	{
		if ($val < $this->warnRatio) 
			 return '<span style="color: red;"><strong>Cache ratio should be at least '.$this->warnRatio.'%</strong></span>';
		else return '';
	}
	
	/***********************************************************************************************/
	//                                    HIGH LEVEL UI FUNCTIONS
	/***********************************************************************************************/

	
	function UI($pollsecs=5)
	{
	
	    $perf_table = perfmon_parent_ADOConnection::table();
		
		$app = $this->host;
		if ($this->host && $this->database) $app .= ', db=';
		$app .= $this->database;
		
		if ($app) $app .= ', ';
		$savelog = $this->LogSQL(false);	
		$info = $this->ServerInfo();
		if(isset($_GET['clearsql']))
		{
			$this->Execute("delete from $perf_table");
		}
		$this->LogSQL($savelog);
		
		// magic quotes
		
		if (isset($_GET['sql']) && get_magic_quotes_gpc())
		{
			$_GET['sql'] = $_GET['sql'] = str_replace(array("\\'",'\"'),array("'",'"'),$_GET['sql']);
		}
		
		if (!isset($_SESSION['ADODB_PERF_SQL'])) $nsql = $_SESSION['ADODB_PERF_SQL'] = 10;
		else  $nsql = $_SESSION['ADODB_PERF_SQL'];
		
		$app .= $info['description'];
		
		if(isset($_GET['do']))
		{
			$do = $_GET['do'];
		}
		else if (isset($_POST['do'])) $do = $_POST['do'];
		else if (isset($_GET['sql'])) $do = 'viewsql';
		else $do = 'stats';
		 
		if (isset($_GET['nsql']))
		{
			if ($_GET['nsql'] > 0) $nsql = $_SESSION['ADODB_PERF_SQL'] = (integer) $_GET['nsql'];
		}
		
		// page header
		echo "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">
			<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\">
			<head>
			<title>ADOdb-Lite Performance Monitor on $app</title>
			<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\" />
			<style type=\"text/css\">
				/*<![CDATA[*/
				body { background-color: #FFFFFF; font-size: 10pt; color: #000000; }
				td { padding: 0px 3px 0px 3px; }
				table { border: 2px groove #000000; }
				/*]]>*/
			</style>
			</head>
			<body>
		";
		
		if ($do == 'viewsql')
		{
			$form = "\n<form method=\"post\" action=\"". $_SERVER['PHP_SELF'] ."\"># SQL:<input type=\"hidden\" value=\"viewsql\" name=\"do\" /> <input type=\"text\" size=\"4\" name=\"nsql\" value=\"$nsql\" /><input type=\"submit\" value=\"Go\" /></form>";
		}
		else
		{
			$form = "&nbsp;";
		}
		
		$allowsql = !defined('ADODB_PERF_NO_RUN_SQL');
		
		if(empty($_GET['hidem']))
		{
			echo "\n<table style=\"width: 100%; background-color: lightyellow;\">\n<tr>\n<td colspan='2'>\n<strong><a href=\"http://adodb.sourceforge.net/?perf=1\">ADOdb-Lite</a> Performance Monitor</strong> <span style=\"font-size: 8pt;\">for $app</span>\n</td>\n</tr>\n<tr>\n<td>\n<a href=\"?do=stats\"><strong>Performance Stats</strong></a> &nbsp; <a href=\"?do=viewsql\"><strong>View SQL</strong></a>&nbsp; <a href=\"?do=tables\"><strong>View Tables</strong></a> &nbsp; <a href=\"?do=poll\"><strong>Poll Stats</strong></a>",
		 $allowsql ? ' &nbsp; <a href="?do=dosql"><strong>Run SQL</strong></a></td>' : '</td>',
		 "\n<td>$form\n</td>",
		 "\n</tr>\n</table>\n";
		}

	 	switch ($do) 
	 	{
			case 'stats':
				echo $this->HealthCheck();
				echo $this->CheckMemory();
				break;
				
			case 'poll':
				echo "<iframe width=\"720\" height=\"80%\" 
					src=\"{$_SERVER['PHP_SELF']}?do=poll2&hidem=1\"></iframe>";
				break;
				
			case 'poll2':
				echo "<pre>";
				$this->Poll($pollsecs);
				echo "</pre>";
				break;
			
			case 'dosql':
				if (!$allowsql) break;
				$this->DoSQLForm();
				break;
				
			case 'viewsql':
				if (empty($_GET['hidem']))
				{
					echo "&nbsp; <a href=\"?do=viewsql&amp;clearsql=1\">Clear SQL Log</a><br />";
				}
				echo($this->SuspiciousSQL($nsql));
				echo($this->ExpensiveSQL($nsql));
				echo($this->InvalidSQL($nsql));
				break;
				
			case 'tables': 
				echo $this->Tables();
				break;
				
			default:
				echo $this->HealthCheck();
				echo $this->CheckMemory();
				break;
		}
		global $ADODB_vers;
		echo "<div align=\"center\"><span style=\"font-size: 8pt;\">$ADODB_vers</span></div>";
	}
	
	/*
		Runs in infinite loop, returning real-time statistics
	*/
	function Poll($secs=5)
	{
		//$saveE = $this->LogSQL(false); // but how to re-enable?

		if ($secs <= 1) $secs = 1;
		echo "Accumulating statistics, every $secs seconds...\n"; flush();
		$arro =& $this->PollParameters();
		$cnt = 0;
		set_time_limit(0);
		sleep($secs);
		while (1) {

			$arr =& $this->PollParameters();
			
			$hits   = sprintf('%2.2f',$arr[0]);
			$reads  = sprintf('%12.4f',($arr[1]-$arro[1])/$secs);
			$writes = sprintf('%12.4f',($arr[2]-$arro[2])/$secs);
			$sess = sprintf('%5d',$arr[3]);
			
			$load = $this->CPULoad();
			if ($load !== false)
			{
				$oslabel = 'WS-CPU%';
				$osval = sprintf(" %2.1f  ",(float) $load);
			}
			else
			{
				$oslabel = '';
				$osval = '';
			}
			if ($cnt % 10 == 0) echo " Time   ".$oslabel."   Hit%   Sess           Reads/s          Writes/s\n"; 
			$cnt += 1;
			echo date('H:i:s').'  '.$osval."$hits  $sess $reads $writes\n";
			flush();
			
			if (connection_aborted()) return;
			
			sleep($secs);
			$arro = $arr;
		}
	}
	
	/*
		Returns basic health check in a command line interface
	*/
	function HealthCheckCLI()
	{
		return $this->HealthCheck(true);
	}
	
		
	/*
		Returns basic health check as HTML
	*/
	function HealthCheck($cli=false)
	{
		$saveE = $this->LogSQL(false);
		if ($cli) $html = '';
		else $html = $this->table.'<tr><td colspan="3"><h3>'.$this->dbtype.'</h3></td></tr>'.$this->titles;
		
		foreach($this->settings as $name => $arr)
		{
			if ($arr === false) break;
			
			if (!is_string($name))
			{
				if ($cli) $html .= " -- $arr -- \n";
				else $html .= "<tr style=\"background-color: $this->color;\"><td colspan=\"3\"><em>$arr</em> &nbsp;</td></tr>";
				continue;
			}
			
			if (!is_array($arr)) break;
			$category = $arr[0];
			$how = $arr[1];
			if (sizeof($arr)>2) $desc = $arr[2];
			else $desc = ' &nbsp; ';
			
			
			if ($category == 'HIDE') continue;
			
			$val = $this->_DBParameter($how);
			
			if ($desc && strncmp($desc,"=",1) === 0)
			{
				$fn = substr($desc,1);
				$desc = $this->$fn($val);
			}
			
			if ($val === false)
			{
				$m = $this->ErrorMsg();
				$val = "Error: $m"; 
			}
			else
			{
				if (is_numeric($val) && $val >= 256*1024)
				{
					if ($val % (1024*1024) == 0)
					{
						$val /= (1024*1024);
						$val .= 'M';
					}
					else if ($val % 1024 == 0)
					{
						$val /= 1024;
						$val .= 'K';
					}
				}
			}
			if ($category != $oldc)
			{
				$oldc = $category;
			}
			if (strlen($desc)==0) $desc = '&nbsp;';
			if (strlen($val)==0) $val = '&nbsp;';
			if ($cli)
			{
				$html  .= str_replace('&nbsp;','',sprintf($this->cliFormat,strip_tags($name),strip_tags($val),strip_tags($desc)));	
			}
			else
			{
				$html .= "<tr><td>".$name.'</td><td>'.$val.'</td><td>'.$desc."</td></tr>\n";
			}
		}
		
		if (!$cli) $html .= "</table>\n";
		$this->LogSQL($saveE);
			
		return $html;	
	}
	
	function Tables($orderby='1')
	{
		if (!$this->tablesSQL) return false;
		
		$savelog = $this->LogSQL(false);
		$rs = $this->Execute($this->tablesSQL.' order by '.$orderby);
		$this->LogSQL($savelog);
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
	
	function DoSQLForm()
	{
		$PHP_SELF = $_SERVER['PHP_SELF'];
		$sql = isset($_REQUEST['sql']) ? $_REQUEST['sql'] : ''; // Let the form spoofing commence... ***

		if (isset($_SESSION['phplens_sqlrows'])) $rows = $_SESSION['phplens_sqlrows'];
		else $rows = 3;
		
		if (isset($_REQUEST['SMALLER'])) {
			$rows /= 2;
			if ($rows < 3) $rows = 3;
			$_SESSION['phplens_sqlrows'] = $rows;
		}
		if (isset($_REQUEST['BIGGER'])) {
			$rows *= 2;
			$_SESSION['phplens_sqlrows'] = $rows;
		}
		
?>

<form method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>">
<table><tr>
<td> Form size: <input type="submit" value=" &lt; " name="SMALLER" /><input type="submit" value=" &gt; &gt; " name="BIGGER" />
</td>
<td align=right>
<input type="submit" value=" Run SQL Below " name="RUN" /><input type="hidden" name="do" value="dosql" />
</td></tr>
  <tr>
  <td colspan="2"><textarea rows="<?php print $rows; ?>" name="sql" cols="80"><?php print htmlentities($sql, ENT_QUOTES, 'UTF-8'); ?></textarea>
  </td>
  </tr>
 </table>
</form>

<?php
		if (!isset($_REQUEST['sql'])) return;
		
		$sql = $this->undomq(trim($sql));
		if (substr($sql,strlen($sql)-1) === ';')
		{
			$print = true;
			$sqla = $this->SplitSQL($sql);
		}
		else 
		{
			$print = false;
			$sqla = array($sql);
		}
		foreach($sqla as $sqls) {

			if (!$sqls) continue;
			
			if ($print) {
				print "<p>".htmlentities($sqls, ENT_QUOTES, 'UTF-8')."</p>";
				flush();
			}
			$savelog = $this->LogSQL(false);
			$rs = $this->Execute($sqls);
			$this->LogSQL($savelog);
			if ($rs && is_object($rs) && !$rs->EOF)
			{
				rs2html($rs);
				while ($rs->NextRecordSet())
				{
					print "<table style=\"width: 98%; background-color: #C0C0FF;\"><tr><td>&nbsp;</td></tr></table>";
					rs2html($rs);
				}
			}
			else
			{
				$e1 = (integer) $this->ErrorNo();
				$e2 = $this->ErrorMsg();
				if (($e1) || ($e2))
				{
					if (empty($e1)) $e1 = '-1'; // postgresql fix
					print ' &nbsp; '.$e1.': '.$e2;
				}
				else
				{
					print "<p>No Recordset returned<br /></p>";
				}
			}
		} // foreach
	}
	
	function SplitSQL($sql)
	{
		$arr = explode(';',$sql);
		return $arr;
	}
	
	function undomq(&$m) 
	{
		if (get_magic_quotes_gpc()) {
			// undo the damage
			$m = str_replace('\\\\','\\',$m);
			$m = str_replace('\"','"',$m);
			$m = str_replace('\\\'','\'',$m);
		}
		return $m;
	}

    
   /************************************************************************/
   
    /** 
     * Reorganise multiple table-indices/statistics/..
     * OptimizeMode could be given by last Parameter
     * 
     * @example
     *      <pre>
     *          optimizeTables( 'tableA');
     *      </pre>
     *      <pre>
     *          optimizeTables( 'tableA', 'tableB', 'tableC');
     *      </pre>
     *      <pre>
     *          optimizeTables( 'tableA', 'tableB', ADODB_OPT_LOW);
     *      </pre>
     * 
     * @param string table name of the table to optimize
     * @param int mode optimization-mode
     *      <code>ADODB_OPT_HIGH</code> for full optimization 
     *      <code>ADODB_OPT_LOW</code> for CPU-less optimization
     *      Default is LOW <code>ADODB_OPT_LOW</code> 
     * @author Markus Staab
     * @return Returns <code>true</code> on success and <code>false</code> on error
     */
    function OptimizeTables()
    {
        $args = func_get_args();
        $numArgs = func_num_args();
        
        if ( $numArgs == 0) return false;
        
        $mode = ADODB_OPT_LOW; 
        $lastArg = $args[ $numArgs - 1];
        if ( !is_string($lastArg))
        {
            $mode = $lastArg;
            unset( $args[ $numArgs - 1]);
        }
        
        foreach( $args as $table)
        {
            $this->optimizeTable( $table, $mode);
        }
	}

    /** 
     * Reorganise the table-indices/statistics/.. depending on the given mode.
     * Default Implementation throws an error.
     * 
     * @param string table name of the table to optimize
     * @param int mode optimization-mode
     *      <code>ADODB_OPT_HIGH</code> for full optimization 
     *      <code>ADODB_OPT_LOW</code> for CPU-less optimization
     *      Default is LOW <code>ADODB_OPT_LOW</code> 
     * @author Markus Staab
     * @return Returns <code>true</code> on success and <code>false</code> on error
     */
    function OptimizeTable( $table, $mode = ADODB_OPT_LOW) 
    {
        $this->outp( sprintf( "<p>%s: '%s' not implemented for driver '%s'</p>", __CLASS__, __FUNCTION__, $this->dbtype));
        return false;
    }
    
    /** 
     * Reorganise current database.
     * Default implementation loops over all <code>MetaTables()</code> and 
     * optimize each using <code>optmizeTable()</code>
     *
     * Non-functional in ADOdb-Lite due to lack of MetaTables() implementation in default modules
     * 
     * @author Markus Staab
     * @return Returns <code>true</code> on success and <code>false</code> on error
     */
    function optimizeDatabase() 
    {  
        //$tables = $this->MetaTables( 'TABLES'); // non-functional without a MetaTables method in ADOdb-Lite...
        if ( !$tables ) return false;

        foreach( $tables as $table)
        {
            if (!$this->optimizeTable( $table))
            {
                return false;
            }
        }
        return true;
    }

}

?>
