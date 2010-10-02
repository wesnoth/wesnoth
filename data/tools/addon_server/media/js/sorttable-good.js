/* http://brothercake.com/site/resources/scripts/domready/
**************************************************************************/

function domFunction(f, a) {
	var n = 0;
	var t = setInterval(function() {
		var c = true;
		n++;
		if(typeof document.getElementsByTagName != 'undefined' && (document.getElementsByTagName('body')[0] != null || document.body != null)) {
			c = false;
			if(typeof a == 'object') {
				for(var i in a) {
					if((a[i] == 'id' && document.getElementById(i) == null) || (a[i] == 'tag' && document.getElementsByTagName(i).length < 1)) { 
						c = true; 
						break; 
					}
				}
			}
			if(!c) { f(); clearInterval(t); }
		}
		if(n >= 60) {
			clearInterval(t);
		}
	}, 250);
};

/* http://kryogenix.org/code/browser/sorttable/
**************************************************************************/

var sci;

function sortTable() {
	if (!document.getElementsByTagName) return;
	tbls = document.getElementsByTagName('table');
	for (ti=0; ti<tbls.length; ti++) {
		thisTbl = tbls[ti];
		st_makeSortable(thisTbl);
	}
}

function st_makeSortable(table) {
	if (table.rows && table.rows.length > 0) {
		var firstRow = table.rows[0];
	}
	if (!firstRow) return;
	for (var i=0; i<firstRow.cells.length; i++) {
		var cell = firstRow.cells[i];
		var txt = st_getInnerText(cell);
		cell.innerHTML = '<a href="" class="sortheader" onclick="st_resortTable(this);return false;">'+txt+'<span class="sortarrow"></span></a>';
	}
}

function st_getInnerText(el) {
	if (typeof el == 'string') return el;
	if (typeof el == 'undefined') return el;
	if (el.innerText) return el.innerText;
	var str = '';
	var cs = el.childNodes;
	var l = cs.length;
	for (var i = 0; i < l; i++) {
		switch (cs[i].nodeType) {
			case 1:
				str += st_getInnerText(cs[i]);
				break;
			case 3:
				str += cs[i].nodeValue;
				break;
		}
	}
	return str;
}

function st_resortTable(lnk) {
	var span;
	for (var ci=0; ci<lnk.childNodes.length; ci++) {
		if (lnk.childNodes[ci].tagName && lnk.childNodes[ci].tagName.toLowerCase() == 'span') span = lnk.childNodes[ci];
	}
	var spantext = st_getInnerText(span);
	var td = lnk.parentNode;
	var column = td.cellIndex;
	var table = st_getParent(td, 'TABLE');
	if (table.rows.length <= 1) return;
	var notDate = 0;
	var notCurrency = 0;
	var notNumerical = 0;
	for (var itmc=1; itmc<table.rows.length; itmc++) {
		var itm = st_getInnerText(table.rows[itmc].cells[column]);
		if (!(itm.match(/^\d\d[\/-]\d\d[\/-]\d\d\d\d$/) || (itm.match(/^\d\d[\/-]\d\d[\/-]\d\d$/))))
			notDate++;
		if (!(itm.match(/^[€£$]/)))
			notCurrency++;
		if (!(itm.match(/^[\+-]?[\d\.,]+$/)))
			notNumerical++;
	}
	   switch (0) {
	   	case notDate: sortfn = st_sortDate; break;
	   	case notCurrency: sortfn = st_sortCurrency; break;
	   	case notNumerical: sortfn = st_sortNumeric; break;
	   	default: sortfn = st_sortCaseInsensitive;
	   }
	sci = column;
	var firstRow = new Array();
	var newRows = new Array();
	for (i=0; i<table.rows[0].length; i++) { firstRow[i] = table.rows[0][i]; }
	for (j=1; j<table.rows.length; j++) { newRows[j-1] = table.rows[j]; }
	newRows.sort(sortfn);
	if (span.getAttribute('sortdir') == 'down') {
		ARROW = '&nbsp;↑';
		newRows.reverse();
		span.setAttribute('sortdir', 'up');
	} else {
		ARROW = '&nbsp;↓';
		span.setAttribute('sortdir', 'down');
	}
	for (i=0; i<newRows.length; i++) { if (!newRows[i].className || (newRows[i].className && (newRows[i].className.indexOf('sortbottom') == -1))) table.tBodies[0].appendChild(newRows[i]); }
	for (i=0; i<newRows.length; i++) { if (newRows[i].className && (newRows[i].className.indexOf('sortbottom') != -1)) table.tBodies[0].appendChild(newRows[i]); }
	var allspans = document.getElementsByTagName('span');
	for (var ci=0; ci<allspans.length; ci++) {
		if (allspans[ci].className == 'sortarrow') {
			if (st_getParent(allspans[ci], 'table') == st_getParent(lnk, 'table')) {
				allspans[ci].innerHTML = '';
			}
		}
	}
	span.innerHTML = ARROW;
	colorTableRows();
}

