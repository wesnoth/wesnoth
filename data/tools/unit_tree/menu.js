/*
 * Popup menu implementation for the Wesnoth units database
 */
var wmlunits_menu_setup = (function() {
    var menus = [];
    var visibleMenu = false;

    function toggleMenu(event, menu)
    {
        hideMenus(menu);

        if (!menu.style.display || menu.style.display === "none") {
            menu.style.display = "block";
            visibleMenu = true;
        } else {
            menu.style.display = "none";
            visibleMenu = false;
        }
    }

    function hideMenus(skipMenu)
    {
        if (!visibleMenu) {
            return;
        }

        for (var i = 0; i < menus.length; ++i) {
            if (menus[i] !== skipMenu) {
                menus[i].style.display = "none";
            }
        }
    }

    function isMenuBox(e)
    {
        return e.className.search(/\bpopupmenu\b/) != -1;
    }

    function isNavBar(e)
    {
        return e.className.search(/\bnavbar\b/) != -1;
    }

    var navItems = document.getElementsByClassName("popupcontainer");

    // Set event handlers for individual menu triggers.
    for (var i = 0; i < navItems.length; ++i) {
        var navItem = navItems[i],
            menu = navItem.getElementsByClassName("popupmenu")[0];

        menus.push(menu);

        var id = menu.id,
            a = navItem.getElementsByClassName("popuptrigger")[0];

        a.addEventListener("click", function(event) {
            event.preventDefault();
            event.stopPropagation();
            toggleMenu(event, event.target.nextElementSibling);
        }, false);
    }

    // Dismiss all visible menus on click outside them.
    document.addEventListener("click", function(event) {
        if (!visibleMenu) {
            return;
        }

        var parent = event.target;
        while(parent && !isMenuBox(parent) && !isNavBar(parent)) {
            parent = parent.parentElement;
        }

        if (!parent || !isMenuBox(parent)) {
            hideMenus();
        }
    }, false);
});
