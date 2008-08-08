<?php
// Generic Object Module

eval('class object_EXTENDER extends ' . $last_module . '_ADOConnection { }');

class object_ADOConnection extends object_EXTENDER
{
}

class ADOFetchObj
{
}

eval('class object_resultset_EXTENDER extends ' . $last_module . '_ResultSet { }');

class object_ResultSet extends object_resultset_EXTENDER
{
	var $_obj;
	var $_names;

	function &FetchObj()
	{
		$object =& $this->FetchObject(false);
		return $object;
	}

	function &FetchObject($isupper=true)
	{
		if (empty($this->_obj))
		{
			$this->_obj = new ADOFetchObj();
			$this->_names = array();
			for ($i=0; $i <$this->_numOfFields; $i++)
			{
				$field = $this->FetchField($i);
				$this->_names[] = $field->name;
			}
		}
		$i = 0;
		if (PHP_VERSION >= 5)
			$object = clone($this->_obj);
		else $object = $this->_obj;

		for ($i=0; $i <$this->_numOfFields; $i++)
		{
			$name = $this->_names[$i];
			if ($isupper)
				$newname = strtoupper($name);
			else $newname = $name;

			$object->$newname = $this->Fields($name);
		}
		return $object;
	}

	function &FetchNextObj()
	{
		$object =& $this->FetchNextObject(false);
		return $object;
	}

	function &FetchNextObject($isupper=true)
	{
		$object = false;
		if ($this->_numOfRows != 0 && !$this->EOF) {
			$object = $this->FetchObject($isupper);	
			$this->_currentRow++;
			if ($this->_fetch())
				return $object;
		}
		$this->EOF = true;
		return $object;
	}

	/* Load a list of database objects 
	 * @param string The field name of a primary key 
	 * @return array If <var>key</var> is empty as sequential list of returned records. 
	 * If <var>key</var> is not empty then the returned array is indexed by the value 
	 * the database key.  Returns <var>null</var> if the query fails.
	 */
	function &loadObjectList( $key='' )
	{
		$array = array();
		while ($row = $this->FetchNextObject(false))
		{
			if ($key)
			{
				$array[$row->$key] = $row;
			}
			else
			{
				$array[] = $row;
			}
		}
		return $array;
	}

}
?>