#ifndef CLIPBOARD_HPP_INCLUDED
#define CLIPBOARD_HPP_INCLUDED

#include <string>
#include "SDL.h"
void copy_ucs2_to_clipboard(const ucs2_string& text);
ucs2_string copy_ucs2_from_clipboard();
void copy_to_clipboard(const std::string& text);
std::string copy_from_clipboard();

#if defined(_X11) && !defined(__APPLE__)
void handle_system_event(const SDL_Event& ev);
#endif

#endif
