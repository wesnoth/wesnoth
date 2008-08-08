<?php
/**
 * ADOdb Lite Date Module for Microsoft SQL Pro
 * 
 */

if (!defined('TIMESTAMP_FIRST_YEAR')) define('TIMESTAMP_FIRST_YEAR',100);

@include(ADODB_DIR . '/adodb-time.inc.php');

if (strnatcmp(PHP_VERSION, '4.3.0') >= 0) {
	ini_set('mssql.datetimeconvert',0); 
} else {
	global $ADODB_mssql_mths;		// array, months must be upper-case

	$ADODB_mssql_date_order = 'mdy'; 
	$ADODB_mssql_mths = array(
		'JAN'=>1,'FEB'=>2,'MAR'=>3,'APR'=>4,'MAY'=>5,'JUN'=>6,
		'JUL'=>7,'AUG'=>8,'SEP'=>9,'OCT'=>10,'NOV'=>11,'DEC'=>12);
}

//---------------------------------------------------------------------------
// Call this to autoset $ADODB_mssql_date_order at the beginning of your code,
// just after you connect to the database. Supports mdy and dmy only.
// Not required for PHP 4.2.0 and above.
function AutoDetect_MSSQL_Date_Order($conn)
{
	global $ADODB_mssql_date_order;
	$adate = $conn->GetOne('select getdate()');
	if ($adate) {
		$anum = (int) $adate;
		if ($anum > 0) {
			if ($anum > 31) {
				//ADOConnection::outp( "MSSQL: YYYY-MM-DD date format not supported currently");
			} else
				$ADODB_mssql_date_order = 'dmy';
		} else
			$ADODB_mssql_date_order = 'mdy';
	}
}

eval('class mssqlpo_date_EXTENDER extends '. $last_module . '_ADOConnection { }');

class mssqlpo_date_ADOConnection extends mssqlpo_date_EXTENDER
{
	var $fmtDate = "'Y-m-d'";
	var $fmtTimeStamp = "'Y-m-d H:i:s'";
	var $emptyDate = '&nbsp;';
	var $emptyTimeStamp = '&nbsp;';
	var $sysDate = 'convert(datetime,convert(char,GetDate(),102),102)';
	var $sysTimeStamp = 'GetDate()';
	var $isoDates = false; /// accepts dates in ISO format

	function Time()
	{
		$rs =& $this->_Execute("select $this->sysTimeStamp");
		if ($rs && !$rs->EOF)
			return $this->UnixTimeStamp(reset($rs->fields));

		return false;
	}

	function OffsetDate($dayFraction, $date=false)
	{
		if (!$date)
			$date = $this->sysDate;

		return  '('.$date.'+'.$dayFraction.')';
	}

	function SetDateLocale($locale = 'En')
	{
		$this->locale = $locale;
		switch (strtoupper($locale))
		{
			case 'EN':
				$this->fmtDate="'Y-m-d'";
				$this->fmtTimeStamp = "'Y-m-d H:i:s'";
				break;

			case 'US':
				$this->fmtDate = "'m-d-Y'";
				$this->fmtTimeStamp = "'m-d-Y H:i:s'";
				break;

			case 'NL':
			case 'FR':
			case 'RO':
			case 'IT':
				$this->fmtDate="'d-m-Y'";
				$this->fmtTimeStamp = "'d-m-Y H:i:s'";
				break;

			case 'GE':
				$this->fmtDate="'d.m.Y'";
				$this->fmtTimeStamp = "'d.m.Y H:i:s'";
				break;

			default:
				$this->fmtDate="'Y-m-d'";
				$this->fmtTimeStamp = "'Y-m-d H:i:s'";
				break;
		}
	}

	function DBDate($date)
	{
		if (empty($date) && $date !== 0)
			return 'null';

		if (is_string($date) && !is_numeric($date)) {
			if ($date === 'null' || strncmp($date, "'", 1) === 0)
				return $date;

			if ($this->isoDates)
				return "'$date'";

			$date = $this->UnixDate($date);
		}

		return adodb_date($this->fmtDate,$date);
	}

