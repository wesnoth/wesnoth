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

#ifdef __EMSCRIPTEN__

#include "filesystem_emscripten.hpp"
#include <emscripten.h>

EM_JS(void, wa_idbfs_sync, (), {
	if (typeof FS !== 'undefined' && FS.syncfs) {
		FS.syncfs(false, function(err) {
			if (err) console.error('[IDBFS] sync error:', err);
		});
	}
});

EM_JS(void, wa_export_saves, (), {
	if (typeof wesnothExportSaves === 'function') {
		wesnothExportSaves();
	}
});

EM_JS(void, wa_import_saves, (), {
	if (typeof wesnothImportSaves === 'function') {
		wesnothImportSaves();
	}
});

namespace filesystem {
namespace emscripten {

void syncfs()
{
	wa_idbfs_sync();
}

void export_saves()
{
	wa_export_saves();
}

void import_saves()
{
	wa_import_saves();
}

} // namespace emscripten
} // namespace filesystem

#endif
