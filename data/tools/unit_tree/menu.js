var all = new Array();

function toggle_menu(event, id, what) {
    var e = document.getElementById(id);
    var show = what;
    
    /* Since we have Javascript, disable the CSS menu trigger. */
    if (event.className != "") {
        event.className = "";
        e.style.display = 'none';
        return;
    }
    
    if (show == 0 || show == 1) return;
   
    if (show == 2) {
        if (e.style.display == 'block') show = 0;
    }
    if (show == 0) {
        e.style.display = 'none';
    }
    else {
        e.style.display = 'block';
        all[id] = 1;
        for (mid in all) {
            if (mid != id) {
                e = document.getElementById(mid);
                e.style.display = 'none';
            }
        }
    }
}
