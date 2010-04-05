#ifndef GUI_AUXILIARY_WIDGET_DEFINITION_PROGRESS_BAR_HPP_INCLUDED
#define GUI_AUXILIARY_WIDGET_DEFINITION_PROGRESS_BAR_HPP_INCLUDED

#include "gui/auxiliary/widget_definition.hpp"

namespace gui2 {

struct tprogress_bar_definition /*@ \label{widget_definition.hpp:control} @*/
	: public tcontrol_definition
{
	explicit tprogress_bar_definition(const config& cfg);

	struct tresolution /*@ \label{widget_definition.hpp:resolution} @*/
		: public tresolution_definition_
	{
		explicit tresolution(const config& cfg);
	};
};

} // namespace gui2

#endif
