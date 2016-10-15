#pragma once

#include <pango/pango.h>
#include <ostream>

namespace font {

inline std::ostream& operator<<(std::ostream& s, const PangoRectangle &rect)
{
	s << rect.x << ',' << rect.y << " x " << rect.width << ',' << rect.height;
	return s;
}

} // end namespace font
