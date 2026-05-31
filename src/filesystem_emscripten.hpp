/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once
#ifdef __EMSCRIPTEN__

namespace filesystem {
namespace emscripten {

/** Fire-and-forget persist of MEMFS -> IndexedDB via FS.syncfs(). */
void syncfs();

/** Trigger browser download of /userdata as a ZIP file. */
void export_saves();

/** Open browser file picker to import a ZIP into /userdata. */
void import_saves();

} // namespace emscripten
} // namespace filesystem

#endif
