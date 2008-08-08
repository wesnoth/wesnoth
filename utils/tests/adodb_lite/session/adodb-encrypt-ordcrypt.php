<?php
// security - hide paths
if (!defined('ADODB_SESSION')) die();

class ADODB_Encrypt_OrdCrypt {

	function write($data, $key) {
		$result = '';
		for($i=0; $i<strlen($data); $i++) {
			$char = substr($data, $i, 1);
			$keychar = substr($key, ($i % strlen($key))-1, 1);
			$char = chr(ord($char)+ord($keychar));
			$result.=$char;
		}
		return bin2hex($result); 
	}

	function read($data, $key) {
		$result = '';
		$data =  @pack("H" . strlen($data), $data); 

		for($i=0; $i<strlen($data); $i++) {
			$char = substr($data, $i, 1);
			$keychar = substr($key, ($i % strlen($key))-1, 1);
			$char = chr(ord($char)-ord($keychar));
			$result.=$char;
		}
		return $result;
	}
}

return 1;

?>