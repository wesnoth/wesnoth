/*
 * Popup menu implementation for the Wesnoth units database
 */
var wmlunits_menu_setup = (function() {
    var menus = [];
    var visibleMenu = false;

    //
    // Legacy browser support functions (IE 9 only, really)
    //

    function removeClass(e, classId)
    {
        var prevClasses = e.className.split(/\s+/),
            newClasses = [];

        for (var i = 0; i < prevClasses.length; ++i) {
            if (prevClasses[i] != classId) {
                newClasses.push(prevClasses[i]);
            }
        }

        e.className = newClasses.join(" ");
    }

    function toggleClass(e, classId)
    {
        var prevClasses = e.className.split(/\s+/),
            newClasses = [],
            foundClass = false;

        for (var i = 0; i < prevClasses.length; ++i) {
            if (prevClasses[i] == classId) {
                foundClass = true;
            } else {
                newClasses.push(prevClasses[i]);
            }
        }

        if (!foundClass) {
            newClasses.push(classId);
        }

        e.className = newClasses.join(" ");

        return !foundClass;
    }

    //
    // The main stuff
    //

    function toggleMenu(event, menu)
    {
        var menu = event.target.nextElementSibling;

        hideMenus(menu);
        visibleMenu = toggleClass(event.target, "menu-visible");
    }

    function hideMenus(skipMenu)
    {
        if (!visibleMenu) {
            return;
        }

        for (var i = 0; i < menus.length; ++i) {
            if (menus[i] !== skipMenu) {
                removeClass(menus[i].previousElementSibling, "menu-visible");
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
            toggleMenu(event);
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
