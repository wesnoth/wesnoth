#ifndef MULTIPLAYER_LOBBY_INCLUDED
#define MULTIPLAYER_LOBBY_INCLUDED

#include "config.hpp"
#include "display.hpp"

///this module controls the multiplayer lobby. A section on the server which
///allows players to chat, create games, and join games.
namespace lobby {

enum RESULT { QUIT, CREATE, JOIN, CONTINUE };

///interface for an interactive dialog that is displayed while lobby user lists
///and lobby chat continue
class dialog
{
public:
	virtual void set_area(const SDL_Rect& area) = 0;
	virtual RESULT process() = 0;
	virtual bool manages_network() const { return false; }
	virtual bool get_network_data(config& out) { return false; }
};

///function which controls the lobby, and will result in the player creating
///a game, joining a game, or quitting the lobby.
RESULT enter(display& disp, config& data, const config& terrain_data, dialog* dlg,
			 std::vector<std::string>& messages);

}

#endif
