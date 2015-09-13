#include "undo_action.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "resources.hpp"
#include "variable.hpp" // vconfig
#include "game_events/pump.hpp" //game_events::queued_event

#include <cassert>
#include <boost/foreach.hpp>

namespace actions
{
void undo_action::execute_undo_umc_wml()
{
	assert(resources::lua_kernel);
	BOOST_FOREACH(const config& c, umc_commands_undo)
	{
		resources::lua_kernel->run_wml_action("command", vconfig(c), game_events::queued_event("undo", map_location(), map_location(), config()));
	}
}

void undo_action::execute_redo_umc_wml()
{
	assert(resources::lua_kernel);
	BOOST_FOREACH(const config& c, umc_commands_redo)
	{
		resources::lua_kernel->run_wml_action("command", vconfig(c), game_events::queued_event("redo", map_location(), map_location(), config()));
	}
}

void undo_action::read_tconfig_vector(tconfig_vector& vec, const config& cfg, const std::string& tag)
{
	config::const_child_itors r = cfg.child_range(tag);
	vec.insert(vec.end(), r.first, r.second);
}
void undo_action::write_tconfig_vector(const tconfig_vector& vec, config& cfg, const std::string& tag)
{
	BOOST_FOREACH(const config& c, vec)
	{
		cfg.add_child(tag, c);		
	}
}

}