	function DBTimeStamp($timestamp)
	{
		if (empty($timestamp) && $timestamp !== 0)
			return 'null';

		# strlen(14) allows YYYYMMDDHHMMSS format
		if (!is_string($timestamp) || (is_numeric($timestamp) && strlen($timestamp)<14)) 
			return adodb_date($this->fmtTimeStamp, $timestamp);

		if ($timestamp === 'null')
			return $timestamp;

		if ($this->isoDates && strlen($timestamp) !== 14)
			return "'$timestamp'";

		$timestamp = $this->UnixTimeStamp($timestamp);
		return adodb_date($this->fmtTimeStamp, $timestamp);
	}

	function UnixDate($v)
	{
		global $ADODB_mssql_mths,$ADODB_mssql_date_order;

		if (is_numeric(substr($v,0,1)) && strnatcmp(PHP_VERSION, '4.2.0') >= 0)
			return $v;

		//Dec 30 2000 12:00AM 
		if ($ADODB_mssql_date_order == 'dmy') {
			if (!preg_match( "|^([0-9]{1,2})[-/\. ]+([A-Za-z]{3})[-/\. ]+([0-9]{4})|" ,$v, $rr)) {
				return $v;
			}
			if ($rr[3] <= TIMESTAMP_FIRST_YEAR)
				return 0;

			$theday = $rr[1];
			$themth =  substr(strtoupper($rr[2]), 0, 3);
		} else {
			if (!preg_match( "|^([A-Za-z]{3})[-/\. ]+([0-9]{1,2})[-/\. ]+([0-9]{4})|" ,$v, $rr)) {
				return $v;
			}
			if ($rr[3] <= TIMESTAMP_FIRST_YEAR)
				return 0;

			$theday = $rr[2];
			$themth = substr(strtoupper($rr[1]), 0, 3);
		}
		$themth = $ADODB_mssql_mths[$themth];
		if ($themth <= 0)
			return false;

		// h-m-s-MM-DD-YY
		return  mktime(0, 0, 0, $themth, $theday, $rr[3]);
	}

	function UnixTimeStamp($v)
	{
		global $ADODB_mssql_mths,$ADODB_mssql_date_order;

		if (is_numeric(substr($v,0,1)) && strnatcmp(PHP_VERSION, '4.2.0') >= 0)
			return $v;

		//Dec 30 2000 12:00AM
		 if ($ADODB_mssql_date_order == 'dmy') {
			if (!preg_match( "|^([0-9]{1,2})[-/\. ]+([A-Za-z]{3})[-/\. ]+([0-9]{4}) +([0-9]{1,2}):([0-9]{1,2}) *([apAP]{0,1})|",$v , $rr))
			 	return $v;

			if ($rr[3] <= TIMESTAMP_FIRST_YEAR)
				return 0;

			$theday = $rr[1];
			$themth =  substr(strtoupper($rr[2]), 0, 3);
		} else {
			if (!preg_match( "|^([A-Za-z]{3})[-/\. ]+([0-9]{1,2})[-/\. ]+([0-9]{4}) +([0-9]{1,2}):([0-9]{1,2}) *([apAP]{0,1})|",$v , $rr))
				return $v;

			if ($rr[3] <= TIMESTAMP_FIRST_YEAR)
				return 0;

			$theday = $rr[2];
			$themth = substr(strtoupper($rr[1]), 0, 3);
		}

		$themth = $ADODB_mssql_mths[$themth];
		if ($themth <= 0)
			return false;

		switch (strtoupper($rr[6])) {
			case 'P':
				if ($rr[4]<12)
					$rr[4] += 12;
				break;
			case 'A':
				if ($rr[4]==12)
					$rr[4] = 0;
				break;
			default:
				break;
		}
		// h-m-s-MM-DD-YY
		return  mktime($rr[4], $rr[5], 0, $themth, $theday, $rr[3]);
	}

	function UserDate($v, $fmt='Y-m-d', $gmt=false)
	{
		$tt = $this->UnixDate($v);

		// $tt == -1 if pre TIMESTAMP_FIRST_YEAR
		if (($tt === false || $tt == -1) && $v != false)
			return $v;
		else if ($tt == 0)
			return $this->emptyDate;
		else if ($tt == -1) { // pre-TIMESTAMP_FIRST_YEAR
		}

		return ($gmt) ? adodb_gmdate($fmt, $tt) : adodb_date($fmt, $tt);
	}

