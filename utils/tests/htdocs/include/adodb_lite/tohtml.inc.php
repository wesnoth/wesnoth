<?php 
/*
  V4.65 22 July 2005  (c) 2000-2005 John Lim (jlim@natsoft.com.my). All rights reserved.
  Released under both BSD license and Lesser GPL library license. 
  Whenever there is any discrepancy between the two licenses, 
  the BSD license will take precedence.
  
  Some pretty-printing by Chris Oxenreider <oxenreid@state.net>
*/ 
  
// specific code for tohtml
GLOBAL $gSQLMaxRows,$gSQLBlockRows,$ADODB_ROUND;

$ADODB_ROUND=4; // rounding
$gSQLMaxRows = 1000; // max no of rows to download
$gSQLBlockRows=20; // max no of rows per table block

// RecordSet to HTML Table
//------------------------------------------------------------
// Convert a recordset to a html table. Multiple tables are generated
// if the number of rows is > $gSQLBlockRows. This is because
// web browsers normally require the whole table to be downloaded
// before it can be rendered, so we break the output into several
// smaller faster rendering tables.
//
// $rs: the recordset
// $ztabhtml: the table tag attributes (optional)
// $zheaderarray: contains the replacement strings for the headers (optional)
//
//  USAGE:
//	include('adodb.inc.php');
//	$db = ADONewConnection('mysql');
//	$db->Connect('mysql','userid','password','database');
//	$rs = $db->Execute('select col1,col2,col3 from table');
//	rs2html($rs, 'BORDER=2', array('Title1', 'Title2', 'Title3'));
//	$rs->Close();
//
// RETURNS: number of rows displayed


function rs2html(&$rs,$ztabhtml=false,$zheaderarray=false,$htmlspecialchars=true,$echo = true)
{
	$s ='';$rows=0;$docnt = false;
	GLOBAL $gSQLMaxRows,$gSQLBlockRows,$ADODB_ROUND;

	if (!$rs)
	{
		printf(ADODB_BAD_RS,'rs2html');
		return false;
	}
	
	if(!$ztabhtml)
	{
		$ztabhtml = 'style="border: 2px groove #000000; background-color: #FFFFFF; width: 98%;"';
	}
;
	$typearr = array();
	$ncols = $rs->FieldCount();
	$hdr = "\n<table $ztabhtml><tr>\n\n"; //removed cols='$ncols' as proprietary table attribute
	for ($i=0; $i < $ncols; $i++)
	{	
		$field = $rs->FetchField($i);
		if ($field)
		{
			if ($zheaderarray) $fname = $zheaderarray[$i];
			else $fname = htmlentities($field->name, ENT_QUOTES, 'UTF-8');	
			$typearr[$i] = $rs->MetaType($field->type,$field->max_length);
		}
		else
		{
			$fname = 'Field '.($i+1);
			$typearr[$i] = 'C';
		}
		if (strlen($fname)==0) $fname = '&nbsp;';
		$hdr .= "\n<th>$fname</th>";
	}
	$hdr .= "\n</tr>";
	if ($echo) print $hdr."\n\n";
	else $html = $hdr;
	
	// smart algorithm - handles ADODB_FETCH_MODE's correctly by probing...
	$numoffset = isset($rs->fields[0]) ||isset($rs->fields[1]) || isset($rs->fields[2]);
	while (!$rs->EOF) {
		
		$s .= "\n<tr style='vertical-align: top;'>\n";
		
		for ($i=0; $i < $ncols; $i++) {
			if ($i===0) $v=($numoffset) ? $rs->fields[0] : reset($rs->fields);
			else $v = ($numoffset) ? $rs->fields[$i] : next($rs->fields);
			
			$type = $typearr[$i];
			switch($type) {
			case 'D':
				if (empty($v)) $s .= "<td> &nbsp; </td>\n";
				else if (!strpos($v,':')) {
					$s .= "	<td>". $v ."&nbsp;</td>\n";
				}
				break;
			case 'T':
				if (empty($v)) $s .= "<td> &nbsp; </td>\n";
				else $s .= "	<td>".$v."&nbsp;</td>\n";
			break;
			
			case 'N':
				if (abs($v) - round($v,0) < 0.00000001)
					$v = round($v);
				else
					$v = round($v,$ADODB_ROUND);
			case 'I':
				$s .= "	<td style='text-align: right;'>".stripslashes((trim($v))) ."&nbsp;</td>\n";
			   	
			break;

			default:
				if ($htmlspecialchars) $v = htmlentities(trim($v), ENT_QUOTES, 'UTF-8');
				$v = trim($v);
				if (strlen($v) == 0) $v = '&nbsp;';
				$s .= "	<td>". str_replace("\n",'<br />',stripslashes($v)) ."</td>\n";
			  
			}
		} // for
		$s .= "</tr>\n\n";
			  
		$rows += 1;
		if ($rows >= $gSQLMaxRows) {
			$rows = "<p>Truncated at $gSQLMaxRows</p>";
			break;
		} // switch

		$rs->MoveNext();
	
	// additional EOF check to prevent a widow header
		if (!$rs->EOF && $rows % $gSQLBlockRows == 0) {
	
		//if (connection_aborted()) break;// not needed as PHP aborts script, unlike ASP
			if ($echo) print $s . "</table>\n\n";
			else $html .= $s ."</table>\n\n";
			$s = $hdr;
		}
	} // while

	if ($echo) print $s."</table>\n\n";
	else $html .= $s."</table>\n\n";
	
	if ($docnt) if ($echo) print "<h2>".$rows." Rows</h2>";
	
	return ($echo) ? $rows : $html;
 }
 
// pass in 2 dimensional array
function arr2html(&$arr,$ztabhtml='',$zheaderarray='')
{
	if (!$ztabhtml) $ztabhtml = 'style="border: 2px groove #000000; background-color: #FFFFFF;"';
	
	$s = "<table $ztabhtml>";//';print_r($arr);

	if ($zheaderarray) {
		$s .= '<tr>';
		for ($i=0; $i<sizeof($zheaderarray); $i++) {
			$s .= "	<th>{$zheaderarray[$i]}</th>\n";
		}
		$s .= "\n</tr>";
	}
	
	for ($i=0; $i<sizeof($arr); $i++) {
		$s .= '<tr>';
		$a = &$arr[$i];
		if (is_array($a)) 
			for ($j=0; $j<sizeof($a); $j++) {
				$val = $a[$j];
				if (empty($val)) $val = '&nbsp;';
				$s .= "	<td>$val</td>\n";
			}
		else if ($a) {
			$s .=  '	<td>'.$a."</td>\n";
		} else $s .= "	<td>&nbsp;</td>\n";
		$s .= "\n</tr>\n";
	}
	$s .= '</table>';
	print $s;
}

?>