function st_getParent(el, pTagName) {
	if (el == null) return null;
	else if (el.nodeType == 1 && el.tagName.toLowerCase() == pTagName.toLowerCase())
		return el;
	else
		return st_getParent(el.parentNode, pTagName);
}

function st_sortDate(a, b) {
	aa = st_getInnerText(a.cells[sci]);
	bb = st_getInnerText(b.cells[sci]);
	if (aa.length == 10) {
		dt1 = aa.substr(6, 4)+aa.substr(3, 2)+aa.substr(0, 2);
	} else {
		yr = aa.substr(6, 2);
		if (parseInt(yr) < 50) { yr = '20'+yr; } else { yr = '19'+yr; }
		dt1 = yr+aa.substr(3, 2)+aa.substr(0, 2);
	}
	if (bb.length == 10) {
		dt2 = bb.substr(6, 4)+bb.substr(3, 2)+bb.substr(0, 2);
	} else {
		yr = bb.substr(6, 2);
		if (parseInt(yr) < 50) { yr = '20'+yr; } else { yr = '19'+yr; }
		dt2 = yr+bb.substr(3, 2)+bb.substr(0, 2);
	}
	if (dt1==dt2) return 0;
	if (dt1<dt2) return -1;
	return 1;
}

function st_sortCurrency(a, b) { 
	aa = st_getInnerText(a.cells[sci]).replace(/[^0-9.]/g, '');
	bb = st_getInnerText(b.cells[sci]).replace(/[^0-9.]/g, '');
	return parseFloat(aa) - parseFloat(bb);
}

function st_sortNumeric(a, b) { 
	aa = parseFloat(st_getInnerText(a.cells[sci]));
	if (isNaN(aa)) aa = 0;
	bb = parseFloat(st_getInnerText(b.cells[sci])); 
	if (isNaN(bb)) bb = 0;
	return aa-bb;
}

function st_sortCaseInsensitive(a, b) {
	aa = st_getInnerText(a.cells[sci]).toLowerCase();
	bb = st_getInnerText(b.cells[sci]).toLowerCase();
	if (aa==bb) return 0;
	if (aa<bb) return -1;
	return 1;
}

function st_sortDefault(a, b) {
	aa = st_getInnerText(a.cells[sci]);
	bb = st_getInnerText(b.cells[sci]);
	if (aa==bb) return 0;
	if (aa<bb) return -1;
	return 1;
}

/* http://ktk.xs4all.nl/stuff/javascript/table-row-alternate/
**************************************************************************/

function colorTableRows() {
	if (document.getElementsByTagName) {
		var tables = document.getElementsByTagName('table');
		for (var i = 0; i < tables.length; i++) {
			var trs = tables[i].getElementsByTagName('tr');
			for (var j = 1; j < trs.length; j++) {
				trs[j].className = (j % 2 == 0 ? '' : 'alternate');
			}
		}
	}
}

/* Load the scripts
**************************************************************************/

var foobar = new domFunction(function() {
	sortTable();
	colorTableRows();
	});