/**
 * Use to make long text to have short description and then on mouse over
 * show whole text over the page
 **/

var autohide_next_id = 0;
var autohide_store = new Array();

var onload_registered = false;

function addOnloadFunction(func)
{
	var oldonload = window.onload; 
	if (typeof window.onload != 'function') { 
		window.onload = func; 
	} else { 
		window.onload = function() { 
			if (oldonload) { 
				oldonload(); 
			} 
			func(); 
		} 
	}
}

function onload_autohide()
{
	for (var hide in autohide_store)
	{
		autohide_store[hide].make_hide_text();
	}
}

function Autohide(text, length, split_from_space, take_end)
{
	this.id = autohide_store.length;
	this.text = text;
	this.length = length;
	this.split_from_space = split_from_space;
	this.take_end = take_end;
	document.write('<div id="autohide' + this.id + '"></div>');
	if (!onload_registered)
	{
		addOnloadFunction(onload_autohide);
		onload_registered = true;
	}
}

Autohide.prototype.make_hide_text = function()
{
	var short_text;
	var split_text;
	if (this.split_from_space)
	{
		split_text = this.text.split(' ');
		short_text = "";

		var i = 0;

		while(i < split_text.length && short_text.length + split_text[i].length < this.length)
		{
			if (i>0)
				short_text += ' ';
			short_text += split_text[i];
			++i;
		}
		short_text = document.createTextNode(short_text);
	} else {
		var start=0;
		if (this.take_end)
		{
			start = this.text.length - this.length;
			if (start < 0)
				start =0;
		}
		short_text = document.createTextNode(this.text.substr(start,this.length));
	}
	this.text = this.text.split("\n");
	split_text = new Array();
	for(var index in this.text)
	{
		if (this.text[index].length)
		{
			split_text.push(document.createTextNode(this.text[index]));
			split_text.push(document.createElement('BR'));
		}
	}
	if (split_text.length)
		split_text.pop();
	// Create short text element
	var new_elem = document.createElement('DIV');
	new_elem.id='autohide_' + this.id;
	new_elem.className="autohide";

	// create full text element
	var elem_over = new_elem.cloneNode(false);
	elem_over.id='autohide_over_' + this.id;
	elem_over.onmouseout=autohide_mouseout;
	elem_over.style.visibility = 'hidden';
	for(var i in split_text)
	{
		elem_over.appendChild(split_text[i]);
	}
	var target = document.getElementById('autohide'+this.id);
	
	// Fill short text element
	new_elem.appendChild(short_text);
	new_elem.onmouseover=autohide_mouseover;
	elem_over.className="autohide_over";

	// add elements to document
	target.parentNode.replaceChild(new_elem, target);
	new_elem.parentNode.appendChild(elem_over);
	// Make over element apear in same place
	var max_iter = 3;
	do {
		var is_hitting_side = (elem_over.offsetLeft + elem_over.offsetWidth >= document.width)
			|| (elem_over.offsetTop + elem_over.offsetHeight > document.height);
		var y_pos = getYpos(new_elem);
		var x_pos = getXpos(new_elem);
		var y_pos2 = y_pos + new_elem.scrollHeight;
		var x_pos2 = x_pos + new_elem.scrollWidth;
		y_pos = (y_pos+y_pos2)/2;
		x_pos = (x_pos+x_pos2)/2;

		y_pos = y_pos - elem_over.scrollHeight/2;
		x_pos = x_pos - elem_over.scrollWidth/2;

//		console.log(this.id + ' x:' + x_pos + ' y:' + y_pos + ' ny:' + new_elem.scrollWidth + ' gy:' + getYpos(new_elem));

		elem_over.style.top = y_pos + 'px';
		elem_over.style.left = x_pos + 'px';
	} while(is_hitting_side && --max_iter);
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
	var new_elem;
	if(!e) e = window.event;
	if (e.target) new_elem = e.target;
	else if (e.srcElement) new_elem = e.srcElement;
	if (new_elem.nodeType == 3) // defeat Safari bug
		new_elem = new_elem.parentNode;


	var over_id = new String(new_elem.id).replace('hide','hide_over');
	var elem_over = document.getElementById(over_id);
	var y_pos = getYpos(new_elem);
	var x_pos = getXpos(new_elem);
	var y_pos2 = y_pos + new_elem.scrollHeight;
	var x_pos2 = x_pos + new_elem.scrollWidth;
	y_pos = (y_pos+y_pos2)/2;
	x_pos = (x_pos+x_pos2)/2;

	y_pos = y_pos - elem_over.scrollHeight/2;
	x_pos = x_pos - elem_over.scrollWidth/2;

	//		console.log(this.id + ' x:' + x_pos + ' y:' + y_pos + ' ny:' + new_elem.scrollWidth + ' gy:' + getYpos(new_elem));

	elem_over.style.top = y_pos + 'px';
	elem_over.style.left = x_pos + 'px';
	elem_over.style.visibility='visible';
	new_elem.style.visibility='hidden';
}

function autohide_mouseout(e)
{
	var elem_over;
	if(!e) e = window.event;
	if (e.target) elem_over = e.target;
	else if (e.srcElement) elem_over = e.srcElement;
	if (elem_over.nodeType == 3) // defeat Safari bug
		elem_over = elem_over.parentNode;

	elem_over.style.visibility='hidden';
	var new_elem_id = new String(elem_over.id).replace('_over','');
	var new_elem = document.getElementById(new_elem_id);
	new_elem.style.visibility='visible';
}
