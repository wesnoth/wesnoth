#include "clipboard.hpp"

#ifdef WIN32
#include <windows.h>
#endif

void copy_to_clipboard(const std::string& text)
{
	if(text.empty()) {
		return;
	}

#ifdef WIN32
	if(!OpenClipboard(0)) {
		return;
	}

	EmptyClipboard();

	const HGLOBAL clip_buffer = GlobalAlloc(GMEM_DDESHARE,text.size()+1);
	char* const buffer = reinterpret_cast<char*>(GlobalLock(clip_buffer));
	strcpy(buffer,text.c_str());

	GlobalUnlock(clip_buffer);
	SetClipboardData(CF_TEXT,clip_buffer);
	CloseClipboard();
#endif
}

std::string copy_from_clipboard()
{
#ifdef WIN32
	if(OpenClipboard(NULL)) {
		const HANDLE data = GetClipboardData(CF_TEXT);
		char* const buffer = reinterpret_cast<char*>(GlobalLock(data));
		GlobalUnlock(data);
		CloseClipboard();

		return buffer;
	}
#endif

	return "";
}
