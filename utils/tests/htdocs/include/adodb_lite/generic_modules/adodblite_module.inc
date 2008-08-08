<?php
// Generic adodblite Module

eval('class adodblite_EXTENDER extends ' . $last_module . '_ADOConnection { }');

class adodblite_ADOConnection extends adodblite_EXTENDER
{
	function &GetToFieldArray($sql, $inputarr=false, $fieldname = false)
	{
		$data = false;
		$result =& $this->Execute($sql, $inputarr);
		if ($result) {
			$data =& $result->GetToFieldArray($fieldname);
			$result->Close();
		}
		return $data;
	}

}

eval('class adodblite_resultset_EXTENDER extends ' . $last_module . '_ResultSet { }');

class adodblite_ResultSet extends adodblite_resultset_EXTENDER
{
	function &GetToFieldArray($fieldname = false) 
	{
		$results = false;
		if(!$fieldname)
		{
			return $results; 
		}

		while (!$this->EOF) {
			$results[$this->fields[$fieldname]] = $this->fields;
			$this->MoveNext();
		}
		return $results; 
	}

}
?>