//! @file doxygen.cpp
//! Text and HTML-code for the startpage of the doxygen-documentation.

/*
	This file contains no code, and is only used when generating documentation.
	It serves as a place for the main page text and various developer documentation.
*/

// ===========================================================================
//	Main page
// ===========================================================================

/**

@mainpage Developer Documentation

	<table border="0">
	<tr>
	<td valign="top">
		@image html wesnoth-icon.png
	</td>
	<td>
		<h3>Howto</h3>
		@li @ref get_involved
		@li @ref howto_document

	</td>
	</tr>

	<tr>
	<td valign="top">
		@image html portraits/humans/mage.png
	</td>
	<td>
		<h3>Reference</h3>
		@li <a href="namespaces.html">Namespaces</a>
		@li @ref unnamed_namespace
		@li <a href="hierarchy.html">Classes</a>
		@li <a href="files.html">Source Files</a>
		@li <a href="todo.html">Todo</a>
		@li <a href="deprecated.html">Deprecated</a>
	</td>
	</tr>
	</table>

*/

// ===========================================================================
//	namespace about
// ===========================================================================

/*-

@namespace about	Display credits about all contributors.

This module is used from the startup screen.
When show_about() is called, a list of contributors
to the game will be presented to the user.

*/

// ===========================================================================
//	namespace dialogs
// ===========================================================================

/**

@namespace dialogs	Various uncategorised dialogs.

*/

// ===========================================================================
//	namespace events
// ===========================================================================

/**

@namespace events	Handling of system events.

System events include mouse and key events, and other events which
are not domain specific.
The program maintains a stack of event_context objects,
the top of the stack being the active event_context.

When an object of a type inheriting from handler is
instantiated, it will be associated with the active event_context
(unless auto_join has been set false, in which case it can manually
be instructed to join a later context). As long as its event_context
remains active, and only then, it will receive all system events.

@note Multiple handler objects will receive the same events,
including key events.

*/

// ===========================================================================
//	namespace font
// ===========================================================================

/**

@namespace font	Graphical text output.

This module is used to display and measure text.
Text can optionally contain special characters, which may
change specified display properties such as colour
or font size.
If special characters are turned on,
they can be escaped, C-style, using backslashes.

*/

// ===========================================================================
//	namespace game_config
// ===========================================================================

/**

@namespace game_config	Game configuration data as global variables.

This module can be used to load various high level
game configuration data from a .cfg file.
The loaded data will subsequently be accessible via the
global variables.

*/

// ===========================================================================
//	namespace game_events
// ===========================================================================

/**

@namespace game_events	Domain specific events

This module defines the game's event mechanism. Events might be units
moving or fighting, or when victory or defeat occurs. A scenario's
configuration file will define actions to take when certain events
occur. This module is responsible for making sure that when the events
occur, the actions take place.

Game events have nothing to do with mouse movement, keyboard events, etc.
These kinds of system events can be handled using namespace @ref events

*/

// ===========================================================================
//	namespace gui
// ===========================================================================

/**

@namespace gui	General purpose widgets.

This module primarily contains a number of common, general purpose
widgets for the construction of composite user interfaces.

*/

// ===========================================================================
//	namespace hotkey
// ===========================================================================

/**

@namespace hotkey	Keyboard shortcuts for game actions.

Hotkey commands can be loaded from configuration objects.
When a keyboard event corresponding to a hotkey occurs,
a command_executor object can execute the hotkeys actions.
For this to work, key_event() must be called whenever a keyboard event happens.

*/

// ===========================================================================
//	namespace image
// ===========================================================================

/*- already documented

@namespace image	Cache of images.

This module manages the cache of images. With an image name, you can get
the surface corresponding to that image, and don't need to free the image.
Note that surfaces returned from here are invalidated whenever events::pump()
is called, and so shouldn't be kept, but should be regotten from here as
needed.

images come in a number of varieties:
 - unscaled: no modifications have been done on the image.
 - scaled: images are scaled to the size of a tile
 - greyed: images are scaled and in greyscale
 - brightened: images are scaled and brighter than normal.

*/

// ===========================================================================
//	namespace mp
// ===========================================================================

/*- already documented

@namespace mp	Multiplayer meeting place and game creation.

This module controls the multiplayer lobby.
The lobby is a section on the server which allows players
to chat, create games, and join games.

*/

// ===========================================================================
//	namespace network
// ===========================================================================

/**

@namespace network	High level network layer for config object transport.

This module provides high level network access using an API similar
to sockets, but primarily for the transport of @ref config objects.
This is how the games protocols work - data is sent via config objects.

A client would create a @ref manager object to initialize
the network layer, connect(), and then send_data().
A server would create a @ref server_manager object,
then accept_connection(), and finally receive_data().

*/

// ===========================================================================
//	namespace preferences
// ===========================================================================

/**

@namespace preferences	Modify, read and display user preferences.

This module contain GUI code to display dialogs regarding
user preferences, and functions which read and modify the preferences.

*/

// ===========================================================================
//	namespace reports
// ===========================================================================

/**

@namespace reports	Unit and team statistics.

This module can provide statistics and information, such as these
presented in the in-game windows rightmost and upper borders.
This is primarily characteristics of units and teams.

*/

// ===========================================================================
//	namespace sound
// ===========================================================================

/**

@namespace sound	Audio output for sound and music.

This module provides the ability to play music and sounds.
Setting music volume to 0 will stop the music.

*/

// ===========================================================================
//	namespace tooltips
// ===========================================================================

/**

@namespace tooltips	tooltips.

This module can be used to register tooltips,
which will be shown provided @ref process()
is called every time mouse motion events
are received from the @ref events system.
If tooltips::draw_text() is used instead of font::draw_text(),
the text will also be registered as a tooltip.

*/