	function UserTimeStamp($v, $fmt='Y-m-d H:i:s', $gmt=false)
	{
		if (!isset($v))
			return $this->emptyTimeStamp;

		# strlen(14) allows YYYYMMDDHHMMSS format
		if (is_numeric($v) && strlen($v)<14)
			return ($gmt) ? adodb_gmdate($fmt,$v) : adodb_date($fmt,$v);

		$tt = $this->UnixTimeStamp($v);
		// $tt == -1 if pre TIMESTAMP_FIRST_YEAR
		if (($tt === false || $tt == -1) && $v != false)
			return $v;

		if ($tt == 0)
			return $this->emptyTimeStamp;
		else return ($gmt) ? adodb_gmdate($fmt,$tt) : adodb_date($fmt,$tt);
	}

	function SQLDate($fmt, $col=false)
	{	
		if (!$col) $col = $this->sysTimeStamp;
		$s = '';
		
		$len = strlen($fmt);
		for ($i=0; $i < $len; $i++) {
			if ($s) $s .= '+';
			$ch = $fmt[$i];
			switch($ch) {
			case 'Y':
			case 'y':
				$s .= "datename(yyyy,$col)";
				break;
			case 'M':
				$s .= "convert(char(3),$col,0)";
				break;
			case 'm':
				$s .= "replace(str(month($col),2),' ','0')";
				break;
			case 'Q':
			case 'q':
				$s .= "datename(quarter,$col)";
				break;
			case 'D':
			case 'd':
				$s .= "replace(str(day($col),2),' ','0')";
				break;
			case 'h':
				$s .= "substring(convert(char(14),$col,0),13,2)";
				break;
			
			case 'H':
				$s .= "replace(str(datepart(hh,$col),2),' ','0')";
				break;
				
			case 'i':
				$s .= "replace(str(datepart(mi,$col),2),' ','0')";
				break;
			case 's':
				$s .= "replace(str(datepart(ss,$col),2),' ','0')";
				break;
			case 'a':
			case 'A':
				$s .= "substring(convert(char(19),$col,0),18,2)";
				break;
				
			default:
				if ($ch == '\\') {
					$i++;
					$ch = substr($fmt,$i,1);
				}
				$s .= $this->qstr($ch);
				break;
			}
		}
		return $s;
	}
}

eval('class mssqlpo_date_resultset_EXTENDER extends '. $last_module . '_ResultSet { }');

class mssqlpo_date_ResultSet extends mssqlpo_date_resultset_EXTENDER
{
	var $emptyTimeStamp = '&nbsp;'; /// what to display when $time==0
	var $emptyDate = '&nbsp;'; /// what to display when $time==0
	var $datetime = false;

	function UserTimeStamp($v, $fmt='Y-m-d H:i:s')
	{
		if (is_numeric($v) && strlen($v)<14)
			return adodb_date($fmt,$v);

		$tt = $this->UnixTimeStamp($v);
		// $tt == -1 if pre TIMESTAMP_FIRST_YEAR
		if (($tt === false || $tt == -1) && $v != false)
			return $v;

		if ($tt === 0)
			return $this->emptyTimeStamp;
		else return adodb_date($fmt,$tt);
	}

	function UserDate($v,$fmt='Y-m-d')
	{
		$tt = $this->UnixDate($v);
		// $tt == -1 if pre TIMESTAMP_FIRST_YEAR
		if (($tt === false || $tt == -1) && $v != false)
			return $v;
		else if ($tt == 0)
			return $this->emptyDate;
		else if ($tt == -1) { // pre-TIMESTAMP_FIRST_YEAR
		}
		return adodb_date($fmt,$tt);
	}

	function UnixDate($v)
	{
		return mssqlpo_date_ADOConnection::UnixDate($v);
	}

	function UnixTimeStamp($v)
	{
		return mssqlpo_date_ADOConnection::UnixTimeStamp($v);
	}

}
?>