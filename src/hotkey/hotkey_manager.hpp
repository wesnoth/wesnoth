#ifndef HOTKEY_MANAGER_HPP_INCLUDED
#define HOTKEY_MANAGER_HPP_INCLUDED


/* The hotkey system allows hotkey definitions to be loaded from
 * configuration objects, and then detect if a keyboard event
 * refers to a hotkey command being executed.
 */
namespace hotkey {

/// this class is initialized once at game start
/// put all initialization and wipe code in the methods here.
class manager {
public:
	manager();
	static void init();
	static void wipe();
	~manager();
};

}

#endif
