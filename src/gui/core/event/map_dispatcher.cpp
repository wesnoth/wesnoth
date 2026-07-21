#include "gui/core/event/map_dispatcher.hpp"
#include "log.hpp"

namespace gui2
{

namespace event
{

map_dispatcher::map_dispatcher() {
	connect();
	PLAIN_LOG << "map_dispatcher created";
}

map_dispatcher::~map_dispatcher() {
	PLAIN_LOG << "map_dispatcher destroyed";
}

bool map_dispatcher::is_at(const point& /*coordinate*/) const
{
	return true;
}

}

}
