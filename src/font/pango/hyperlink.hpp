#pragma once

#include <string>

namespace font {

// Helper functions for link-aware text feature

inline bool looks_like_url(const std::string & str)
{
	return (str.size() >= 8) && ((str.substr(0,7) == "http://") || (str.substr(0,8) == "https://"));
}

inline std::string format_as_link(const std::string & link, const std::string & color) {
	return "<span underline=\'single\' color=\'" + color + "\'>" + link + "</span>";
}

} // end namespace font
