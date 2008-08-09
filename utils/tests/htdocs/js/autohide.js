/**
 * Use to make long text to have short description and then on mouse over
 * show whole text over the page
 **/

var next_id = 1;

function make_hide_text(text, length, split_from_space, take_en)
{
	var id = next_id++;
	var short_text;
	var split_text = text.split(' ');
	if (split_from_space)
	{
		short_text = "";

		var i = 0;

		while(i < split_text.length && short_text.length + split_text[i].length < length)
		{
			if (i>0)
				short_text += ' ';
			short_text += split_text[i];
			++i;
		}
		short_text = document.createTextNode(short_text);
	} else {
		var start=0;
		if (take_end)
		{
			start = text.length - length;
			if (start < 0)
				start =0;
		}
		short_text = document.createTextNode(text.substr(start,length));
	}
	text = text.split("\n");
	split_text = new Array();
	while(true)
	{
		split_text.push(document.createTextNode(text.shift()));
		if (text.length)
		{
			split_text.push(document.createElement('BR'));
		} else {
			break;
		}
	}
	// Create short text element
	var new_elem = document.createElement('DIV');
	new_elem.id='autohide_' + id;
	new_elem.className="autohide";

	// create full text element
	var elem_over = new_elem.cloneNode(true);
	elem_over.id='autohide_over_' + id;
	elem_over.onmouseout=autohide_mouseout;
	elem_over.style.visibility = 'hidden';
	for(var i = 0; i < split_text.length; ++i)
	{
		elem_over.appendChild(split_text[i]);
	}
	var target = document.getElementById('autohide');
	
	// Fill short text element
	new_elem.appendChild(short_text);
	new_elem.onmouseover=autohide_mouseover;

	// add elements to document
	target.parentNode.replaceChild(new_elem, target);
	new_elem.parentNode.appendChild(elem_over);
	// Make over element apear in same place
	elem_over.className="autohide_over";
}

function getYpos(element)
{
	if (!element.offsetParent)
		return element.offsetTop;
	else
		return element.offsetTop + getYpos(element.offsetParent);
}

function getXpos(element)
{
	if (!element.offsetParent)
		return element.offsetLeft;
	else
		return element.offsetLeft + getXpos(element.offsetParent);
}

function autohide_mouseover(e)
{
	var targ;
	if(!e) e = window.event;
	if (e.target) targ = e.target;
	else if (e.srcElement) targ = e.srcElement;
	if (targ.nodeType == 3) // defeat Safari bug
		targ = targ.parentNode;


	var over_id = new String(targ.id).replace('hide','hide_over');
	var over_elem = document.getElementById(over_id);
	over_elem.style.visibility='visible';
	var y_pos = getYpos(targ);
	var x_pos = getXpos(targ);
	var y_pos2 = y_pos + targ.scrollHeight;
	var x_pos2 = x_pos + targ.scrollWidth;
	y_pos = (y_pos+y_pos2)/2;
	x_pos = (x_pos+x_pos2)/2;

	y_pos = y_pos - over_elem.scrollHeight/2;
	x_pos = x_pos - over_elem.scrollWidth/2;

	over_elem.style.top = y_pos + 'px';
	over_elem.style.left = x_pos + 'px';
	targ.style.visibility='hidden';
}

function autohide_mouseout(e)
{
	var targ;
	if(!e) e = window.event;
	if (e.target) targ = e.target;
	else if (e.srcElement) targ = e.srcElement;
	if (targ.nodeType == 3) // defeat Safari bug
		targ = targ.parentNode;

	targ.style.visibility='hidden';
	var orig_id = new String(targ.id).replace('_over','');
	var orig = document.getElementById(orig_id);
//	orig.onmouseover=autohide_mouseover;
	orig.style.visibility='visible';
}
