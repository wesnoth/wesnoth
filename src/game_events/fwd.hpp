#pragma once

#include <tuple>

namespace game_events
{
	using pump_result_t = std::tuple<bool /* undo_disabled*/, bool /* action_aborted */>;
}
