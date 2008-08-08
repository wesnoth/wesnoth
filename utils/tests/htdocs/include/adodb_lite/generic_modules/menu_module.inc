<?php
// Generic Menu Module

eval('class menu_EXTENDER extends ' . $last_module . '_ADOConnection { }');

class menu_ADOConnection extends menu_EXTENDER
{
}

eval('class menu_resultset_EXTENDER extends ' . $last_module . '_ResultSet { }');

class menu_ResultSet extends menu_resultset_EXTENDER
{
	// Requires $ADODB_FETCH_MODE = ADODB_FETCH_NUM
	function GetMenu($name, $defstr='', $blank1stItem=true, $multiple=false, $size=0, $selectAttr='', $compareFields0=true)
	{
		$hasvalue = false;

		if($multiple or is_array($defstr))
		{
			if($size==0)
				$size=5;
			$attr = ' multiple size="'.$size.'"';
			if(!strpos($name,'[]'))
				$name .= '[]';
		}
		else if($size)
			$attr = ' size="'.$size.'"';
		else $attr ='';

		$s = '<select name="'.$name.'"'.$attr.' '.$selectAttr.'>';
		if($blank1stItem) 
			if(is_string($blank1stItem))
			{
				$barr = explode(':',$blank1stItem);
				if(sizeof($barr) == 1)
					$barr[] = '';
				$s .= "\n<option value=\"".$barr[0]."\">".$barr[1]."</option>";
			} else $s .= "\n<option></option>";

		if($this->FieldCount() > 1)
			$hasvalue=true;
		else $compareFields0 = true;

		$value = '';
		$optgroup = null;
		$firstgroup = true;
		$fieldsize = $this->FieldCount();
		while(!$this->EOF)
		{
			$zval = rtrim(reset($this->fields));

			if($blank1stItem && $zval=="")
			{
				$this->MoveNext();
				continue;
			}

			if($fieldsize > 1)
			{
				if(isset($this->fields[1]))
					$zval2 = rtrim($this->fields[1]);
				else
					$zval2 = rtrim(next($this->fields));
			}
			$selected = ($compareFields0) ? $zval : $zval2;

			$group = '';
			if($fieldsize > 2)
			{
				$group = rtrim($this->fields[2]);
			}

			if($optgroup != $group)
			{
				$optgroup = $group;
				if($firstgroup)
				{
					$firstgroup = false;
					$s .="\n<optgroup label='". htmlspecialchars($group) ."'>";
				}
				else
				{
					$s .="\n</optgroup>";
					$s .="\n<optgroup label='". htmlspecialchars($group) ."'>";
				}
			}

			if($hasvalue) 
				$value = " value='".htmlspecialchars($zval2)."'";

			if(is_array($defstr))
			{
				if(in_array($selected,$defstr))
					$s .= "\n<option selected='selected'$value>".htmlspecialchars($zval).'</option>';
				else 
					$s .= "\n<option".$value.'>'.htmlspecialchars($zval).'</option>';
			}
			else
			{
				if(strcasecmp($selected,$defstr)==0) 
					$s .= "\n<option selected='selected'$value>".htmlspecialchars($zval).'</option>';
				else
					$s .= "\n<option".$value.'>'.htmlspecialchars($zval).'</option>';
			}
			$this->MoveNext();
		} // while

		// closing last optgroup
		if($optgroup != null)
		{
			$s .= "\n</optgroup>";
		}
		return $s ."\n</select>\n";
	}

	function GetMenu2($name,$defstr='',$blank1stItem=true,$multiple=false,$size=0, $selectAttr='')	
	{
		return $this->GetMenu($name, $defstr, $blank1stItem, $multiple, $size, $selectAttr, false);
	}

	function GetMenu3($name, $defstr='', $blank1stItem=true, $multiple=false, $size=0, $selectAttr='')
	{
		$hasvalue = false;

		if($multiple or is_array($defstr))
		{
			if($size==0)
				$size=5;
			$attr = ' multiple size="'.$size.'"';
			if(!strpos($name,'[]'))
				$name .= '[]';
		}
		else if($size)
			$attr = ' size="'.$size.'"';
		else $attr ='';

		$s = '<select name="'.$name.'"'.$attr.' '.$selectAttr.'>';
		if($blank1stItem) 
			if(is_string($blank1stItem))
			{
				$barr = explode(':',$blank1stItem);
				if(sizeof($barr) == 1)
					$barr[] = '';
				$s .= "\n<option value=\"".$barr[0]."\">".$barr[1]."</option>";
			} else $s .= "\n<option></option>";

		if($this->FieldCount() > 1)
			$hasvalue=true;
		else $compareFields0 = true;

		$value = '';
		$optgroup = null;
		$firstgroup = true;
		$fieldsize = sizeof($this->fields);
		while(!$this->EOF)
		{
			$zval = rtrim(reset($this->fields));

			if($blank1stItem && $zval=="")
			{
				$this->MoveNext();
				continue;
			}

			if($fieldsize > 1)
			{
				if(isset($this->fields[1]))
					$zval2 = rtrim($this->fields[1]);
				else
					$zval2 = rtrim(next($this->fields));
			}
			$selected = ($compareFields0) ? $zval : $zval2;

			$group = '';
			if(isset($this->fields[2]))
			{
				$group = rtrim($this->fields[2]);
			}

			if($optgroup != $group)
			{
				$optgroup = $group;
				if($firstgroup)
				{
					$firstgroup = false;
					$s .="\n<optgroup label='". htmlspecialchars($group) ."'>";
				}
				else
				{
					$s .="\n</optgroup>";
					$s .="\n<optgroup label='". htmlspecialchars($group) ."'>";
				}
			}

			if($hasvalue) 
				$value = " value='".htmlspecialchars($zval2)."'";

			if(is_array($defstr))
			{
				if(in_array($selected,$defstr))
					$s .= "\n<option selected='selected'$value>".htmlspecialchars($zval).'</option>';
				else 
					$s .= "\n<option".$value.'>'.htmlspecialchars($zval).'</option>';
			}
			else
			{
				if(strcasecmp($selected,$defstr)==0) 
					$s .= "\n<option selected='selected'$value>".htmlspecialchars($zval).'</option>';
				else
					$s .= "\n<option".$value.'>'.htmlspecialchars($zval).'</option>';
			}
			$this->MoveNext();
		} // while

		// closing last optgroup
		if($optgroup != null)
		{
			$s .= "\n</optgroup>";
		}
		return $s ."\n</select>\n";
	}

}

?>