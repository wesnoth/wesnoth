/*
	Copyright (C) 2025 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/fps_report.hpp"

#include "filesystem.hpp"
#include "gui/core/tracked_drawable.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/dialogs/modeless_dialog.hpp"
#include "gui/widgets/rich_label.hpp"
#include "utils/rate_counter.hpp"

#include <tuple>
#include <vector>

#ifdef DUMP_FPS_TO_FILE
#undef DUMP_FPS_TO_FILE
#endif

namespace gui2::dialogs
{
namespace
{
class fps_report : public modeless_dialog
{
public:
	fps_report(const gui2::tracked_drawable& target)
		: modeless_dialog(window_id())
		, target_(target)
	{
	}

private:
	const std::string& window_id() const override;

	/* Inherited from top_level_drawable. */
	void update() override;

	/** The drawable whose render calls we are tracking. */
	const gui2::tracked_drawable& target_;

	/** Only update the displayed count every few update cycles. */
	utils::rate_counter update_check_{10};

	/** Holds the prior (max 1000) displayed fps values. */
	std::vector<std::tuple<int, int, int>> fps_history_{};
};

/** Generates the rich_label markup for the report. */
auto generate_markup(const gui2::tracked_drawable::frame_info& info)
{
	return config{ "table", config{ "width", "fill",
		"row", config{
			"col", config{},
			"col", config{ "halign", "right", "text", "min" },
			"col", config{ "halign", "right", "text", "avg" },
			"col", config{ "halign", "right", "text", "max" },
			"col", config{ "halign", "right", "text", "act" },
		},

		"row", config{
			"col", config{ "halign", "right", "text", "fps" },
			"col", config{ "halign", "right", "text", info.min_fps },
			"col", config{ "halign", "right", "text", info.avg_fps },
			"col", config{ "halign", "right", "text", info.max_fps },
			"col", config{ "halign", "right", "text", info.act_fps },
		},

		"row", config{
			"col", config{ "halign", "right", "text", "ms" },
			"col", config{ "halign", "right", "text", info.max_time },
			"col", config{ "halign", "right", "text", info.avg_time },
			"col", config{ "halign", "right", "text", info.min_time },
			"col", config{},
		},
	}};
}

void fps_report::update()
{
	if(!update_check_.poll()) {
		return;
	}

	// Will be null if no times have been recorded yet
	auto info = target_.get_info();
	if(!info) {
		return;
	}

	find_widget<rich_label>("values").set_dom(generate_markup(*info));

#ifdef DUMP_FPS_TO_FILE
	fps_history_.emplace_back(info->min_fps, info->avg_fps, info->max_fps);

	// Flush out the stored values every so often
	if(fps_history_.size() == 1000) {
		std::string filename = filesystem::get_user_data_dir() + "/fps_log.csv";
		auto fps_log = filesystem::ostream_file(filename, std::ios_base::binary | std::ios_base::app);

		for(const auto& [min, avg, max] : fps_history_) {
			*fps_log << min << "," << avg << "," << max << "\n";
		}

		fps_history_.clear();
	}
#endif
}

std::unique_ptr<fps_report> report;

} // namespace

REGISTER_DIALOG(fps_report)

namespace fps
{
void show(const gui2::tracked_drawable& target)
{
	// No-op if currently displayed
	if(report) {
		return;
	}

	report.reset(new fps_report(target));
	report->show();

	// HACK: in order that the display not prevent events from reaching the in-game
	// event context, leave the UI event context. This should be removed if ever we
	// get the two event contexts to play nicely...
	report->disconnect();
}

void hide()
{
	report.reset();
}

} // namespace fps

} // namespace gui2::dialogs
