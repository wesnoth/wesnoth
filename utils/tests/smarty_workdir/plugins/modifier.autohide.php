<?php

function smarty_modifier_autohide($text, $min_length_to_hide, $split_from_space, $take_end = false)
{
	if (mb_strlen($text) > $min_length_to_hide)
	{
		$split_from_space = $split_from_space?"true":"false";
		$take_end = $take_end?"true":"false";
		return "<script type='text/javascript'>
			/*<![CDATA[*/
			autohide_store.push(new Autohide('$text', $min_length_to_hide, $split_from_space, $take_end));
			/*]]>*/
			</script>";
	} else {
		return $text;
	}
}

?>
