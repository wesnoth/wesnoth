#define GETTEXT_DOMAIN "wesnoth-lib" /*@ \label{window_builder.cpp:textdomain} @*/

#include "gui/auxiliary/window_builder/progress_bar.hpp"

#include "config.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/progress_bar.hpp"

namespace gui2 {

namespace implementation {

tbuilder_progress_bar::tbuilder_progress_bar(const config& cfg) /*@ \label{window_builder.cpp:constructor} @*/
	: tbuilder_control(cfg)
{
}

twidget* tbuilder_progress_bar::build() const /*@ \label{window_builder.cpp:build} @*/
{
	tprogress_bar* widget = new tprogress_bar();

	init_control(widget);

	DBG_GUI_G << "Window builder: placed progress bar '"
			<< id << "' with definition '"
			<< definition << "'.\n";

	return widget;
}

} // namespace implementation

} // namespace gui2

/*@ \label{window_builder.cpp:wiki} @*//*WIKI_MACRO
 * @start_macro = progress_bar_description
 *
 *        A progress bar shows the progress of a certain object.
 * @end_macro
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_progress_bar
 *
 * == Image ==
 *
 * @macro = progress_bar_description
 *
 * A progress bar has no extra fields.
 */

