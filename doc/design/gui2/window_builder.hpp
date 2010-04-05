#ifndef GUI_AUXILIARY_WINDOW_BUILDER_PROGRESS_BAR_HPP_INCLUDED
#define GUI_AUXILIARY_WINDOW_BUILDER_PROGRESS_BAR_HPP_INCLUDED

#include "gui/auxiliary/window_builder/control.hpp"

namespace gui2 {

namespace implementation {

struct tbuilder_progress_bar /*@ \label{window_builder.hpp:control} @*/
	: public tbuilder_control
{

	tbuilder_progress_bar(const config& cfg);

	twidget* build () const;
};

} // namespace implementation

} // namespace gui2

#endif


