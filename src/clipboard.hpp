#ifndef CLIPBOARD_HPP_INCLUDED
#define CLIPBOARD_HPP_INCLUDED

#include <string>

void copy_to_clipboard(const std::string& text);
std::string copy_from_clipboard();

#endif