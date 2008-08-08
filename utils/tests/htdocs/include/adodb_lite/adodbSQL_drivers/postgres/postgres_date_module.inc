<?php
/**
 * ADOdb Lite Date Module for Postgres
 * 
 */

if (!defined('TIMESTAMP_FIRST_YEAR')) define('TIMESTAMP_FIRST_YEAR',100);

@include(ADODB_DIR . '/adodb-time.inc.php');

eval('class postgres_date_EXTENDER extends '. $last_module . '_ADOConnection { }');

class postgres_date_ADOConnection extends postgres_date_EXTENDER
{
	var $fmtDate = "'Y-m-d'";	// used by DBDate() as the default date format used by the database
	var $fmtTimeStamp = "'Y-m-d H:i:s'"; // used by DBTimeStamp as the default timestamp fmt.
	var $emptyDate = '&nbsp;';
	var $emptyTimeStamp = '&nbsp;';
	var $sysDate = "CURRENT_DATE";
	var $sysTimeStamp = "CURRENT_TIMESTAMP";
	var $isoDates = true; /// accepts dates in ISO format

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
		else if (strncmp($date,"'",1) == 0) {
			$len = strlen($date);
			if (10 <= $len && $len <= 12)
				$date = 'date ' . $date;
			else $date = 'timestamp ' . $date;
		}
		return "($date+interval'$dayFraction days')";
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
		if (is_object($v)) {
		// odbtp support
		//( [year] => 2004 [month] => 9 [day] => 4 [hour] => 12 [minute] => 44 [second] => 8 [fraction] => 0 )
			return adodb_mktime($v->hour, $v->minute, $v->second, $v->month, $v->day, $v->year);
		}
	
		if (is_numeric($v) && strlen($v) !== 8)
			return $v;

		if (!preg_match( "|^([0-9]{4})[-/\.]?([0-9]{1,2})[-/\.]?([0-9]{1,2})|", ($v), $rr))
			return false;

		if ($rr[1] <= TIMESTAMP_FIRST_YEAR)
			return 0;

		// h-m-s-MM-DD-YY
		return adodb_mktime(0, 0, 0, $rr[2], $rr[3], $rr[1]);
	}

	function UnixTimeStamp($v)
	{
		if (is_object($v)) {
		// odbtp support
		//( [year] => 2004 [month] => 9 [day] => 4 [hour] => 12 [minute] => 44 [second] => 8 [fraction] => 0 )
			return adodb_mktime($v->hour, $v->minute, $v->second, $v->month, $v->day, $v->year);
		}
		
		if (!preg_match("|^([0-9]{4})[-/\.]?([0-9]{1,2})[-/\.]?([0-9]{1,2})[ ,-]*(([0-9]{1,2}):?([0-9]{1,2}):?([0-9\.]{1,4}))?|", ($v), $rr))
			return false;

		if ($rr[1] <= TIMESTAMP_FIRST_YEAR && $rr[2]<= 1)
			return 0;

		// h-m-s-MM-DD-YY
		if (!isset($rr[5]))
			return adodb_mktime(0, 0, 0, $rr[2], $rr[3], $rr[1]);
		else return adodb_mktime($rr[5], $rr[6], $rr[7], $rr[2], $rr[3], $rr[1]);
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
		if (!$col)
			$col = $this->sysTimeStamp;

		$s = 'TO_CHAR('.$col.",'";
		$len = strlen($fmt);
		for ($i=0; $i < $len; $i++) {
			$ch = $fmt[$i];
			switch($ch) {
				case 'Y':
				case 'y':
					$s .= 'YYYY';
					break;

				case 'Q':
				case 'q':
					$s .= 'Q';
					break;

				case 'M':
					$s .= 'Mon';
					break;

				case 'm':
					$s .= 'MM';
					break;

				case 'D':
				case 'd':
					$s .= 'DD';
					break;

				case 'H':
					$s.= 'HH24';
					break;

				case 'h':
					$s .= 'HH';
					break;

				case 'i':
					$s .= 'MI';
					break;

				case 's':
					$s .= 'SS';
					break;

				case 'a':
				case 'A':
					$s .= 'AM';
					break;

				case 'w':
					$s .= 'D';
					break;

				case 'l':
					$s .= 'DAY';
					break;

				default:
				// handle escape characters...
					if ($ch == '\\') {
						$i++;
						$ch = substr($fmt,$i,1);
					}
					if (strpos('-/.:;, ',$ch) !== false)
						$s .= $ch;
					else $s .= '"'.$ch.'"';
			}
		}
		return $s. "')";
	}
}

eval('class postgres_date_resultset_EXTENDER extends '. $last_module . '_ResultSet { }');

class postgres_date_ResultSet extends postgres_date_resultset_EXTENDER
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
		return postgres_date_ADOConnection::UnixDate($v);
	}

	function UnixTimeStamp($v)
	{
		return postgres_date_ADOConnection::UnixTimeStamp($v);
	}

}
?>