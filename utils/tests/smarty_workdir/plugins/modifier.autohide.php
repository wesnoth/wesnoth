<?php

function smarty_modifier_autohide($text, $min_length_to_hide, $split_from_space, $take_end = false)
{
	if (mb_strlen($text) > $min_length_to_hide)
	{
		$split_from_space = $split_from_space?"true":"false";
		$take_end = $take_end?"true":"false";
		return "<div id='autohide'></div><script type='text/javascript'>
			/*<![CDATA[*/
			var text = '$text';
			var length = $min_length_to_hide;
			var split_from_space = $split_from_space;
			var take_end = $take_end;
			make_hide_text(text, length, split_from_space, take_end);
			/*]]>*/
			</script>";
	} else {
		return $text;
	}
}

?>
