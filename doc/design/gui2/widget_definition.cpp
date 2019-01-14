#define GETTEXT_DOMAIN "wesnoth-lib" /*@ \label{widget_definition.cpp:textdomain} @*/

#include "gui/auxiliary/widget_definition/progress_bar.hpp"

#include "gui/auxiliary/log.hpp"

namespace gui2 {

tprogress_bar_definition::tprogress_bar_definition(const config& cfg) /*@ \label{widget_definition.cpp:constructor} @*/
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing progress bar " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

tprogress_bar_definition::tresolution::tresolution(const config& cfg) /*@ \label{widget_definition.cpp:resolution_constructor} @*/
	: tresolution_definition_(cfg)
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_progress_bar
 *
 * == Progress bar ==
 *
 * @macro = progress_bar_description
 *
 * The definition of a progress bar. This object shows the progress of a certain
 * action, or the value state of a certain item.
 *
 * The following states exist:
 * * state_enabled, the progress bar is enabled.
 */
	// Note the order should be the same as the enum tstate is progress_bar.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
}

} // namespace gui2