// ===========================================================================
//	Tutorials
// ===========================================================================

/**

@page get_involved	Get Involved

Before you can join the development team, your work
needs to be reviewed by other developers.

- Clone the <a href="https://wiki.wesnoth.org/WesnothRepository">Git repository</a> to obtain access to the latest source code.
- Read up on our C++ <a href="https://wiki.wesnoth.org/CodingStandards">coding standards</a>.
- Check the <a href="https://wiki.wesnoth.org/PatchSubmissionGuidelines">Patch Submission Guidelines</a> and the <a href="https://wiki.wesnoth.org/DeveloperGuide">Developer Guide</a> for information on best practices for authoring Git commits.
- Submit your pull requests on <a href="https://github.com/wesnoth/wesnoth/">GitHub</a>.

One way to contribute is to find an unassigned bug in the
<a href="http://gna.org/bugs/?group=wesnoth">bug tracker</a>
and fix it.
If you wish to work on something else, you should probably
explain on the
<a href="https://forums.wesnoth.org/">forum</a>
what you wish to do before doing any actual development.
When adding new features, keep in mind the project philosophy,
which is that of
<a href="https://wiki.wesnoth.org/WesnothPhilosophy">KISS</a>.

The development team hangs out in the IRC channel
<a href="https://webchat.freenode.net/?channels=wesnoth-dev">@#wesnoth-dev on irc.freenode.net</a>.
Feel free to join us and ask any questions you may have about our codebase!

*/

/**

@page howto_document	Document your code

@section motivation	Motivation

The document you are reading now was generated using
<a href="http://www.doxygen.org/">Doxygen</a>.
It follows in the tradition of
<a href="http://www.literateprogramming.com/">literal programming</a>,
the goal of which is to keep documentation in the source code,
when practical.
This way, the documentation will not be outdated or unmaintained.

@section interfaces	Commenting interfaces

Concise comments are preferred, as long as the explanation
is correct, is not open to interpretation and does not
assume extensive knowledge of other parts of the system.

By interface, we mean all content of a header file which
is available from a C++ source file, and could result in
compile errors if removed.
When you comment a header file, you need to take care
of a few, minor things in order to produce readable
documentation using Doxygen.
The basic guidelines for this project are:

- When commenting part of an interface, use one slash followed by
  two asterisks, followed by a line break and then the actual comment
  with an asterisk and a space aligned to the first asterisk in the
  opening line.
  The first sentence you write, terminated by a period,
  will be the brief description.
  After that, you can write a longer, more detailed description.
  The brief description will be shown in overviews,
  so it should be no more than a single line.
  It is possible to document virtually all parts of an interface,
  so it is not limited to classes.

Example:
@code
/** Takes care of displaying the map and game-data on the screen.
 *
 * The display is divided into two main sections: the game area,
 * which displays the tiles of the game board, and units on them,
 * and the side bar, which appears on the right hand side.
 * The side bar display is divided into three sections.
 */
class display
{
   ...
};
@endcode

- It is also possible to document symbols on the same line by using a slash
  followed by two asterisks and a left angular bracket:

@code
enum ADDON_TYPE {
	ADDON_SP_CAMPAIGN,	/**< Single-player campaign. */
	ADDON_SP_SCENARIO,	/**< Single-player scenario. */
	ADDON_MP_CAMPAIGN,	/**< Multiplayer campaign. */
	ADDON_MP_SCENARIO,	/**< Multiplayer scenario. */
};
@endcode

- Do not refer to multiple objects of the type "Manager"
  as "Managers" or "manager". Instead, say "Manager objects".
  Doxygen will automatically link to class documentation
  whenever it finds class names in comments,
  but will not do so if you do not use their proper names.

- Many <a href="http://www.stack.nl/~dimitri/doxygen/commands.html">Doxygen commands</a>
  can be used in comments to enhance the generated documentation and structure the comments.

- There is a balance between readable autogenerated documentation and readable code,
  so beware of overdoing it.

Example:
@code
/** @param a	an integer dividend
 * @param b	an integer divisor, which must not be zero
 * @returns	a / b
 *
 * @pre b != 0
 * @post divide' = a / b
 *
 * @throws std::runtime_error
 * @todo this has not been peer reviewed yet
 */
int divide(int a, int b)
{
	return a / b;
}
@endcode

*/

// ===========================================================================
//	Reference
// ===========================================================================

/**
	@defgroup unnamed_namespace	Unnamed Namespace

	@{
*/

/**	-file actions.hpp */
/**	-file ai_attack.hpp */
/**	-file ai.hpp */
/**	-file ai_move.hpp */
/**	-file config.hpp */
/**	-file display.hpp */
/**	-file filesystem.hpp */
/**	@file game.hpp */
/**	-file gamestatus.hpp */
/**	@file intro.hpp */
/**	@file key.hpp */
/**	@file language.hpp */
/**	-file log.hpp */
/**	@file mapgen_dialog.hpp */
/**	-file mapgen.hpp */
/**	-file map.hpp */
/**	-file multiplayer_client.hpp */
/**	-file multiplayer_connect.hpp */
/**	@file multiplayer.hpp */
/**	-file pathfind.hpp */
/**	-file playlevel.hpp */
/**	@file playturn.hpp */
/**	@file race.hpp */
/**	@file replay.hpp */
/**	-file sdl_utils.hpp */
/**	@file team.hpp */
/**	@file terrain.hpp */
/**	-file theme.hpp */
/**	-file unit.hpp */
/**	@file unit_types.hpp */
/**	-file util.hpp */
/**	@file video.hpp */

/** @} */

//.
