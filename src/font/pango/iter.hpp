#pragma once

#include <pango/pango.h>

namespace font {

/**
 * Small helper wrapper for PangoLayoutIter*.
 *
 * Needed to make sure it gets freed properly.
 */
class p_itor
{
public:

	explicit p_itor(PangoLayout* layout_) :
		itor_(pango_layout_get_iter(layout_))
	{
	}

	p_itor(const p_itor &) = delete;
	p_itor & operator = (const p_itor &) = delete;

	~p_itor() { pango_layout_iter_free(itor_); }

	operator PangoLayoutIter*() { return itor_; }

private:

	PangoLayoutIter* itor_;
};

} // end namespace font
